/*
 * AmigaAI - Claude AI Agent for AmigaOS
 *
 * Native Amiga application that communicates with the Claude API
 * via HTTPS (AmiSSL v5) over Roadshow TCP/IP stack.
 *
 * Features:
 * - MUI-based GUI with chat interface
 * - Workbench and CLI launch support
 * - Persistent memory across sessions
 * - Chat history save/load
 * - ARexx port for automation (port: AMIGAAI)
 * - Configurable model, system prompt, API key
 */

#include "version.h"
#include "config.h"
#include "http.h"
#include "claude.h"
#include "gui.h"
#include "locale.h"
#include "arexx_port.h"
#include "memory.h"
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/icon.h>
#include <clib/alib_protos.h>
#include <libraries/mui.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <rexx/rxslib.h>

/* MAKE_ID for MUI window IDs */
#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG)(a)<<24|(ULONG)(b)<<16|(ULONG)(c)<<8|(ULONG)(d))
#endif

/* Version string for AmigaOS "VERSION" command */
static const char *verstag = VERSTAG;

/* Request 128KB stack - MUI's nested object creation needs substantial stack */
unsigned long __stack = 131072;

/* Library bases */
struct IntuitionBase *IntuitionBase = NULL;
struct Library       *MUIMasterBase = NULL;
struct Library       *IconBase      = NULL;
struct RxsLib        *RexxSysBase   = NULL;

/* Application state */
static struct Config     app_config;
static struct Claude     app_claude;
static struct Gui        app_gui;
static struct ARexxPort  app_arexx;
static struct Memory     app_memory;

/* Workbench state */
static int  from_wb   = 0;
static BPTR old_dir   = 0;
static BPTR old_home  = 0;  /* Original pr_HomeDir to restore on exit */
static BPTR our_home  = 0;  /* Our DupLock'd home dir to free on exit */
static int  assign_ok = 0;  /* Non-zero if AmigaAI: assign was created */

/* API log path (set via CLI APILOG= or ToolType APILOG=) */
static char api_log_file[256] = "";

/* BPTR to the original cli_CommandDir, so we can restore it on exit */
static BPTR orig_cmd_dir = 0;
static int  path_setup_done = 0;

/* Forward declarations */
static int  open_libraries(void);
static void close_libraries(void);
static void handle_send(void);
static void handle_new_chat(void);
static void handle_about(void);
static void handle_memory_view(void);
static void handle_memory_add(void);
static void handle_memory_clear(void);
static void handle_chat_save(void);
static void handle_chat_load(void);
static void arexx_response_cb(const char *response);
static void create_icon(const char *name);
static void wb_error(const char *msg);

/* Add standard directories to the process command search path.
 * AmigaDOS path list is a BPTR-linked list of 8-byte nodes:
 *   BPTR next_node   (offset 0)
 *   BPTR dir_lock    (offset 4)
 * We prepend our directories and save the original list head
 * so cleanup_search_path() can free only what we added. */
static void setup_search_path(void)
{
    struct Process *pr = (struct Process *)FindTask(NULL);
    struct CommandLineInterface *cli;
    static const char *dirs[] = {
        "C:", "S:", "SYS:Utilities", "SYS:System",
        "SYS:Tools", "SYS:Prefs", "SYS:Rexxc",
        NULL
    };
    int i;

    if (!pr || !pr->pr_CLI) return;
    cli = BADDR(pr->pr_CLI);
    if (!cli) return;

    orig_cmd_dir = cli->cli_CommandDir;

    for (i = 0; dirs[i]; i++) {
        BPTR lock = Lock((CONST_STRPTR)dirs[i], ACCESS_READ);
        if (lock) {
            LONG *node = (LONG *)AllocVec(8, MEMF_PUBLIC | MEMF_CLEAR);
            if (node) {
                node[0] = (LONG)cli->cli_CommandDir;
                node[1] = (LONG)lock;
                cli->cli_CommandDir = MKBADDR(node);
            } else {
                UnLock(lock);
            }
        }
    }
    path_setup_done = 1;
}

/* Free only the path nodes we added, restore the original list */
static void cleanup_search_path(void)
{
    struct Process *pr;
    struct CommandLineInterface *cli;

    if (!path_setup_done) return;

    pr = (struct Process *)FindTask(NULL);
    if (!pr || !pr->pr_CLI) return;
    cli = BADDR(pr->pr_CLI);
    if (!cli) return;

    /* Walk our added nodes until we reach the original head */
    while (cli->cli_CommandDir != orig_cmd_dir) {
        LONG *node = (LONG *)BADDR(cli->cli_CommandDir);
        BPTR next_bptr;
        BPTR lock_bptr;

        if (!node) break;
        next_bptr = (BPTR)node[0];
        lock_bptr = (BPTR)node[1];
        if (lock_bptr) UnLock(lock_bptr);
        cli->cli_CommandDir = next_bptr;
        FreeVec(node);
    }
    path_setup_done = 0;
}

/* Append a timestamped entry to the conversation log file.
 * Format: [HH:MM:SS] prefix: text */
static void chat_log(const char *prefix, const char *text)
{
    FILE *f = fopen("AmigaAI:chat.log", "a");
    if (!f) return;

    /* AmigaOS DateStamp for timestamp */
    {
        struct DateStamp ds;
        int h, m, s;
        DateStamp(&ds);
        s = ds.ds_Minute * 60 + ds.ds_Tick / 50;
        h = (ds.ds_Minute / 60) % 24;
        m = ds.ds_Minute % 60;
        s = ds.ds_Tick / 50;
        fprintf(f, "[%02d:%02d:%02d] %s: %s\n", h, m, s, prefix, text);
    }

    fclose(f);
}

/* Crash diagnostic: write step number to T:amigaai.log so we can find
 * where it dies even if the Guru Meditation hides the console output.
 * After a crash, run: type T:amigaai.log */
static void dbg_step(int step, const char *msg)
{
    FILE *f = fopen("T:amigaai.log", "w");
    if (f) {
        fprintf(f, "Step %d: %s\n", step, msg);
        fclose(f);
    }
    printf("  [%d] %s\n", step, msg);
}

static int open_libraries(void)
{
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((CONST_STRPTR)"intuition.library", 39);
    if (!IntuitionBase) {
        printf("ERROR: Requires AmigaOS 3.0+ (intuition.library v39)\n");
        return -1;
    }

    MUIMasterBase = OpenLibrary((CONST_STRPTR)MUIMASTER_NAME, 19);
    if (!MUIMasterBase) {
        if (from_wb)
            wb_error("Cannot open muimaster.library v19.\nPlease install MUI 3.8+");
        else {
            printf("ERROR: Cannot open muimaster.library v19\n");
            printf("Please install MUI 3.8+\n");
        }
        return -1;
    }

    RexxSysBase = (struct RxsLib *)OpenLibrary((CONST_STRPTR)"rexxsyslib.library", 0);
    if (!RexxSysBase)
        printf("WARNING: Cannot open rexxsyslib.library - ARexx disabled\n");

    return 0;
}

static void close_libraries(void)
{
    if (RexxSysBase)   CloseLibrary((struct Library *)RexxSysBase);
    if (MUIMasterBase) CloseLibrary(MUIMasterBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);

    RexxSysBase   = NULL;
    MUIMasterBase = NULL;
    IntuitionBase = NULL;
}

/* Show an error requester when launched from Workbench (no console) */
static void wb_error(const char *msg)
{
    struct EasyStruct es;

    if (!IntuitionBase) return;

    es.es_StructSize   = sizeof(struct EasyStruct);
    es.es_Flags        = 0;
    es.es_Title        = (UBYTE *)PROGRAM_NAME;
    es.es_TextFormat   = (UBYTE *)msg;
    es.es_GadgetFormat = (UBYTE *)"OK";

    EasyRequestArgs(NULL, &es, NULL, NULL);
}

/* Create a Workbench icon for the program if one doesn't exist */
static void create_icon(const char *name)
{
    struct DiskObject *dobj;
    BPTR lock;
    char path[256];

    static char *tooltypes[] = { NULL };

    /* Check if icon already exists */
    snprintf(path, sizeof(path), "%s.info", name);
    lock = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (lock) {
        UnLock(lock);
        return;
    }

    IconBase = OpenLibrary((CONST_STRPTR)"icon.library", 36);
    if (!IconBase) return;

    dobj = GetDefDiskObject(WBTOOL);
    if (dobj) {
        dobj->do_StackSize = 131072;
        dobj->do_ToolTypes = (STRPTR *)tooltypes;
        PutDiskObject((CONST_STRPTR)name, dobj);
        FreeDiskObject(dobj);
    }

    CloseLibrary(IconBase);
    IconBase = NULL;
}

/* Called when ARexx returns a response - update the GUI */
static void arexx_response_cb(const char *response)
{
    gui_add_text(&app_gui, "Claude: ", response);
}

/* Called during tool execution - show status in GUI */
static void tool_status_cb(const char *tool_name, const char *status,
                            const char *detail, void *userdata)
{
    char buf[512];
    (void)userdata;

    if (strcmp(status, "executing") == 0) {
        snprintf(buf, sizeof(buf), "\033b> %s\033n %s",
                 tool_name, detail ? detail : "");
        gui_add_line(&app_gui, buf);
        snprintf(buf, sizeof(buf), "Executing: %s", tool_name);
        gui_set_status(&app_gui, buf);

        snprintf(buf, sizeof(buf), "TOOL %s", tool_name);
        chat_log(buf, detail ? detail : "");
    } else if (strcmp(status, "error") == 0) {
        if (detail && detail[0]) {
            snprintf(buf, sizeof(buf), "\033bError:\033n %.*s",
                     (int)(sizeof(buf) - 16), detail);
            gui_add_line(&app_gui, buf);
        }
        snprintf(buf, sizeof(buf), "Tool %s failed", tool_name);
        gui_set_status(&app_gui, buf);

        snprintf(buf, sizeof(buf), "TOOL_ERROR %s", tool_name);
        chat_log(buf, detail ? detail : "failed");
    } else {
        /* "done" - show result */
        if (detail && detail[0]) {
            gui_add_text(&app_gui, NULL, detail);
        }
        snprintf(buf, sizeof(buf), "Tool %s done", tool_name);
        gui_set_status(&app_gui, buf);

        snprintf(buf, sizeof(buf), "TOOL_RESULT %s", tool_name);
        chat_log(buf, detail ? detail : "");
    }
}

/* HTTP event callback - called periodically during SSL reads.
 * Processes MUI events so the Stop button works.
 * Returns non-zero to abort the request. */
static int http_poll_cb(void *userdata)
{
    (void)userdata;
    return gui_check_abort(&app_gui);
}

/* ===================== Message handling ===================== */

static void handle_send(void)
{
    const char *input;
    char *reply;
    char *error_msg = NULL;
    char status_buf[128];

    input = gui_get_input(&app_gui);
    if (!input || !input[0]) return;

    /* Save input to history (all commands, including slash commands) */
    gui_history_push(&app_gui, input);

    /* --- Slash commands --- */

    /* /help - list all available commands */
    if (strcasecmp(input, "/help") == 0) {
        gui_add_line(&app_gui, GetString(MSG_HELP_TITLE));
        gui_add_line(&app_gui, "");
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_HELP));
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_SHELL));
        gui_add_line(&app_gui, "  Example: /shell list SYS:Utilities");
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_AREXX));
        gui_add_line(&app_gui, "  Example: /arexx MULTIVIEW ABOUT");
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_READ));
        gui_add_line(&app_gui, "  Example: /read S:Startup-Sequence");
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_WRITE));
        gui_add_line(&app_gui, "  Example: /write RAM:test.txt Hello World");
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_PORTS));
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_REMEMBER));
        gui_add_line(&app_gui, "  Example: /remember I prefer 68030 mode");
        gui_add_line(&app_gui, GetString(MSG_HELP_CMD_MEMORY));
        gui_add_line(&app_gui, "");
        gui_add_line(&app_gui, GetString(MSG_HELP_FOOTER1));
        gui_add_line(&app_gui, GetString(MSG_HELP_FOOTER2));
        gui_add_line(&app_gui, "");
        gui_set_status(&app_gui, GetString(MSG_STATUS_TYPE_CMD));
        gui_clear_input(&app_gui);
        return;
    }

    /* /remember <text> - add a memory entry */
    if (strncasecmp(input, "/remember ", 10) == 0) {
        const char *entry = input + 10;
        while (*entry == ' ') entry++;
        if (*entry) {
            if (memory_add(&app_memory, entry) == 0) {
                memory_save(&app_memory);
                gui_add_line(&app_gui, GetString(MSG_MEM_ADDED));
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), GetString(MSG_CMD_MEM_ENTRIES), app_memory.count);
                    gui_set_status(&app_gui, buf);
                }
            } else {
                gui_add_line(&app_gui, GetString(MSG_MEM_FULL));
            }
        }
        gui_clear_input(&app_gui);
        return;
    }

    /* /memory - show all memory entries */
    if (strcasecmp(input, "/memory") == 0) {
        handle_memory_view();
        gui_clear_input(&app_gui);
        return;
    }

    /* /ports - list all public Exec message ports */
    if (strcasecmp(input, "/ports") == 0) {
        int is_error = 0;
        char *result = tool_execute("list_ports", NULL, &is_error);
        gui_add_line(&app_gui, GetString(MSG_CMD_PORTS_TITLE));
        if (result) {
            gui_add_text(&app_gui, NULL, result);
            free(result);
        }
        gui_add_line(&app_gui, "");
        gui_set_status(&app_gui, GetString(MSG_CMD_PORTS_LISTED));
        gui_clear_input(&app_gui);
        return;
    }

    /* /shell <cmd> - test shell_command tool directly */
    if (strncasecmp(input, "/shell ", 7) == 0) {
        const char *cmd = input + 7;
        while (*cmd == ' ') cmd++;
        if (*cmd) {
            cJSON *inp = cJSON_CreateObject();
            int is_error = 0;
            char *result;
            char line[256];

            cJSON_AddStringToObject(inp, "command", cmd);
            snprintf(line, sizeof(line), GetString(MSG_CMD_SHELL), cmd);
            gui_add_line(&app_gui, line);
            gui_set_status(&app_gui, GetString(MSG_STATUS_EXECUTING));

            result = tool_execute("shell_command", inp, &is_error);
            cJSON_Delete(inp);

            if (result) {
                gui_add_text(&app_gui, is_error ? "\033bError:\033n " : "",
                             result);
                free(result);
            }
            gui_add_line(&app_gui, "");
            gui_set_status(&app_gui, is_error ? GetString(MSG_CMD_FAILED) : GetString(MSG_CMD_DONE));
        }
        gui_clear_input(&app_gui);
        return;
    }

    /* /arexx <port> <cmd> - test arexx_command tool directly */
    if (strncasecmp(input, "/arexx ", 7) == 0) {
        const char *args = input + 7;
        while (*args == ' ') args++;
        if (*args) {
            char port[64];
            const char *cmd;
            const char *sp = strchr(args, ' ');
            cJSON *inp;
            int is_error = 0;
            char *result;
            char line[256];

            if (!sp) {
                gui_add_line(&app_gui, GetString(MSG_CMD_AREXX_USAGE));
                gui_clear_input(&app_gui);
                return;
            }
            {
                int plen = sp - args;
                if (plen >= (int)sizeof(port)) plen = sizeof(port) - 1;
                memcpy(port, args, plen);
                port[plen] = '\0';
            }
            cmd = sp + 1;
            while (*cmd == ' ') cmd++;

            inp = cJSON_CreateObject();
            cJSON_AddStringToObject(inp, "port", port);
            cJSON_AddStringToObject(inp, "command", cmd);

            snprintf(line, sizeof(line), GetString(MSG_CMD_AREXX), port, cmd);
            gui_add_line(&app_gui, line);
            gui_set_status(&app_gui, GetString(MSG_STATUS_AREXX_SENDING));

            result = tool_execute("arexx_command", inp, &is_error);
            cJSON_Delete(inp);

            if (result) {
                gui_add_text(&app_gui, is_error ? "\033bError:\033n " : "\033bResult:\033n ",
                             result);
                free(result);
            }
            gui_add_line(&app_gui, "");
            gui_set_status(&app_gui, is_error ? GetString(MSG_CMD_FAILED) : GetString(MSG_CMD_DONE));
        }
        gui_clear_input(&app_gui);
        return;
    }

    /* /read <path> - test read_file tool directly */
    if (strncasecmp(input, "/read ", 6) == 0) {
        const char *path = input + 6;
        while (*path == ' ') path++;
        if (*path) {
            cJSON *inp = cJSON_CreateObject();
            int is_error = 0;
            char *result;
            char line[256];

            cJSON_AddStringToObject(inp, "path", path);
            snprintf(line, sizeof(line), GetString(MSG_CMD_READ), path);
            gui_add_line(&app_gui, line);

            result = tool_execute("read_file", inp, &is_error);
            cJSON_Delete(inp);

            if (result) {
                gui_add_text(&app_gui, is_error ? "\033bError:\033n " : "",
                             result);
                free(result);
            }
            gui_add_line(&app_gui, "");
            gui_set_status(&app_gui, is_error ? GetString(MSG_CMD_FAILED) : GetString(MSG_CMD_DONE));
        }
        gui_clear_input(&app_gui);
        return;
    }

    /* /write <path> <content> - test write_file tool directly */
    if (strncasecmp(input, "/write ", 7) == 0) {
        const char *args = input + 7;
        while (*args == ' ') args++;
        if (*args) {
            const char *sp = strchr(args, ' ');
            char path[256];
            cJSON *inp;
            int is_error = 0;
            char *result;

            if (!sp) {
                gui_add_line(&app_gui, GetString(MSG_CMD_WRITE_USAGE));
                gui_clear_input(&app_gui);
                return;
            }
            {
                int plen = sp - args;
                if (plen >= (int)sizeof(path)) plen = sizeof(path) - 1;
                memcpy(path, args, plen);
                path[plen] = '\0';
            }

            inp = cJSON_CreateObject();
            cJSON_AddStringToObject(inp, "path", path);
            cJSON_AddStringToObject(inp, "content", sp + 1);

            result = tool_execute("write_file", inp, &is_error);
            cJSON_Delete(inp);

            if (result) {
                gui_add_line(&app_gui, result);
                free(result);
            }
            gui_add_line(&app_gui, "");
            gui_set_status(&app_gui, is_error ? GetString(MSG_CMD_FAILED) : GetString(MSG_CMD_DONE));
        }
        gui_clear_input(&app_gui);
        return;
    }

    /* --- Normal message send --- */

    /* Copy input before clearing — gui_clear_input invalidates the
     * MUI string buffer pointer returned by gui_get_input. */
    {
        char input_copy[1024];
        strncpy(input_copy, input, sizeof(input_copy) - 1);
        input_copy[sizeof(input_copy) - 1] = '\0';

        /* Display user message */
        gui_add_text(&app_gui, GetString(MSG_LABEL_YOU), input_copy);
        chat_log("USER", input_copy);

        /* Clear input and set busy */
        gui_clear_input(&app_gui);
        gui_set_status(&app_gui, GetString(MSG_STATUS_SENDING));
        gui_set_busy(&app_gui, 1);

        /* Send to API */
        reply = claude_send(&app_claude, input_copy, &error_msg);
    }

    gui_set_busy(&app_gui, 0);

    if (app_gui.abort_requested) {
        /* User clicked Stop */
        gui_add_line(&app_gui, GetString(MSG_LABEL_ABORTED));
        gui_set_status(&app_gui, GetString(MSG_STATUS_ABORTED));
        chat_log("SYSTEM", "Request aborted by user");
        free(reply);
        free(error_msg);
    } else if (reply) {
        /* Display response */
        gui_add_text(&app_gui, GetString(MSG_LABEL_CLAUDE), reply);
        gui_add_line(&app_gui, "");
        chat_log("CLAUDE", reply);

        /* Show token usage in status bar */
        snprintf(status_buf, sizeof(status_buf),
                 GetString(MSG_STATUS_TOKENS),
                 app_claude.last_input_tokens,
                 app_claude.last_output_tokens,
                 claude_message_count(&app_claude));
        gui_set_status(&app_gui, status_buf);

        free(reply);
    } else {
        /* Display error */
        char err_buf[256];
        snprintf(err_buf, sizeof(err_buf), "%s%s",
                 GetString(MSG_LABEL_ERROR),
                 error_msg ? error_msg : GetString(MSG_ERR_UNKNOWN));
        gui_add_line(&app_gui, err_buf);
        gui_set_status(&app_gui, error_msg ? error_msg : GetString(MSG_STATUS_ERROR));
        chat_log("ERROR", error_msg ? error_msg : "Unknown error");
        free(error_msg);
    }
}

static void handle_new_chat(void)
{
    if (claude_clear_history(&app_claude) != 0) {
        gui_set_status(&app_gui, GetString(MSG_ERR_OOM_HISTORY));
        return;
    }
    gui_clear_chat(&app_gui);
    gui_add_line(&app_gui, GetString(MSG_LABEL_NEW_CHAT));
    gui_add_line(&app_gui, "");
    gui_set_status(&app_gui, GetString(MSG_STATUS_CHAT_CLEARED));
}

static void handle_about(void)
{
    char buf[512];
    snprintf(buf, sizeof(buf),
             "%s %s\n\n%s\n\n\xa9 2026 Thomas \xd6llinger\n\n%s\nARexx Port: AMIGAAI",
             PROGRAM_NAME, VERSION_STRING,
             GetString(MSG_ABOUT_DESCRIPTION),
             GetString(MSG_ABOUT_STACK));
    gui_about(&app_gui, PROGRAM_NAME, buf);
}

/* Safe non-variadic wrappers for MUI attribute access.
 * GCC 13 m68k breaks variadic inline stubs (set/get/DoMethod). */
static inline void xset(Object *obj, ULONG attr, ULONG val)
{
    struct TagItem tags[2];
    tags[0].ti_Tag = attr;
    tags[0].ti_Data = val;
    tags[1].ti_Tag = TAG_DONE;
    SetAttrsA(obj, tags);
}

static inline ULONG xget(Object *obj, ULONG attr)
{
    ULONG val = 0;
    GetAttr(attr, obj, &val);
    return val;
}

/* ===================== Memory handlers ===================== */

static void handle_memory_view(void)
{
    char *memstr = memory_to_string(&app_memory);
    if (memstr) {
        gui_about(&app_gui, GetString(MSG_MEM_TITLE_VIEW), memstr);
        free(memstr);
    } else {
        gui_about(&app_gui, GetString(MSG_MEM_TITLE_VIEW),
                  GetString(MSG_MEM_NONE_BODY));
    }
}

static const char *model_list[] = {
    "claude-sonnet-4-6",
    "claude-haiku-4-5-20251001",
    "claude-opus-4-6",
    NULL
};

static void handle_model_select(void)
{
    Object *win, *list, *ok_btn, *cancel_btn;
    Object *hgrp, *vgrp;
    ULONG open;
    ULONG sigs;
    int done = 0;
    int i, active = 0;
    struct IClass *wincl;

    /* Find currently active model in list */
    for (i = 0; model_list[i]; i++) {
        if (strcmp(model_list[i], app_config.model) == 0) {
            active = i;
            break;
        }
    }

    {
        ULONG ok_params[1];
        ok_params[0] = (ULONG)GetString(MSG_BTN_OK);
        ok_btn = MUI_MakeObjectA(MUIO_Button, ok_params);
    }
    {
        ULONG cancel_params[1];
        cancel_params[0] = (ULONG)GetString(MSG_BTN_CANCEL);
        cancel_btn = MUI_MakeObjectA(MUIO_Button, cancel_params);
    }
    if (!ok_btn || !cancel_btn) return;

    /* Create List with model entries */
    {
        struct TagItem tags[] = {
            { MUIA_Frame, MUIV_Frame_InputList },
            { MUIA_List_ConstructHook, MUIV_List_ConstructHook_String },
            { MUIA_List_DestructHook,  MUIV_List_DestructHook_String },
            { MUIA_List_SourceArray, (ULONG)model_list },
            { MUIA_CycleChain, (ULONG)TRUE },
            { TAG_DONE, 0 }
        };
        list = MUI_NewObjectA((CONST_STRPTR)MUIC_List, tags);
    }

    if (!list) return;

    /* Pre-select active model */
    xset(list, MUIA_List_Active, (ULONG)active);

    {
        struct TagItem tags[] = {
            { MUIA_Group_Horiz, TRUE },
            { MUIA_Group_Child, (ULONG)ok_btn },
            { MUIA_Group_Child, (ULONG)cancel_btn },
            { TAG_DONE, 0 }
        };
        hgrp = MUI_NewObjectA((CONST_STRPTR)MUIC_Group, tags);
    }

    {
        struct TagItem tags[] = {
            { MUIA_Group_Child, (ULONG)list },
            { MUIA_Group_Child, (ULONG)hgrp },
            { TAG_DONE, 0 }
        };
        vgrp = MUI_NewObjectA((CONST_STRPTR)MUIC_Group, tags);
    }

    if (!vgrp) return;

    wincl = MUI_GetClass((CONST_STRPTR)MUIC_Window);
    if (!wincl) {
        MUI_DisposeObject(vgrp);
        return;
    }
    {
        struct TagItem tags[] = {
            { MUIA_Window_Title, (ULONG)GetString(MSG_MODEL_TITLE) },
            { MUIA_Window_ID,    MAKE_ID('M','O','D','L') },
            { MUIA_Window_RootObject, (ULONG)vgrp },
            { TAG_DONE, 0 }
        };
        win = NewObjectA(wincl, NULL, tags);
    }
    MUI_FreeClass(wincl);

    if (!win) return;

    {
        ULONG msg[] = { OM_ADDMEMBER, (ULONG)win };
        DoMethodA(app_gui.app, (Msg)msg);
    }

    /* OK button */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Pressed, (ULONG)FALSE,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 100 };
        DoMethodA(ok_btn, (Msg)msg);
    }
    /* Double-click on list entry = OK */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Listview_DoubleClick, (ULONG)TRUE,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 100 };
        DoMethodA(list, (Msg)msg);
    }
    /* Cancel button */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Pressed, (ULONG)FALSE,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 101 };
        DoMethodA(cancel_btn, (Msg)msg);
    }
    /* Window close = Cancel */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Window_CloseRequest, (ULONG)TRUE,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 101 };
        DoMethodA(win, (Msg)msg);
    }

    xset(win, MUIA_Window_Open, (ULONG)TRUE);
    open = xget(win, MUIA_Window_Open);
    if (!open) {
        ULONG msg[] = { OM_REMMEMBER, (ULONG)win };
        DoMethodA(app_gui.app, (Msg)msg);
        MUI_DisposeObject(win);
        return;
    }

    while (!done) {
        ULONG mid;
        {
            ULONG msg[] = { MUIM_Application_NewInput, (ULONG)&sigs };
            mid = DoMethodA(app_gui.app, (Msg)msg);
        }
        switch (mid) {
        case 100: /* OK / Double-click */
        {
            LONG sel = (LONG)xget(list, MUIA_List_Active);
            if (sel >= 0) {
                char *entry = NULL;
                {
                    ULONG msg[] = { MUIM_List_GetEntry, (ULONG)sel, (ULONG)&entry };
                    DoMethodA(list, (Msg)msg);
                }
                if (entry) {
                    char buf[128];
                    strncpy(app_config.model, entry, CONFIG_MAX_MODEL_LEN - 1);
                    app_config.model[CONFIG_MAX_MODEL_LEN - 1] = '\0';
                    config_save(&app_config, 1);
                    snprintf(buf, sizeof(buf), GetString(MSG_MODEL_SET), app_config.model);
                    gui_set_status(&app_gui, buf);
                }
            }
            done = 1;
            break;
        }
        case 101: /* Cancel / Close */
            done = 1;
            break;
        case MUIV_Application_ReturnID_Quit:
            done = 1;
            break;
        }
        if (sigs && !done)
            sigs = Wait(sigs);
    }

    xset(win, MUIA_Window_Open, FALSE);
    {
        ULONG msg[] = { OM_REMMEMBER, (ULONG)win };
        DoMethodA(app_gui.app, (Msg)msg);
    }
    MUI_DisposeObject(win);
}

static void handle_memory_add(void)
{
    Object *win, *str, *ok_btn, *cancel_btn;
    Object *text_obj, *hgrp, *vgrp;
    ULONG open;
    ULONG sigs;
    int done = 0;
    struct IClass *wincl;

    /* Create buttons via MUI_MakeObjectA (non-variadic) */
    {
        ULONG ok_params[1];
        ok_params[0] = (ULONG)GetString(MSG_BTN_OK);
        ok_btn = MUI_MakeObjectA(MUIO_Button, ok_params);
    }
    {
        ULONG cancel_params[1];
        cancel_params[0] = (ULONG)GetString(MSG_BTN_CANCEL);
        cancel_btn = MUI_MakeObjectA(MUIO_Button, cancel_params);
    }
    if (!ok_btn || !cancel_btn) return;

    /* Create text label */
    {
        struct TagItem tags[] = {
            { MUIA_Text_Contents, (ULONG)GetString(MSG_MEM_ENTER_FACT) },
            { TAG_DONE, 0 }
        };
        text_obj = MUI_NewObjectA((CONST_STRPTR)MUIC_Text, tags);
    }

    /* Create string gadget */
    {
        struct TagItem tags[] = {
            { MUIA_Frame, MUIV_Frame_String },
            { MUIA_String_MaxLen, (ULONG)MEMORY_MAX_ENTRY_LEN },
            { MUIA_CycleChain, (ULONG)TRUE },
            { TAG_DONE, 0 }
        };
        str = MUI_NewObjectA((CONST_STRPTR)MUIC_String, tags);
    }

    /* Create HGroup for buttons */
    {
        struct TagItem tags[] = {
            { MUIA_Group_Horiz, TRUE },
            { MUIA_Group_Child, (ULONG)ok_btn },
            { MUIA_Group_Child, (ULONG)cancel_btn },
            { TAG_DONE, 0 }
        };
        hgrp = MUI_NewObjectA((CONST_STRPTR)MUIC_Group, tags);
    }

    /* Create VGroup */
    {
        struct TagItem tags[] = {
            { MUIA_Group_Child, (ULONG)text_obj },
            { MUIA_Group_Child, (ULONG)str },
            { MUIA_Group_Child, (ULONG)hgrp },
            { TAG_DONE, 0 }
        };
        vgrp = MUI_NewObjectA((CONST_STRPTR)MUIC_Group, tags);
    }

    if (!vgrp) return;

    /* Create Window via NewObject with IClass (bypasses broken variadic) */
    wincl = MUI_GetClass((CONST_STRPTR)MUIC_Window);
    if (!wincl) {
        MUI_DisposeObject(vgrp);
        return;
    }
    {
        struct TagItem tags[] = {
            { MUIA_Window_Title, (ULONG)GetString(MSG_MEM_TITLE_ADD) },
            { MUIA_Window_ID,    MAKE_ID('M','E','M','A') },
            { MUIA_Window_RootObject, (ULONG)vgrp },
            { TAG_DONE, 0 }
        };
        win = NewObjectA(wincl, NULL, tags);
    }
    MUI_FreeClass(wincl);

    if (!win) return;

    /* Add window to application */
    {
        ULONG msg[] = { OM_ADDMEMBER, (ULONG)win };
        DoMethodA(app_gui.app, (Msg)msg);
    }

    /* Button notifications */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Pressed, (ULONG)FALSE,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 100 };
        DoMethodA(ok_btn, (Msg)msg);
    }
    {
        ULONG msg[] = { MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 100 };
        DoMethodA(str, (Msg)msg);
    }
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Pressed, (ULONG)FALSE,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 101 };
        DoMethodA(cancel_btn, (Msg)msg);
    }
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Window_CloseRequest, (ULONG)TRUE,
                         (ULONG)app_gui.app, 2,
                         MUIM_Application_ReturnID, 101 };
        DoMethodA(win, (Msg)msg);
    }

    xset(win, MUIA_Window_Open, (ULONG)TRUE);
    open = xget(win, MUIA_Window_Open);
    if (!open) {
        ULONG msg[] = { OM_REMMEMBER, (ULONG)win };
        DoMethodA(app_gui.app, (Msg)msg);
        MUI_DisposeObject(win);
        return;
    }

    xset(win, MUIA_Window_ActiveObject, (ULONG)str);

    /* Mini event loop for modal dialog */
    while (!done) {
        ULONG mid;
        {
            ULONG msg[] = { MUIM_Application_NewInput, (ULONG)&sigs };
            mid = DoMethodA(app_gui.app, (Msg)msg);
        }
        switch (mid) {
        case 100: /* OK / Enter */
        {
            char *text = (char *)xget(str, MUIA_String_Contents);
            if (text && text[0]) {
                if (memory_add(&app_memory, text) == 0) {
                    memory_save(&app_memory);
                    gui_set_status(&app_gui, GetString(MSG_MEM_ADDED));
                } else {
                    gui_set_status(&app_gui, GetString(MSG_MEM_FULL));
                }
            }
            done = 1;
            break;
        }
        case 101: /* Cancel / Close */
            done = 1;
            break;
        case MUIV_Application_ReturnID_Quit:
            done = 1;
            break;
        }
        if (sigs && !done)
            sigs = Wait(sigs);
    }

    xset(win, MUIA_Window_Open, FALSE);
    {
        ULONG msg[] = { OM_REMMEMBER, (ULONG)win };
        DoMethodA(app_gui.app, (Msg)msg);
    }
    MUI_DisposeObject(win);
}

static void handle_memory_clear(void)
{
    LONG result;

    if (app_memory.count == 0) {
        gui_about(&app_gui, GetString(MSG_MEM_TITLE_CLEAR), GetString(MSG_MEM_NONE));
        return;
    }

    result = MUI_RequestA(app_gui.app, app_gui.win, 0,
                          (CONST_STRPTR)GetString(MSG_MEM_TITLE_CLEAR),
                          (CONST_STRPTR)GetString(MSG_MEM_CLEAR_BUTTONS),
                          (CONST_STRPTR)GetString(MSG_MEM_CLEAR_CONFIRM),
                          NULL);
    if (result == 1) {
        memory_clear(&app_memory);
        memory_save(&app_memory);
        gui_set_status(&app_gui, GetString(MSG_MEM_CLEARED));
    }
}

/* ===================== Chat save/load ===================== */

static void handle_chat_save(void)
{
    char *json_str;
    FILE *f;
    const char *filename = "AmigaAI:chat.json";

    if (claude_message_count(&app_claude) == 0) {
        gui_about(&app_gui, GetString(MSG_CHAT_SAVE_TITLE), GetString(MSG_CHAT_SAVE_NONE));
        return;
    }

    json_str = cJSON_Print(app_claude.messages);
    if (!json_str) {
        gui_set_status(&app_gui, GetString(MSG_CHAT_SAVE_FAIL));
        return;
    }

    f = fopen(filename, "w");
    if (f) {
        fputs(json_str, f);
        fclose(f);
        gui_set_status(&app_gui, GetString(MSG_CHAT_SAVE_OK));
    } else {
        gui_set_status(&app_gui, GetString(MSG_CHAT_SAVE_WRITE_FAIL));
    }
    cJSON_free(json_str);
}

static void handle_chat_load(void)
{
    FILE *f;
    char *buf;
    long len;
    cJSON *loaded;
    const char *filename = "AmigaAI:chat.json";

    f = fopen(filename, "r");
    if (!f) {
        gui_set_status(&app_gui, GetString(MSG_CHAT_LOAD_NONE));
        return;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len <= 0 || len > 256L * 1024L) {
        fclose(f);
        gui_set_status(&app_gui, GetString(MSG_CHAT_LOAD_TOO_LARGE));
        return;
    }

    buf = malloc(len + 1);
    if (!buf) {
        fclose(f);
        gui_set_status(&app_gui, GetString(MSG_CHAT_LOAD_OOM));
        return;
    }

    if ((long)fread(buf, 1, len, f) != len) {
        free(buf);
        fclose(f);
        gui_set_status(&app_gui, GetString(MSG_CHAT_LOAD_READ_FAIL));
        return;
    }
    buf[len] = '\0';
    fclose(f);

    loaded = cJSON_Parse(buf);
    free(buf);

    if (loaded && cJSON_IsArray(loaded)) {
        /* Replace conversation */
        cJSON_Delete(app_claude.messages);
        app_claude.messages = loaded;

        /* Rebuild the chat display */
        gui_clear_chat(&app_gui);
        gui_add_line(&app_gui, GetString(MSG_CHAT_LOADED_LINE));
        gui_add_line(&app_gui, "");

        /* Replay messages into display */
        {
            int i, count = cJSON_GetArraySize(loaded);
            for (i = 0; i < count; i++) {
                cJSON *msg = cJSON_GetArrayItem(loaded, i);
                cJSON *role = cJSON_GetObjectItemCaseSensitive(msg, "role");
                cJSON *content = cJSON_GetObjectItemCaseSensitive(msg, "content");
                if (role && content &&
                    cJSON_IsString(role) && cJSON_IsString(content))
                {
                    if (strcmp(role->valuestring, "user") == 0)
                        gui_add_text(&app_gui, GetString(MSG_LABEL_YOU),
                                     content->valuestring);
                    else
                        gui_add_text(&app_gui, GetString(MSG_LABEL_CLAUDE),
                                     content->valuestring);
                    gui_add_line(&app_gui, "");
                }
            }
        }

        {
            char buf2[64];
            snprintf(buf2, sizeof(buf2), GetString(MSG_CHAT_LOADED),
                     claude_message_count(&app_claude));
            gui_set_status(&app_gui, buf2);
        }
    } else {
        if (loaded) cJSON_Delete(loaded);
        gui_set_status(&app_gui, GetString(MSG_CHAT_LOAD_PARSE_FAIL));
    }
}

/* ========================= main ========================= */

int main(int argc, char *argv[])
{
    ULONG sigs = 0;
    ULONG id;
    ULONG arexx_sig;
    int   running = 1;
    const char *prog_name = PROGRAM_NAME;

    (void)verstag;

    /* === Workbench / CLI detection === */
    if (argc == 0) {
        /* Launched from Workbench */
        struct WBStartup *wbs = (struct WBStartup *)argv;
        from_wb = 1;

        /* Change to the program's directory and set PROGDIR: */
        if (wbs->sm_NumArgs > 0) {
            struct Process *pr = (struct Process *)FindTask(NULL);
            old_dir = CurrentDir(wbs->sm_ArgList[0].wa_Lock);
            prog_name = (const char *)wbs->sm_ArgList[0].wa_Name;

            /* Set pr_HomeDir (= PROGDIR:) to the program's directory.
             * Workbench doesn't always set this correctly. */
            if (pr) {
                BPTR home = DupLock(wbs->sm_ArgList[0].wa_Lock);
                if (home) {
                    old_home = pr->pr_HomeDir;
                    pr->pr_HomeDir = home;
                    our_home = home;
                }
            }
        }

        /* Read ToolTypes from program icon */
        {
            struct DiskObject *dobj;
            IconBase = OpenLibrary((CONST_STRPTR)"icon.library", 36);
            if (IconBase) {
                dobj = GetDiskObject((CONST_STRPTR)prog_name);
                if (dobj) {
                    char *tt;
                    tt = (char *)FindToolType(
                            (CONST_STRPTR *)dobj->do_ToolTypes,
                            (CONST_STRPTR)"APILOG");
                    if (tt) {
                        strncpy(api_log_file, tt,
                                sizeof(api_log_file) - 1);
                        api_log_file[sizeof(api_log_file) - 1] = '\0';
                    }
                    FreeDiskObject(dobj);
                }
                CloseLibrary(IconBase);
                IconBase = NULL;
            }
        }

        /* No console from Workbench - send output to NIL: */
        freopen("NIL:", "w", stdout);
        freopen("NIL:", "w", stderr);
    } else {
        /* CLI mode: parse arguments with ReadArgs */
        {
            /* Template: CREATEICON/S,APILOG/K */
            #define TEMPLATE "CREATEICON/S,APILOG/K"
            enum { ARG_CREATEICON, ARG_APILOG, ARG_COUNT };
            LONG args[ARG_COUNT] = { 0, 0 };
            struct RDArgs *rda;

            rda = ReadArgs((CONST_STRPTR)TEMPLATE, args, NULL);
            if (rda) {
                if (args[ARG_CREATEICON]) {
                    prog_name = argv[0];
                    create_icon(prog_name);
                    printf("Icon created for %s\n", prog_name);
                    FreeArgs(rda);
                    return 0;
                }
                if (args[ARG_APILOG]) {
                    strncpy(api_log_file, (char *)args[ARG_APILOG],
                            sizeof(api_log_file) - 1);
                    api_log_file[sizeof(api_log_file) - 1] = '\0';
                }
                FreeArgs(rda);
            }
        }

        prog_name = argv[0];
    }

    /* Add standard directories to command search path */
    setup_search_path();

    /* Create AmigaAI: assign pointing to the program's directory.
     * This gives shell commands and Claude a stable way to reference
     * the application directory (PROGDIR: doesn't work in child processes). */
    {
        struct Process *pr = (struct Process *)FindTask(NULL);
        BPTR cur = pr ? pr->pr_CurrentDir : 0;
        if (cur) {
            BPTR lock = DupLock(cur);
            if (lock) {
                if (AssignLock((CONST_STRPTR)"AmigaAI", lock))
                    assign_ok = 1;
                else
                    UnLock(lock);  /* AssignLock failed, free the lock */
            }
        }
    }

    /* Force unbuffered stdout so crash diagnostics are not lost */
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("%s %s starting...\n", PROGRAM_NAME, VERSION_STRING);

    /* Log session start */
    {
        char sbuf[128];
        snprintf(sbuf, sizeof(sbuf), "--- %s %s session start ---",
                 PROGRAM_NAME, VERSION_STRING);
        chat_log("SYSTEM", sbuf);
    }

    /* Open Amiga libraries */
    dbg_step(1, "Opening libraries...");
    if (open_libraries() != 0) {
        close_libraries();
        if (from_wb && old_dir) CurrentDir(old_dir);
        return 20;
    }
    dbg_step(2, "Libraries OK");

    /* Initialize locale for translations */
    locale_open();

    /* Initialize HTTP/SSL subsystem */
    dbg_step(3, "Init HTTP/SSL...");
    if (http_init() != 0) {
        if (from_wb)
            wb_error("Failed to initialize HTTP/SSL.\n"
                     "Please install Roadshow and AmiSSL v5.");
        else
            printf("ERROR: Failed to initialize HTTP/SSL\n");
        close_libraries();
        if (from_wb && old_dir) CurrentDir(old_dir);
        return 20;
    }
    dbg_step(4, "HTTP/SSL OK");

    /* Enable API logging if requested */
    if (api_log_file[0]) {
        http_set_api_log(api_log_file);
        printf("  API log: %s\n", api_log_file);
    }

    /* Load configuration */
    dbg_step(5, "Loading config...");
    if (!config_load(&app_config)) {
        printf("WARNING: No API key found.\n");
        printf("Set it with: echo \"sk-ant-...\" > ENV:AmigaAI/api_key\n");
    }
    dbg_step(6, "Config OK");

    /* Load persistent memory */
    dbg_step(7, "Loading memory...");
    {
        int mem_count = memory_load(&app_memory);
        printf("  Loaded %d memory entries\n", mem_count);
    }
    dbg_step(8, "Memory OK");

    /* Initialize Claude API */
    dbg_step(9, "Init Claude API...");
    if (claude_init(&app_claude, &app_config, &app_memory) != 0) {
        if (from_wb)
            wb_error("Failed to initialize Claude API.");
        else
            printf("ERROR: Failed to initialize Claude API\n");
        http_cleanup();
        close_libraries();
        if (from_wb && old_dir) CurrentDir(old_dir);
        return 20;
    }
    claude_set_tool_callback(&app_claude, tool_status_cb, NULL);
    http_set_event_callback(http_poll_cb, NULL);
    tools_set_poll_callback(http_poll_cb, NULL);
    dbg_step(10, "Claude OK");

    /* Open MUI GUI */
    dbg_step(11, "Opening MUI GUI...");
    if (gui_open(&app_gui) != 0) {
        if (from_wb)
            wb_error("Failed to open GUI.\nIs MUI installed?");
        else
            printf("ERROR: Failed to open GUI (MUI installed?)\n");
        claude_cleanup(&app_claude);
        http_cleanup();
        close_libraries();
        if (from_wb && old_dir) CurrentDir(old_dir);
        return 20;
    }
    dbg_step(12, "GUI OK");

    /* Initialize ARexx port */
    dbg_step(13, "Init ARexx...");
    if (arexx_init(&app_arexx, &app_claude, arexx_response_cb) != 0) {
        printf("WARNING: ARexx port not available\n");
    }
    arexx_sig = arexx_signal(&app_arexx);
    dbg_step(14, "All init done - entering main loop");

    /* From Workbench, create icon if it doesn't exist yet */
    if (from_wb)
        create_icon(prog_name);

    /* Check for missing API key */
    if (!app_config.api_key[0]) {
        gui_add_line(&app_gui, GetString(MSG_WARN_NO_APIKEY));
        gui_add_line(&app_gui, GetString(MSG_WARN_SET_APIKEY));
        gui_add_line(&app_gui, "");
        gui_set_status(&app_gui, GetString(MSG_STATUS_NO_APIKEY));
    }

    /* Show memory count on startup */
    if (app_memory.count > 0) {
        char mbuf[64];
        snprintf(mbuf, sizeof(mbuf), GetString(MSG_WARN_MEM_LOADED), app_memory.count);
        gui_add_line(&app_gui, mbuf);
        gui_add_line(&app_gui, "");
    }

    /* === MUI Application Main Loop === */
    while (running) {
        id = gui_process(&app_gui, &sigs);

        switch (id) {
        case MUIV_Application_ReturnID_Quit:
            running = 0;
            break;

        case GUI_ID_SEND:
            handle_send();
            break;

        case GUI_ID_NEW:
            handle_new_chat();
            break;

        case GUI_ID_ABOUT:
            handle_about();
            break;

        case GUI_ID_APIKEY:
            /* TODO: Open API key requester */
            gui_set_status(&app_gui, GetString(MSG_APIKEY_HINT));
            break;

        case GUI_ID_MODEL:
            handle_model_select();
            break;

        case GUI_ID_SYSTEM:
            /* TODO: Open system prompt editor */
            gui_set_status(&app_gui, GetString(MSG_SYSTEM_COMING_SOON));
            break;

        /* Memory menu */
        case GUI_ID_MEMVIEW:
            handle_memory_view();
            break;

        case GUI_ID_MEMADD:
            handle_memory_add();
            break;

        case GUI_ID_MEMCLEAR:
            handle_memory_clear();
            break;

        /* Chat save/load */
        case GUI_ID_CHATSAVE:
            handle_chat_save();
            break;

        case GUI_ID_CHATLOAD:
            handle_chat_load();
            break;
        }

        /* Re-activate input field after any action.
         * Done here (outside MUI event processing) because
         * SetAttrsA for MUIA_Window_ActiveObject is unreliable
         * when called from within notification handlers. */
        if (id && id != MUIV_Application_ReturnID_Quit && !app_gui.busy)
            gui_focus_input(&app_gui);

        if (sigs && running) {
            sigs = Wait(sigs | arexx_sig | SIGBREAKF_CTRL_C);

            /* CTRL-C from shell */
            if (sigs & SIGBREAKF_CTRL_C)
                running = 0;

            /* ARexx events */
            if (arexx_sig && (sigs & arexx_sig)) {
                if (arexx_handle(&app_arexx))
                    running = 0;
            }
        }
    }

    /* Cleanup */
    printf("Shutting down...\n");

    arexx_cleanup(&app_arexx);
    gui_close(&app_gui);
    claude_cleanup(&app_claude);
    http_cleanup();
    locale_close();
    close_libraries();
    cleanup_search_path();

    /* Remove AmigaAI: assign */
    if (assign_ok)
        AssignLock((CONST_STRPTR)"AmigaAI", 0);

    /* Restore original directory and PROGDIR: if launched from Workbench */
    if (from_wb) {
        if (our_home) {
            struct Process *pr = (struct Process *)FindTask(NULL);
            if (pr) pr->pr_HomeDir = old_home;
            UnLock(our_home);
        }
        if (old_dir)
            CurrentDir(old_dir);
    }

    printf("%s terminated.\n", PROGRAM_NAME);
    return 0;
}
