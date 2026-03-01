#include "arexx_port.h"
#include "input.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <utility/hooks.h>
#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>

/* Global context pointer - accessed by hook functions */
static struct ARexxContext *arx_ctx = NULL;

/*
 * MUI ARexx hook calling convention:
 *   hookfunc(struct Hook *hook, Object *app, LONG *params)
 *   - params is array of parsed template parameters (LONG pointers)
 *   - Return 0 for success, non-zero for error
 *   - Set result via MUIA_Application_RexxString on app
 */

/* ASK TEXT/F - Send question to Claude, return response */
static ULONG ask_func(struct Hook *hook, Object *app, LONG *params)
{
    const char *text = (const char *)params[0];
    char *response, *error_msg = NULL;
    (void)hook;

    if (!text || !*text)
        return 10;

    response = claude_send(arx_ctx->claude, text, &error_msg);
    if (response) {
        free(arx_ctx->last_response);
        arx_ctx->last_response = strdup(response);

        set(app, MUIA_Application_RexxString, (ULONG)response);

        if (arx_ctx->on_response)
            arx_ctx->on_response(response);

        free(response);
        return 0;
    }

    free(error_msg);
    return 10;
}

/* GETLAST - Return last ASK response */
static ULONG getlast_func(struct Hook *hook, Object *app, LONG *params)
{
    (void)hook; (void)params;
    set(app, MUIA_Application_RexxString,
        (ULONG)(arx_ctx->last_response ? arx_ctx->last_response : ""));
    return 0;
}

/* CLEAR - Clear conversation history */
static ULONG clear_func(struct Hook *hook, Object *app, LONG *params)
{
    (void)hook; (void)app; (void)params;
    if (claude_clear_history(arx_ctx->claude) != 0)
        return 20;
    free(arx_ctx->last_response);
    arx_ctx->last_response = NULL;
    return 0;
}

/* SETMODEL MODEL/A - Change AI model */
static ULONG setmodel_func(struct Hook *hook, Object *app, LONG *params)
{
    const char *model = (const char *)params[0];
    (void)hook; (void)app;
    if (!model || !*model)
        return 10;
    strncpy(arx_ctx->claude->config->model, model,
            CONFIG_MAX_MODEL_LEN - 1);
    return 0;
}

/* SETSYSTEM PROMPT/F - Change system prompt */
static ULONG setsystem_func(struct Hook *hook, Object *app, LONG *params)
{
    const char *prompt = (const char *)params[0];
    (void)hook; (void)app;
    if (!prompt || !*prompt)
        return 10;
    strncpy(arx_ctx->claude->config->system_prompt, prompt,
            CONFIG_MAX_PROMPT_LEN - 1);
    return 0;
}

/* MEMADD TEXT/F - Add a memory entry */
static ULONG memadd_func(struct Hook *hook, Object *app, LONG *params)
{
    const char *text = (const char *)params[0];
    (void)hook; (void)app;
    if (!text || !*text)
        return 10;
    if (arx_ctx->claude->memory &&
        memory_add(arx_ctx->claude->memory, text) == 0)
    {
        memory_save(arx_ctx->claude->memory);
        return 0;
    }
    return 10;
}

/* MEMCLEAR - Clear all memory entries */
static ULONG memclear_func(struct Hook *hook, Object *app, LONG *params)
{
    (void)hook; (void)app; (void)params;
    if (arx_ctx->claude->memory) {
        memory_clear(arx_ctx->claude->memory);
        memory_save(arx_ctx->claude->memory);
    }
    return 0;
}

/* MEMCOUNT - Return number of memory entries */
static ULONG memcount_func(struct Hook *hook, Object *app, LONG *params)
{
    char buf[16];
    (void)hook; (void)params;
    snprintf(buf, sizeof(buf), "%d",
             arx_ctx->claude->memory ? arx_ctx->claude->memory->count : 0);
    set(app, MUIA_Application_RexxString, (ULONG)buf);
    return 0;
}

/* MEMORY - Return all memory entries as text */
static ULONG memory_func(struct Hook *hook, Object *app, LONG *params)
{
    (void)hook; (void)params;
    if (arx_ctx->claude->memory) {
        char *memstr = memory_to_string(arx_ctx->claude->memory);
        set(app, MUIA_Application_RexxString,
            (ULONG)(memstr ? memstr : "No memories stored."));
        free(memstr);
    } else {
        set(app, MUIA_Application_RexxString,
            (ULONG)"No memories stored.");
    }
    return 0;
}

/* Helper: get the Intuition Window pointer from the MUI Window object */
static struct Window *get_intuition_win(void)
{
    ULONG val = 0;
    if (!arx_ctx->win) return NULL;
    GetAttr(MUIA_Window_Window, arx_ctx->win, &val);
    return (struct Window *)val;
}

/* MOVE LEFT/A/N,TOP/A/N - Move window to position */
static ULONG move_func(struct Hook *hook, Object *app, LONG *params)
{
    LONG *p_left = (LONG *)params[0];
    LONG *p_top  = (LONG *)params[1];
    struct Window *iwin;
    (void)hook; (void)app;

    if (!p_left || !p_top)
        return 10;

    iwin = get_intuition_win();
    if (!iwin) return 10;

    ChangeWindowBox(iwin, *p_left, *p_top,
                    iwin->Width, iwin->Height);
    return 0;
}

/* RESIZE WIDTH/A/N,HEIGHT/A/N - Resize window */
static ULONG resize_func(struct Hook *hook, Object *app, LONG *params)
{
    LONG *p_width  = (LONG *)params[0];
    LONG *p_height = (LONG *)params[1];
    struct Window *iwin;
    (void)hook; (void)app;

    if (!p_width || !p_height)
        return 10;

    iwin = get_intuition_win();
    if (!iwin) return 10;

    ChangeWindowBox(iwin, iwin->LeftEdge, iwin->TopEdge,
                    *p_width, *p_height);
    return 0;
}

/* WINDOWPOS - Return window position and size as "LEFT TOP WIDTH HEIGHT" */
static ULONG windowpos_func(struct Hook *hook, Object *app, LONG *params)
{
    struct Window *iwin;
    char buf[64];
    (void)hook; (void)params;

    iwin = get_intuition_win();
    if (!iwin) return 10;

    snprintf(buf, sizeof(buf), "%d %d %d %d",
             (int)iwin->LeftEdge, (int)iwin->TopEdge,
             (int)iwin->Width,    (int)iwin->Height);
    set(app, MUIA_Application_RexxString, (ULONG)buf);
    return 0;
}

/* WINDOWTOFRONT - Bring window to front */
static ULONG tofront_func(struct Hook *hook, Object *app, LONG *params)
{
    (void)hook; (void)app; (void)params;
    if (!arx_ctx->win) return 10;
    {
        ULONG msg[] = { MUIM_Window_ToFront };
        DoMethodA(arx_ctx->win, (Msg)msg);
    }
    return 0;
}

/* WINDOWTOBACK - Send window to back */
static ULONG toback_func(struct Hook *hook, Object *app, LONG *params)
{
    (void)hook; (void)app; (void)params;
    if (!arx_ctx->win) return 10;
    {
        ULONG msg[] = { MUIM_Window_ToBack };
        DoMethodA(arx_ctx->win, (Msg)msg);
    }
    return 0;
}

/* MOUSEMOVE X/A/N,Y/A/N - Move mouse to absolute position */
static ULONG mousemove_func(struct Hook *hook, Object *app, LONG *params)
{
    LONG *p_x = (LONG *)params[0];
    LONG *p_y = (LONG *)params[1];
    (void)hook; (void)app;
    if (!p_x || !p_y) return 10;
    return input_mouse_move(*p_x, *p_y) == 0 ? 0 : 10;
}

/* MOUSECLICK BUTTON/A,ACTION/K - Click a mouse button */
static ULONG mouseclick_func(struct Hook *hook, Object *app, LONG *params)
{
    const char *btn_str = (const char *)params[0];
    const char *act_str = (const char *)params[1];
    int button = 0, action = 0;
    (void)hook; (void)app;
    if (!btn_str) return 10;

    if (strcasecmp(btn_str, "LEFT") == 0)        button = 0;
    else if (strcasecmp(btn_str, "RIGHT") == 0)   button = 1;
    else if (strcasecmp(btn_str, "MIDDLE") == 0)  button = 2;
    else return 10;

    if (act_str) {
        if (strcasecmp(act_str, "PRESS") == 0)        action = 1;
        else if (strcasecmp(act_str, "RELEASE") == 0)  action = 2;
    }

    return input_mouse_click(button, action) == 0 ? 0 : 10;
}

/* KEYPRESS CODE/A/N,QUAL/N - Send a raw key event */
static ULONG keypress_func(struct Hook *hook, Object *app, LONG *params)
{
    LONG *p_code = (LONG *)params[0];
    LONG *p_qual = (LONG *)params[1];
    int qual = 0;
    (void)hook; (void)app;
    if (!p_code) return 10;
    if (p_qual) qual = (int)*p_qual;
    return input_key((int)*p_code, qual) == 0 ? 0 : 10;
}

/* TYPETEXT TEXT/F - Type a string via keyboard simulation */
static ULONG typetext_func(struct Hook *hook, Object *app, LONG *params)
{
    const char *text = (const char *)params[0];
    (void)hook; (void)app;
    if (!text || !*text) return 10;
    return input_type_text(text) == 0 ? 0 : 10;
}

/* Hook structs */
static struct Hook ask_hook;
static struct Hook getlast_hook;
static struct Hook clear_hook;
static struct Hook setmodel_hook;
static struct Hook setsystem_hook;
static struct Hook memadd_hook;
static struct Hook memclear_hook;
static struct Hook memcount_hook;
static struct Hook memory_hook;
static struct Hook move_hook;
static struct Hook resize_hook;
static struct Hook windowpos_hook;
static struct Hook tofront_hook;
static struct Hook toback_hook;
static struct Hook mousemove_hook;
static struct Hook mouseclick_hook;
static struct Hook keypress_hook;
static struct Hook typetext_hook;

/* MUI ARexx command table.
 * MUI handles QUIT automatically via MUIV_Application_ReturnID_Quit. */
static struct MUI_Command arexx_commands[] = {
    { (CONST_STRPTR)"ASK",       (CONST_STRPTR)"TEXT/F",   1, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"GETLAST",   NULL,                     0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"CLEAR",     NULL,                     0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"SETMODEL",  (CONST_STRPTR)"MODEL/A",  1, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"SETSYSTEM", (CONST_STRPTR)"PROMPT/F", 1, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"MEMADD",    (CONST_STRPTR)"TEXT/F",   1, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"MEMCLEAR",  NULL,                     0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"MEMCOUNT",  NULL,                     0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"MEMORY",        NULL,                                0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"MOVE",          (CONST_STRPTR)"LEFT/A/N,TOP/A/N",   2, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"RESIZE",        (CONST_STRPTR)"WIDTH/A/N,HEIGHT/A/N", 2, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"WINDOWPOS",     NULL,                                0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"WINDOWTOFRONT", NULL,                                0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"WINDOWTOBACK",  NULL,                                0, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"MOUSEMOVE",     (CONST_STRPTR)"X/A/N,Y/A/N",          2, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"MOUSECLICK",    (CONST_STRPTR)"BUTTON/A,ACTION/K",    2, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"KEYPRESS",      (CONST_STRPTR)"CODE/A/N,QUAL/N",      2, NULL, {0,0,0,0,0} },
    { (CONST_STRPTR)"TYPETEXT",      (CONST_STRPTR)"TEXT/F",                1, NULL, {0,0,0,0,0} },
    { NULL, NULL, 0, NULL, {0,0,0,0,0} }
};

/* Initialize a hook with HookEntry trampoline */
static void init_hook(struct Hook *hook, ULONG (*func)())
{
    memset(hook, 0, sizeof(*hook));
    hook->h_Entry    = (HOOKFUNC)HookEntry;
    hook->h_SubEntry = (HOOKFUNC)func;
}

void arexx_setup(struct ARexxContext *ctx, struct Claude *claude,
                 ARexxCallback cb)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->claude      = claude;
    ctx->on_response = cb;
    arx_ctx = ctx;

    /* Initialize hooks */
    init_hook(&ask_hook,       (ULONG (*)())ask_func);
    init_hook(&getlast_hook,   (ULONG (*)())getlast_func);
    init_hook(&clear_hook,     (ULONG (*)())clear_func);
    init_hook(&setmodel_hook,  (ULONG (*)())setmodel_func);
    init_hook(&setsystem_hook, (ULONG (*)())setsystem_func);
    init_hook(&memadd_hook,    (ULONG (*)())memadd_func);
    init_hook(&memclear_hook,  (ULONG (*)())memclear_func);
    init_hook(&memcount_hook,  (ULONG (*)())memcount_func);
    init_hook(&memory_hook,    (ULONG (*)())memory_func);
    init_hook(&move_hook,      (ULONG (*)())move_func);
    init_hook(&resize_hook,    (ULONG (*)())resize_func);
    init_hook(&windowpos_hook, (ULONG (*)())windowpos_func);
    init_hook(&tofront_hook,  (ULONG (*)())tofront_func);
    init_hook(&toback_hook,   (ULONG (*)())toback_func);

    /* Wire hooks into command table */
    arexx_commands[0].mc_Hook  = &ask_hook;
    arexx_commands[1].mc_Hook  = &getlast_hook;
    arexx_commands[2].mc_Hook  = &clear_hook;
    arexx_commands[3].mc_Hook  = &setmodel_hook;
    arexx_commands[4].mc_Hook  = &setsystem_hook;
    arexx_commands[5].mc_Hook  = &memadd_hook;
    arexx_commands[6].mc_Hook  = &memclear_hook;
    arexx_commands[7].mc_Hook  = &memcount_hook;
    arexx_commands[8].mc_Hook  = &memory_hook;
    arexx_commands[9].mc_Hook  = &move_hook;
    arexx_commands[10].mc_Hook = &resize_hook;
    arexx_commands[11].mc_Hook = &windowpos_hook;
    arexx_commands[12].mc_Hook = &tofront_hook;
    arexx_commands[13].mc_Hook = &toback_hook;

    init_hook(&mousemove_hook,  (ULONG (*)())mousemove_func);
    init_hook(&mouseclick_hook, (ULONG (*)())mouseclick_func);
    init_hook(&keypress_hook,   (ULONG (*)())keypress_func);
    init_hook(&typetext_hook,   (ULONG (*)())typetext_func);
    arexx_commands[14].mc_Hook = &mousemove_hook;
    arexx_commands[15].mc_Hook = &mouseclick_hook;
    arexx_commands[16].mc_Hook = &keypress_hook;
    arexx_commands[17].mc_Hook = &typetext_hook;
}

void arexx_cleanup(struct ARexxContext *ctx)
{
    free(ctx->last_response);
    ctx->last_response = NULL;
    arx_ctx = NULL;
}

struct MUI_Command *arexx_get_commands(void)
{
    return arexx_commands;
}

/* Execute a command locally without ARexx message passing.
 * Parses the command string, dispatches to the matching handler,
 * and returns the result. Avoids deadlock when sending to own port. */
char *arexx_exec_local(const char *command, int *rc)
{
    char cmd_buf[256];
    char *cmd_name, *args;
    struct Window *iwin;
    char result_buf[256];

    *rc = 0;
    result_buf[0] = '\0';

    if (!arx_ctx || !command || !*command) {
        *rc = 10;
        return strdup("ARexx context not initialized");
    }

    /* Copy command to mutable buffer and split name from args */
    strncpy(cmd_buf, command, sizeof(cmd_buf) - 1);
    cmd_buf[sizeof(cmd_buf) - 1] = '\0';

    cmd_name = cmd_buf;
    /* Skip leading whitespace */
    while (*cmd_name == ' ') cmd_name++;

    args = cmd_name;
    while (*args && *args != ' ') args++;
    if (*args) {
        *args = '\0';
        args++;
        while (*args == ' ') args++;
    }

    printf("  [arexx_local] cmd='%s' args='%s'\n", cmd_name, args);

    /* --- Dispatch commands --- */

    if (strcasecmp(cmd_name, "WINDOWPOS") == 0) {
        iwin = get_intuition_win();
        if (!iwin) { *rc = 10; return strdup("Window not available"); }
        snprintf(result_buf, sizeof(result_buf), "%d %d %d %d",
                 (int)iwin->LeftEdge, (int)iwin->TopEdge,
                 (int)iwin->Width, (int)iwin->Height);
        return strdup(result_buf);
    }

    if (strcasecmp(cmd_name, "MOVE") == 0) {
        int left, top;
        if (sscanf(args, "%d %d", &left, &top) != 2) {
            *rc = 10; return strdup("Usage: MOVE <left> <top>");
        }
        iwin = get_intuition_win();
        if (!iwin) { *rc = 10; return strdup("Window not available"); }
        ChangeWindowBox(iwin, left, top, iwin->Width, iwin->Height);
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "RESIZE") == 0) {
        int w, h;
        if (sscanf(args, "%d %d", &w, &h) != 2) {
            *rc = 10; return strdup("Usage: RESIZE <width> <height>");
        }
        iwin = get_intuition_win();
        if (!iwin) { *rc = 10; return strdup("Window not available"); }
        ChangeWindowBox(iwin, iwin->LeftEdge, iwin->TopEdge, w, h);
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "WINDOWTOFRONT") == 0) {
        if (arx_ctx->win) {
            ULONG msg[] = { MUIM_Window_ToFront };
            DoMethodA(arx_ctx->win, (Msg)msg);
        }
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "WINDOWTOBACK") == 0) {
        if (arx_ctx->win) {
            ULONG msg[] = { MUIM_Window_ToBack };
            DoMethodA(arx_ctx->win, (Msg)msg);
        }
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "HIDE") == 0) {
        if (arx_ctx->app) {
            struct TagItem tags[2];
            tags[0].ti_Tag  = MUIA_Application_Iconified;
            tags[0].ti_Data = TRUE;
            tags[1].ti_Tag  = TAG_DONE;
            SetAttrsA(arx_ctx->app, tags);
        }
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "SHOW") == 0) {
        if (arx_ctx->app) {
            struct TagItem tags[2];
            tags[0].ti_Tag  = MUIA_Application_Iconified;
            tags[0].ti_Data = FALSE;
            tags[1].ti_Tag  = TAG_DONE;
            SetAttrsA(arx_ctx->app, tags);
        }
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "CLEAR") == 0) {
        if (claude_clear_history(arx_ctx->claude) != 0) {
            *rc = 20; return strdup("Failed to clear history");
        }
        free(arx_ctx->last_response);
        arx_ctx->last_response = NULL;
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "GETLAST") == 0) {
        return strdup(arx_ctx->last_response ? arx_ctx->last_response : "");
    }

    if (strcasecmp(cmd_name, "MEMCOUNT") == 0) {
        snprintf(result_buf, sizeof(result_buf), "%d",
                 arx_ctx->claude->memory ? arx_ctx->claude->memory->count : 0);
        return strdup(result_buf);
    }

    if (strcasecmp(cmd_name, "MEMORY") == 0) {
        if (arx_ctx->claude->memory) {
            char *memstr = memory_to_string(arx_ctx->claude->memory);
            if (memstr) return memstr;
        }
        return strdup("No memories stored.");
    }

    if (strcasecmp(cmd_name, "MEMADD") == 0) {
        if (!args || !*args) { *rc = 10; return strdup("Usage: MEMADD <text>"); }
        if (arx_ctx->claude->memory &&
            memory_add(arx_ctx->claude->memory, args) == 0) {
            memory_save(arx_ctx->claude->memory);
            return strdup("OK");
        }
        *rc = 10; return strdup("Failed to add memory");
    }

    if (strcasecmp(cmd_name, "MEMCLEAR") == 0) {
        if (arx_ctx->claude->memory) {
            memory_clear(arx_ctx->claude->memory);
            memory_save(arx_ctx->claude->memory);
        }
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "SETMODEL") == 0) {
        if (!args || !*args) { *rc = 10; return strdup("Usage: SETMODEL <model>"); }
        strncpy(arx_ctx->claude->config->model, args, CONFIG_MAX_MODEL_LEN - 1);
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "SETSYSTEM") == 0) {
        if (!args || !*args) { *rc = 10; return strdup("Usage: SETSYSTEM <prompt>"); }
        strncpy(arx_ctx->claude->config->system_prompt, args, CONFIG_MAX_PROMPT_LEN - 1);
        return strdup("OK");
    }

    if (strcasecmp(cmd_name, "MOUSEMOVE") == 0) {
        int x, y;
        if (sscanf(args, "%d %d", &x, &y) != 2) {
            *rc = 10; return strdup("Usage: MOUSEMOVE <x> <y>");
        }
        *rc = input_mouse_move(x, y) == 0 ? 0 : 10;
        return strdup(*rc == 0 ? "OK" : "Failed to move mouse");
    }

    if (strcasecmp(cmd_name, "MOUSECLICK") == 0) {
        char btn[16] = "", act[16] = "";
        int button = 0, action = 0;
        sscanf(args, "%15s %15s", btn, act);
        if (strcasecmp(btn, "LEFT") == 0)        button = 0;
        else if (strcasecmp(btn, "RIGHT") == 0)  button = 1;
        else if (strcasecmp(btn, "MIDDLE") == 0) button = 2;
        else { *rc = 10; return strdup("Usage: MOUSECLICK LEFT|RIGHT|MIDDLE [PRESS|RELEASE]"); }
        if (act[0]) {
            if (strcasecmp(act, "PRESS") == 0)        action = 1;
            else if (strcasecmp(act, "RELEASE") == 0)  action = 2;
        }
        *rc = input_mouse_click(button, action) == 0 ? 0 : 10;
        return strdup(*rc == 0 ? "OK" : "Failed to click");
    }

    if (strcasecmp(cmd_name, "KEYPRESS") == 0) {
        int code, qual = 0;
        if (sscanf(args, "%d %d", &code, &qual) < 1) {
            *rc = 10; return strdup("Usage: KEYPRESS <code> [<qualifiers>]");
        }
        *rc = input_key(code, qual) == 0 ? 0 : 10;
        return strdup(*rc == 0 ? "OK" : "Failed to send key");
    }

    if (strcasecmp(cmd_name, "TYPETEXT") == 0) {
        if (!args || !*args) { *rc = 10; return strdup("Usage: TYPETEXT <text>"); }
        *rc = input_type_text(args) == 0 ? 0 : 10;
        return strdup(*rc == 0 ? "OK" : "Failed to type text");
    }

    *rc = 5;
    snprintf(result_buf, sizeof(result_buf), "Unknown command: %s", cmd_name);
    return strdup(result_buf);
}
