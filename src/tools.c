/*
 * tools.c - Agent tool execution for AmigaAI
 *
 * Implements tool definitions and execution for Claude's tool use API.
 * Tools: shell_command, arexx_command, read_file, write_file
 */

#include "tools.h"
#include "dt_identify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/rexxsyslib.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <utility/tagitem.h>

/* Temp file for capturing shell command output */
#define TOOL_CMD_OUTPUT "T:amigaai_cmd.out"

/* Poll callback for async shell execution */
static ToolPollCallback tool_poll_cb = NULL;
static void *tool_poll_data = NULL;

void tools_set_poll_callback(ToolPollCallback cb, void *userdata)
{
    tool_poll_cb = cb;
    tool_poll_data = userdata;
}

/* Shared state between parent and child process for async shell */
struct ShellTask {
    const char   *command;
    const char   *outfile;
    LONG          rc;
    volatile BYTE done;
    volatile BYTE abandoned;  /* parent gave up waiting (background mode) */
    struct Task  *parent;
    BYTE          sigbit;
    BPTR          parent_path; /* parent's cli_CommandDir to copy */
};

/* Copy the parent's CLI command search path into the child's CLI.
 * Walks the parent's BPTR-linked path list and DupLocks each entry. */
static void copy_parent_path(BPTR parent_cmd_dir)
{
    struct Process *pr = (struct Process *)FindTask(NULL);
    struct CommandLineInterface *cli;
    LONG *src;

    if (!pr || !pr->pr_CLI) return;
    cli = BADDR(pr->pr_CLI);
    if (!cli) return;

    /* Walk parent's path list and duplicate each entry */
    src = parent_cmd_dir ? (LONG *)BADDR(parent_cmd_dir) : NULL;
    while (src) {
        BPTR lock = DupLock((BPTR)src[1]);
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
        src = src[0] ? (LONG *)BADDR((BPTR)src[0]) : NULL;
    }
}

/* Free all path nodes in the current process's CLI path list */
static void cleanup_child_path(void)
{
    struct Process *pr = (struct Process *)FindTask(NULL);
    struct CommandLineInterface *cli;

    if (!pr || !pr->pr_CLI) return;
    cli = BADDR(pr->pr_CLI);
    if (!cli) return;

    while (cli->cli_CommandDir) {
        LONG *node = (LONG *)BADDR(cli->cli_CommandDir);
        cli->cli_CommandDir = (BPTR)node[0];
        UnLock((BPTR)node[1]);
        FreeVec(node);
    }
}

/* Child process entry point for async shell execution */
static void shell_child_entry(void)
{
    struct Process *me = (struct Process *)FindTask(NULL);
    struct ShellTask *st = (struct ShellTask *)me->pr_ExitData;
    BPTR outfh;

    /* Copy parent's command search path so programs are found */
    copy_parent_path(st->parent_path);

    outfh = Open((CONST_STRPTR)st->outfile, MODE_NEWFILE);
    if (outfh) {
        struct TagItem sys_tags[] = {
            { SYS_Output, (ULONG)outfh },
            { SYS_Input,  (ULONG)NULL },
            { TAG_DONE,   0 }
        };
        st->rc = SystemTagList((CONST_STRPTR)st->command, sys_tags);
        Close(outfh);
    } else {
        st->rc = -1;
    }

    cleanup_child_path();

    st->done = 1;

    /* Synchronize with parent using Forbid to avoid race conditions */
    Forbid();
    if (st->abandoned) {
        /* Parent gave up waiting — we own st, clean up.
         * outfile is a string literal, do NOT free it. */
        Permit();
        DeleteFile((CONST_STRPTR)st->outfile);
        free((void *)st->command);
        free(st);
    } else {
        Signal(st->parent, 1UL << st->sigbit);
        Permit();
    }
}

/* ===================== Tool JSON definitions ===================== */

cJSON *tools_build_json(void)
{
    cJSON *tools = cJSON_CreateArray();
    if (!tools) return NULL;

    /* Tool 1: shell_command */
    {
        cJSON *tool = cJSON_CreateObject();
        cJSON *schema = cJSON_CreateObject();
        cJSON *props = cJSON_CreateObject();
        cJSON *cmd_prop = cJSON_CreateObject();
        cJSON *req = cJSON_CreateArray();

        cJSON_AddStringToObject(tool, "name", "shell_command");
        cJSON_AddStringToObject(tool, "description",
            "Execute an AmigaDOS shell command and return its output. "
            "Use for running programs, listing files, checking system state. "
            "Examples: 'list SYS:', 'version', 'assign', 'type S:Startup-Sequence'. "
            "Set background=true for interactive/GUI programs (editors, viewers) "
            "that the user will interact with, so AmigaAI does not block.");

        cJSON_AddStringToObject(cmd_prop, "type", "string");
        cJSON_AddStringToObject(cmd_prop, "description",
            "The AmigaDOS command to execute");
        cJSON_AddItemToObject(props, "command", cmd_prop);

        {
            cJSON *bg_prop = cJSON_CreateObject();
            cJSON_AddStringToObject(bg_prop, "type", "boolean");
            cJSON_AddStringToObject(bg_prop, "description",
                "Launch program in background (returns immediately). "
                "Use for interactive/GUI programs like Ed, MultiView.");
            cJSON_AddItemToObject(props, "background", bg_prop);
        }

        cJSON_AddStringToObject(schema, "type", "object");
        cJSON_AddItemToObject(schema, "properties", props);
        cJSON_AddItemToArray(req, cJSON_CreateString("command"));
        cJSON_AddItemToObject(schema, "required", req);

        cJSON_AddItemToObject(tool, "input_schema", schema);
        cJSON_AddItemToArray(tools, tool);
    }

    /* Tool 2: arexx_command */
    {
        cJSON *tool = cJSON_CreateObject();
        cJSON *schema = cJSON_CreateObject();
        cJSON *props = cJSON_CreateObject();
        cJSON *port_prop = cJSON_CreateObject();
        cJSON *cmd_prop = cJSON_CreateObject();
        cJSON *req = cJSON_CreateArray();

        cJSON_AddStringToObject(tool, "name", "arexx_command");
        cJSON_AddStringToObject(tool, "description",
            "Send an ARexx command to a named ARexx port. "
            "Use to control running Amiga applications that have ARexx ports. "
            "Common ports: WORKBENCH, MULTIVIEW, IBROWSE, REXX, DOPUS.1. "
            "The command is sent and the result string returned.");

        cJSON_AddStringToObject(port_prop, "type", "string");
        cJSON_AddStringToObject(port_prop, "description",
            "Target ARexx port name (e.g. MULTIVIEW, REXX)");
        cJSON_AddItemToObject(props, "port", port_prop);

        cJSON_AddStringToObject(cmd_prop, "type", "string");
        cJSON_AddStringToObject(cmd_prop, "description",
            "The ARexx command to send");
        cJSON_AddItemToObject(props, "command", cmd_prop);

        cJSON_AddStringToObject(schema, "type", "object");
        cJSON_AddItemToObject(schema, "properties", props);
        cJSON_AddItemToArray(req, cJSON_CreateString("port"));
        cJSON_AddItemToArray(req, cJSON_CreateString("command"));
        cJSON_AddItemToObject(schema, "required", req);

        cJSON_AddItemToObject(tool, "input_schema", schema);
        cJSON_AddItemToArray(tools, tool);
    }

    /* Tool 3: read_file */
    {
        cJSON *tool = cJSON_CreateObject();
        cJSON *schema = cJSON_CreateObject();
        cJSON *props = cJSON_CreateObject();
        cJSON *path_prop = cJSON_CreateObject();
        cJSON *req = cJSON_CreateArray();

        cJSON_AddStringToObject(tool, "name", "read_file");
        cJSON_AddStringToObject(tool, "description",
            "Read the contents of a file. Returns the file content as text. "
            "Use AmigaDOS paths like SYS:, WORK:, S:, RAM:, AmigaAI: etc. "
            "Output is truncated to 4KB.");

        cJSON_AddStringToObject(path_prop, "type", "string");
        cJSON_AddStringToObject(path_prop, "description",
            "AmigaDOS file path (e.g. S:Startup-Sequence, RAM:test.txt)");
        cJSON_AddItemToObject(props, "path", path_prop);

        cJSON_AddStringToObject(schema, "type", "object");
        cJSON_AddItemToObject(schema, "properties", props);
        cJSON_AddItemToArray(req, cJSON_CreateString("path"));
        cJSON_AddItemToObject(schema, "required", req);

        cJSON_AddItemToObject(tool, "input_schema", schema);
        cJSON_AddItemToArray(tools, tool);
    }

    /* Tool 4: write_file */
    {
        cJSON *tool = cJSON_CreateObject();
        cJSON *schema = cJSON_CreateObject();
        cJSON *props = cJSON_CreateObject();
        cJSON *path_prop = cJSON_CreateObject();
        cJSON *content_prop = cJSON_CreateObject();
        cJSON *req = cJSON_CreateArray();

        cJSON_AddStringToObject(tool, "name", "write_file");
        cJSON_AddStringToObject(tool, "description",
            "Write content to a file, creating or overwriting it. "
            "Use AmigaDOS paths. Be careful with system files!");

        cJSON_AddStringToObject(path_prop, "type", "string");
        cJSON_AddStringToObject(path_prop, "description",
            "AmigaDOS file path to write to");
        cJSON_AddItemToObject(props, "path", path_prop);

        cJSON_AddStringToObject(content_prop, "type", "string");
        cJSON_AddStringToObject(content_prop, "description",
            "Content to write to the file");
        cJSON_AddItemToObject(props, "content", content_prop);

        cJSON_AddStringToObject(schema, "type", "object");
        cJSON_AddItemToObject(schema, "properties", props);
        cJSON_AddItemToArray(req, cJSON_CreateString("path"));
        cJSON_AddItemToArray(req, cJSON_CreateString("content"));
        cJSON_AddItemToObject(schema, "required", req);

        cJSON_AddItemToObject(tool, "input_schema", schema);
        cJSON_AddItemToArray(tools, tool);
    }

    /* Tool 5: list_ports */
    {
        cJSON *tool = cJSON_CreateObject();
        cJSON *schema = cJSON_CreateObject();
        cJSON *props = cJSON_CreateObject();

        cJSON_AddStringToObject(tool, "name", "list_ports");
        cJSON_AddStringToObject(tool, "description",
            "List all public Exec message ports (including ARexx ports). "
            "Use this to discover which applications are running and have "
            "ARexx ports available for sending commands to.");

        cJSON_AddStringToObject(schema, "type", "object");
        cJSON_AddItemToObject(schema, "properties", props);

        cJSON_AddItemToObject(tool, "input_schema", schema);
        cJSON_AddItemToArray(tools, tool);
    }

    /* Tool 6: identify_file */
    {
        cJSON *tool = cJSON_CreateObject();
        cJSON *schema = cJSON_CreateObject();
        cJSON *props = cJSON_CreateObject();
        cJSON *path_prop = cJSON_CreateObject();
        cJSON *filter_prop = cJSON_CreateObject();
        cJSON *recursive_prop = cJSON_CreateObject();
        cJSON *maxresults_prop = cJSON_CreateObject();
        cJSON *req = cJSON_CreateArray();

        cJSON_AddStringToObject(tool, "name", "identify_file");
        cJSON_AddStringToObject(tool, "description",
            "Identify file types using the AmigaOS DataType system. "
            "Can identify a single file or list all files in a directory. "
            "Optionally filter by type group (picture, text, sound, music, "
            "document, animation, movie, system) or by specific DataType "
            "name (e.g. ILBM, JPEG, PNG, ASCII, Protracker, AIFF). "
            "Set recursive to true to scan subdirectories. "
            "IMPORTANT: When the user asks for the first N files or a limited "
            "number of results, always set max_results accordingly.");

        cJSON_AddStringToObject(path_prop, "type", "string");
        cJSON_AddStringToObject(path_prop, "description",
            "AmigaDOS file or directory path");
        cJSON_AddItemToObject(props, "path", path_prop);

        cJSON_AddStringToObject(filter_prop, "type", "string");
        cJSON_AddStringToObject(filter_prop, "description",
            "Filter by group (picture, text, sound, music, "
            "document, animation, movie, system) or by "
            "specific DataType name (e.g. ILBM, JPEG, ASCII)");
        cJSON_AddItemToObject(props, "filter", filter_prop);

        cJSON_AddStringToObject(recursive_prop, "type", "boolean");
        cJSON_AddStringToObject(recursive_prop, "description",
            "Scan subdirectories recursively (default: false)");
        cJSON_AddItemToObject(props, "recursive", recursive_prop);

        cJSON_AddStringToObject(maxresults_prop, "type", "integer");
        cJSON_AddStringToObject(maxresults_prop, "description",
            "Limit results to this number of matching files. "
            "Required when the user specifies a count, e.g. "
            "'show me 20 images' -> max_results=20. Default: 0 (unlimited).");
        cJSON_AddItemToObject(props, "max_results", maxresults_prop);

        cJSON_AddStringToObject(schema, "type", "object");
        cJSON_AddItemToObject(schema, "properties", props);
        cJSON_AddItemToArray(req, cJSON_CreateString("path"));
        cJSON_AddItemToObject(schema, "required", req);

        cJSON_AddItemToObject(tool, "input_schema", schema);
        cJSON_AddItemToArray(tools, tool);
    }

    return tools;
}

/* ===================== Tool execution ===================== */

/* List all public Exec message ports */
static char *tool_exec_list_ports(int *is_error)
{
    char *result;
    int pos = 0;
    int cap = 2048;
    struct Node *node;

    (void)is_error;

    result = malloc(cap);
    if (!result) {
        *is_error = 1;
        return strdup("Out of memory");
    }
    result[0] = '\0';

    Forbid();
    for (node = SysBase->PortList.lh_Head;
         node->ln_Succ != NULL;
         node = node->ln_Succ)
    {
        if (node->ln_Name && node->ln_Name[0]) {
            int nlen = strlen(node->ln_Name);
            if (pos + nlen + 2 < cap) {
                if (pos > 0) result[pos++] = '\n';
                memcpy(result + pos, node->ln_Name, nlen);
                pos += nlen;
                result[pos] = '\0';
            }
        }
    }
    Permit();

    if (pos == 0) {
        free(result);
        return strdup("(no public ports found)");
    }

    return result;
}

/* Execute an AmigaDOS shell command, capture output */
/* Run shell command synchronously (fallback if async setup fails) */
static LONG shell_exec_sync(const char *command)
{
    LONG rc;
    BPTR outfh = Open((CONST_STRPTR)TOOL_CMD_OUTPUT, MODE_NEWFILE);
    if (!outfh) return -1;
    {
        struct TagItem sys_tags[] = {
            { SYS_Output, (ULONG)outfh },
            { SYS_Input,  (ULONG)NULL },
            { TAG_DONE,   0 }
        };
        rc = SystemTagList((CONST_STRPTR)command, sys_tags);
    }
    Close(outfh);
    return rc;
}

static char *tool_exec_shell(cJSON *input, int *is_error)
{
    cJSON *cmd_json;
    const char *command;
    LONG rc;
    FILE *f;
    char *result;
    long len;

    cmd_json = cJSON_GetObjectItemCaseSensitive(input, "command");
    if (!cmd_json || !cJSON_IsString(cmd_json) || !cmd_json->valuestring[0]) {
        *is_error = 1;
        return strdup("Missing 'command' parameter");
    }
    command = cmd_json->valuestring;

    /* Background mode: launch in child process, wait briefly for errors */
    {
        cJSON *bg_json = cJSON_GetObjectItemCaseSensitive(input, "background");
        if (bg_json && cJSON_IsTrue(bg_json)) {
            struct ShellTask *st;
            struct Process *child;
            BYTE sigbit;
            int i;

            printf("  [tool] shell (background): %s\n", command);

            sigbit = AllocSignal(-1);
            if (sigbit < 0) {
                *is_error = 1;
                return strdup("Cannot allocate signal");
            }

            st = malloc(sizeof(*st));
            if (!st) {
                FreeSignal(sigbit);
                *is_error = 1;
                return strdup("Out of memory");
            }

            st->command   = strdup(command);
            st->outfile   = "T:amigaai_bg.out";
            st->rc        = 0;
            st->done      = 0;
            st->abandoned   = 0;
            st->parent      = FindTask(NULL);
            st->sigbit      = sigbit;
            {
                struct Process *me = (struct Process *)FindTask(NULL);
                struct CommandLineInterface *cli = me->pr_CLI ? BADDR(me->pr_CLI) : NULL;
                st->parent_path = cli ? cli->cli_CommandDir : 0;
            }

            {
                struct Process *me = (struct Process *)FindTask(NULL);
                BPTR dup_cur  = me->pr_CurrentDir  ? DupLock(me->pr_CurrentDir) : 0;
                BPTR dup_home = me->pr_HomeDir ? DupLock(me->pr_HomeDir) : 0;
                struct TagItem np_tags[] = {
                    { NP_Entry,      (ULONG)shell_child_entry },
                    { NP_Name,       (ULONG)"AmigaAI Background" },
                    { NP_StackSize,  65536 },
                    { NP_ExitData,   (ULONG)st },
                    { NP_CurrentDir, (ULONG)dup_cur },
                    { NP_HomeDir,    (ULONG)dup_home },
                    { NP_Cli,        TRUE },
                    { TAG_DONE,      0 }
                };
                child = CreateNewProcTagList(np_tags);
                if (!child) {
                    if (dup_cur)  UnLock(dup_cur);
                    if (dup_home) UnLock(dup_home);
                    FreeSignal(sigbit);
                    free((void *)st->command);
                    free(st);
                    *is_error = 1;
                    return strdup("Cannot create background process");
                }
            }

            /* Wait up to ~2 seconds for quick errors, process MUI events */
            for (i = 0; i < 20 && !st->done; i++) {
                Delay(5);  /* 0.1 second */
                if (tool_poll_cb) tool_poll_cb(tool_poll_data);
            }

            Forbid();
            if (st->done) {
                /* Command finished quickly — read output (may contain error) */
                Permit();
                rc = st->rc;
                FreeSignal(sigbit);
                free((void *)st->command);
                free(st);
                /* Read the output file */
                {
                    FILE *bgf = fopen("T:amigaai_bg.out", "r");
                    if (bgf) {
                        long blen;
                        fseek(bgf, 0, SEEK_END);
                        blen = ftell(bgf);
                        fseek(bgf, 0, SEEK_SET);
                        if (blen > 0 && blen < TOOLS_MAX_OUTPUT) {
                            result = malloc(blen + 1);
                            if (result) {
                                fread(result, 1, blen, bgf);
                                result[blen] = '\0';
                            }
                        }
                        fclose(bgf);
                    }
                    DeleteFile((CONST_STRPTR)"T:amigaai_bg.out");
                }
                if (rc != 0) *is_error = 1;
                return result ? result :
                       strdup(rc ? "Command failed" : "OK");
            } else {
                /* Still running — it's a long-running/interactive program */
                st->abandoned = 1;
                Permit();
                FreeSignal(sigbit);
                return strdup("Program launched in background");
            }
        }
    }

    printf("  [tool] shell: %s\n", command);

    /* Try async execution so MUI stays responsive */
    {
        BYTE sigbit = AllocSignal(-1);

        if (sigbit >= 0) {
            struct ShellTask st;
            struct Process *child;

            st.command     = command;
            st.outfile     = TOOL_CMD_OUTPUT;
            st.rc          = 0;
            st.done        = 0;
            st.abandoned   = 0;
            st.parent      = FindTask(NULL);
            st.sigbit      = sigbit;
            {
                struct Process *me = (struct Process *)FindTask(NULL);
                struct CommandLineInterface *cli = me->pr_CLI ? BADDR(me->pr_CLI) : NULL;
                st.parent_path = cli ? cli->cli_CommandDir : 0;
            }

            {
                struct Process *me = (struct Process *)FindTask(NULL);
                BPTR cur_dir = me->pr_CurrentDir;
                BPTR home_dir = me->pr_HomeDir;
                BPTR dup_cur = cur_dir ? DupLock(cur_dir) : 0;
                BPTR dup_home = home_dir ? DupLock(home_dir) : 0;
                struct TagItem np_tags[] = {
                    { NP_Entry,      (ULONG)shell_child_entry },
                    { NP_Name,       (ULONG)"AmigaAI Shell" },
                    { NP_StackSize,  65536 },
                    { NP_ExitData,   (ULONG)&st },
                    { NP_CurrentDir, (ULONG)dup_cur },
                    { NP_HomeDir,    (ULONG)dup_home },
                    { NP_Cli,        TRUE },
                    { TAG_DONE,      0 }
                };
                child = CreateNewProcTagList(np_tags);
                if (!child) {
                    /* Clean up locks if proc creation fails */
                    if (dup_cur)  UnLock(dup_cur);
                    if (dup_home) UnLock(dup_home);
                }
            }

            if (child) {
                /* Poll loop: process MUI events while child runs */
                while (!st.done) {
                    ULONG sigs = Wait((1UL << sigbit) | SIGBREAKF_CTRL_C);

                    if (sigs & SIGBREAKF_CTRL_C) {
                        Signal((struct Task *)child, SIGBREAKF_CTRL_C);
                    }

                    if (!st.done && tool_poll_cb &&
                        tool_poll_cb(tool_poll_data))
                    {
                        /* Stop requested - send CTRL-C and wait */
                        Signal((struct Task *)child, SIGBREAKF_CTRL_C);
                        while (!st.done)
                            Wait(1UL << sigbit);
                    }
                }

                rc = st.rc;
                FreeSignal(sigbit);
                goto read_output;
            }

            FreeSignal(sigbit);
        }

        /* Fallback: synchronous execution */
        printf("  [tool] shell: async failed, running synchronously\n");
        rc = shell_exec_sync(command);
    }

read_output:

    /* Read the output */
    f = fopen(TOOL_CMD_OUTPUT, "r");
    if (!f) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Command exited with code %ld (no output)", rc);
        DeleteFile((CONST_STRPTR)TOOL_CMD_OUTPUT);
        if (rc != 0) *is_error = 1;
        return strdup(buf);
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len <= 0) {
        fclose(f);
        DeleteFile((CONST_STRPTR)TOOL_CMD_OUTPUT);
        if (rc != 0) {
            char buf[64];
            *is_error = 1;
            snprintf(buf, sizeof(buf), "Command failed with code %ld", rc);
            return strdup(buf);
        }
        return strdup("(no output)");
    }

    /* Truncate to max output size */
    if (len > TOOLS_MAX_OUTPUT - 1)
        len = TOOLS_MAX_OUTPUT - 1;

    result = malloc(len + 1);
    if (!result) {
        fclose(f);
        DeleteFile((CONST_STRPTR)TOOL_CMD_OUTPUT);
        *is_error = 1;
        return strdup("Out of memory");
    }

    fread(result, 1, len, f);
    result[len] = '\0';
    fclose(f);
    DeleteFile((CONST_STRPTR)TOOL_CMD_OUTPUT);

    if (rc != 0) *is_error = 1;

    return result;
}

/* Send an ARexx command to an external port */
static char *tool_exec_arexx(cJSON *input, int *is_error)
{
    cJSON *port_json, *cmd_json;
    const char *port_name, *command;
    struct MsgPort *reply_port = NULL;
    struct MsgPort *target_port;
    struct RexxMsg *rmsg = NULL;
    char *result = NULL;

    port_json = cJSON_GetObjectItemCaseSensitive(input, "port");
    cmd_json  = cJSON_GetObjectItemCaseSensitive(input, "command");

    if (!port_json || !cJSON_IsString(port_json) || !port_json->valuestring[0]) {
        *is_error = 1;
        return strdup("Missing 'port' parameter");
    }
    if (!cmd_json || !cJSON_IsString(cmd_json) || !cmd_json->valuestring[0]) {
        *is_error = 1;
        return strdup("Missing 'command' parameter");
    }

    port_name = port_json->valuestring;
    command   = cmd_json->valuestring;

    printf("  [tool] arexx: port=%s cmd=%s\n", port_name, command);

    /* Create a reply port for the response */
    reply_port = CreateMsgPort();
    if (!reply_port) {
        *is_error = 1;
        return strdup("Cannot create reply port");
    }

    /* Create the RexxMsg */
    rmsg = CreateRexxMsg(reply_port, NULL, NULL);
    if (!rmsg) {
        DeleteMsgPort(reply_port);
        *is_error = 1;
        return strdup("Cannot create RexxMsg");
    }

    /* Set command string */
    rmsg->rm_Args[0] = (STRPTR)CreateArgstring((STRPTR)command, strlen(command));
    if (!rmsg->rm_Args[0]) {
        DeleteRexxMsg(rmsg);
        DeleteMsgPort(reply_port);
        *is_error = 1;
        return strdup("Cannot create argstring");
    }

    rmsg->rm_Action = RXCOMM | RXFF_RESULT;

    /* Find the target port and send */
    Forbid();
    target_port = FindPort((CONST_STRPTR)port_name);
    if (target_port) {
        PutMsg(target_port, (struct Message *)rmsg);
    }
    Permit();

    if (!target_port) {
        DeleteArgstring(rmsg->rm_Args[0]);
        DeleteRexxMsg(rmsg);
        DeleteMsgPort(reply_port);
        *is_error = 1;
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "ARexx port '%s' not found", port_name);
            return strdup(buf);
        }
    }

    /* Wait for reply */
    WaitPort(reply_port);
    {
        struct RexxMsg *reply = (struct RexxMsg *)GetMsg(reply_port);
        if (reply) {
            if (reply->rm_Result1 == 0) {
                /* Success */
                if (reply->rm_Result2) {
                    result = strdup((char *)reply->rm_Result2);
                    DeleteArgstring((STRPTR)reply->rm_Result2);
                } else {
                    result = strdup("OK");
                }
            } else {
                /* Error — rm_Result2 is a numeric secondary error code, NOT a pointer */
                char buf[80];
                *is_error = 1;
                snprintf(buf, sizeof(buf), "ARexx error %ld/%ld",
                         reply->rm_Result1, reply->rm_Result2);
                result = strdup(buf);
            }
        }
    }

    /* Cleanup */
    DeleteArgstring(rmsg->rm_Args[0]);
    DeleteRexxMsg(rmsg);
    DeleteMsgPort(reply_port);

    if (!result)
        result = strdup("No reply received");

    /* Auto-load ARexx documentation on FIRST use of each port.
     * Strip instance suffix so MULTIVIEW.1 or ED_1 finds the base doc file */
    {
        static char docs_sent[16][32];  /* ports we already sent docs for */
        static int  docs_count = 0;
        char base_port[32];
        int already_sent = 0, i;

        strncpy(base_port, port_name, sizeof(base_port) - 1);
        base_port[sizeof(base_port) - 1] = '\0';
        {
            /* Try ".N" suffix (MULTIVIEW.1) then "_N" suffix (ED_1) */
            char *sep = strrchr(base_port, '.');
            if (!sep || sep[1] < '0' || sep[1] > '9')
                sep = strrchr(base_port, '_');
            if (sep && sep[1] >= '0' && sep[1] <= '9')
                *sep = '\0';
        }

        for (i = 0; i < docs_count; i++) {
            if (stricmp(docs_sent[i], base_port) == 0) {
                already_sent = 1;
                break;
            }
        }

        if (!already_sent) {
            char docpath[256];
            FILE *docf;
            snprintf(docpath, sizeof(docpath),
                     "AmigaAI:instructions/ARexx/%s.md", base_port);
            docf = fopen(docpath, "r");
            if (docf) {
                long doclen;
                fseek(docf, 0, SEEK_END);
                doclen = ftell(docf);
                fseek(docf, 0, SEEK_SET);
                if (doclen > 0 && doclen < 8192) {
                    char *docbuf = malloc(doclen + 1);
                    if (docbuf) {
                        fread(docbuf, 1, doclen, docf);
                        docbuf[doclen] = '\0';
                        {
                            char *combined = malloc(doclen + strlen(result) + 80);
                            if (combined) {
                                sprintf(combined,
                                        "--- ARexx reference for %s ---\n%s"
                                        "\n--- Command result ---\n%s",
                                        port_name, docbuf, result);
                                free(result);
                                result = combined;
                            }
                        }
                        free(docbuf);
                    }
                }
                fclose(docf);
            }
            /* Remember this port (even if no doc file found) */
            if (docs_count < 16) {
                strncpy(docs_sent[docs_count], base_port,
                        sizeof(docs_sent[0]) - 1);
                docs_sent[docs_count][sizeof(docs_sent[0]) - 1] = '\0';
                docs_count++;
            }
        }
    }

    return result;
}

/* Read a file and return its contents */
static char *tool_exec_read_file(cJSON *input, int *is_error)
{
    cJSON *path_json;
    const char *path;
    FILE *f;
    long len;
    char *result;

    path_json = cJSON_GetObjectItemCaseSensitive(input, "path");
    if (!path_json || !cJSON_IsString(path_json) || !path_json->valuestring[0]) {
        *is_error = 1;
        return strdup("Missing 'path' parameter");
    }
    path = path_json->valuestring;

    printf("  [tool] read_file: %s\n", path);

    f = fopen(path, "r");
    if (!f) {
        *is_error = 1;
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "Cannot open file: %s", path);
            return strdup(buf);
        }
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len <= 0) {
        fclose(f);
        return strdup("(empty file)");
    }

    if (len > TOOLS_MAX_OUTPUT - 64)
        len = TOOLS_MAX_OUTPUT - 64;

    result = malloc(len + 64);
    if (!result) {
        fclose(f);
        *is_error = 1;
        return strdup("Out of memory");
    }

    {
        long actual = fread(result, 1, len, f);
        result[actual] = '\0';
    }

    fclose(f);
    return result;
}

/* Write content to a file */
static char *tool_exec_write_file(cJSON *input, int *is_error)
{
    cJSON *path_json, *content_json;
    const char *path, *content;
    FILE *f;

    path_json    = cJSON_GetObjectItemCaseSensitive(input, "path");
    content_json = cJSON_GetObjectItemCaseSensitive(input, "content");

    if (!path_json || !cJSON_IsString(path_json) || !path_json->valuestring[0]) {
        *is_error = 1;
        return strdup("Missing 'path' parameter");
    }
    if (!content_json || !cJSON_IsString(content_json)) {
        *is_error = 1;
        return strdup("Missing 'content' parameter");
    }

    path    = path_json->valuestring;
    content = content_json->valuestring;

    printf("  [tool] write_file: %s (%d bytes)\n", path, (int)strlen(content));

    f = fopen(path, "w");
    if (!f) {
        *is_error = 1;
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "Cannot create file: %s", path);
            return strdup(buf);
        }
    }

    fputs(content, f);
    fclose(f);

    {
        char buf[256];
        snprintf(buf, sizeof(buf), "Wrote %d bytes to %s",
                 (int)strlen(content), path);
        return strdup(buf);
    }
}

/* Callback context for identify_file tool */
struct identify_ctx {
    char *buf;
    int   pos;
    int   cap;
};

static void identify_cb(const char *path, const char *name,
                          const char *dt_name, const char *group,
                          void *userdata)
{
    struct identify_ctx *ctx = (struct identify_ctx *)userdata;
    int left = ctx->cap - ctx->pos - 1;
    int n;
    (void)name;

    if (left <= 0) return;

    if (strchr(path, ' '))
        n = snprintf(ctx->buf + ctx->pos, left, "\"%-28s\" %-16s %s\n",
                     path, dt_name, group);
    else
        n = snprintf(ctx->buf + ctx->pos, left, "%-30s %-16s %s\n",
                     path, dt_name, group);
    if (n > 0 && n < left)
        ctx->pos += n;
    else if (n >= left)
        ctx->pos = ctx->cap - 1;  /* truncated */
}

/* Identify file types using the DataType system */
static char *tool_exec_identify(cJSON *input, int *is_error)
{
    cJSON *path_json, *filter_json, *recursive_json, *maxresults_json;
    const char *path;
    const char *filter = NULL;
    int recursive = 0;
    int maxfiles = 0;
    BPTR lock;
    struct FileInfoBlock *fib;
    int is_dir;

    path_json       = cJSON_GetObjectItemCaseSensitive(input, "path");
    filter_json     = cJSON_GetObjectItemCaseSensitive(input, "filter");
    recursive_json  = cJSON_GetObjectItemCaseSensitive(input, "recursive");
    maxresults_json = cJSON_GetObjectItemCaseSensitive(input, "max_results");

    if (!path_json || !cJSON_IsString(path_json) || !path_json->valuestring[0]) {
        *is_error = 1;
        return strdup("Missing 'path' parameter");
    }
    path = path_json->valuestring;

    if (filter_json && cJSON_IsString(filter_json) && filter_json->valuestring[0])
        filter = filter_json->valuestring;

    if (recursive_json && cJSON_IsTrue(recursive_json))
        recursive = 1;

    if (maxresults_json && cJSON_IsNumber(maxresults_json))
        maxfiles = maxresults_json->valueint;

    printf("  [tool] identify_file: %s (filter=%s, recursive=%d, max=%d)\n",
           path, filter ? filter : "(none)", recursive, maxfiles);

    /* Initialize datatypes.library if needed */
    if (dt_init() != 0) {
        *is_error = 1;
        return strdup("Cannot open datatypes.library");
    }

    /* Check if path is a file or directory */
    lock = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (!lock) {
        *is_error = 1;
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "Cannot find: %s", path);
            return strdup(buf);
        }
    }

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if (!fib) {
        UnLock(lock);
        *is_error = 1;
        return strdup("Out of memory");
    }

    is_dir = 0;
    if (Examine(lock, fib))
        is_dir = (fib->fib_DirEntryType > 0);

    FreeDosObject(DOS_FIB, fib);
    UnLock(lock);

    if (is_dir) {
        /* Directory scan */
        struct identify_ctx ctx;
        int count;

        ctx.cap = TOOLS_MAX_OUTPUT;
        ctx.buf = malloc(ctx.cap);
        ctx.pos = 0;
        if (!ctx.buf) {
            *is_error = 1;
            return strdup("Out of memory");
        }
        ctx.buf[0] = '\0';

        count = dt_scan_dir(path, filter, recursive, maxfiles, identify_cb, &ctx);

        if (count == 0) {
            free(ctx.buf);
            return strdup("No matching files found.");
        }
        if (count < 0) {
            free(ctx.buf);
            *is_error = 1;
            return strdup("Error scanning directory");
        }

        /* Warn if output was truncated */
        if (ctx.pos >= ctx.cap - 2) {
            int left = ctx.cap - ctx.pos - 1;
            if (left > 0)
                snprintf(ctx.buf + ctx.pos, left,
                         "\n[truncated, use max_results to limit]");
        }

        return ctx.buf;
    } else {
        /* Single file */
        char dt_name[64], dt_group[32];

        if (dt_identify_file(path, dt_name, sizeof(dt_name),
                             dt_group, sizeof(dt_group)) == 0)
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "%s: %s (%s)", path, dt_name, dt_group);
            return strdup(buf);
        }

        *is_error = 1;
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "Unknown file type: %s", path);
            return strdup(buf);
        }
    }
}

/* ===================== Dispatcher ===================== */

char *tool_execute(const char *name, cJSON *input, int *is_error)
{
    *is_error = 0;

    if (strcmp(name, "shell_command") == 0)
        return tool_exec_shell(input, is_error);

    if (strcmp(name, "arexx_command") == 0)
        return tool_exec_arexx(input, is_error);

    if (strcmp(name, "read_file") == 0)
        return tool_exec_read_file(input, is_error);

    if (strcmp(name, "write_file") == 0)
        return tool_exec_write_file(input, is_error);

    if (strcmp(name, "list_ports") == 0)
        return tool_exec_list_ports(is_error);

    if (strcmp(name, "identify_file") == 0)
        return tool_exec_identify(input, is_error);

    *is_error = 1;
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "Unknown tool: %s", name);
        return strdup(buf);
    }
}
