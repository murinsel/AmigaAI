#include "arexx_port.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>

int arexx_init(struct ARexxPort *arx, struct Claude *claude, ARexxCallback cb)
{
    memset(arx, 0, sizeof(*arx));
    arx->claude = claude;
    arx->on_response = cb;

    /* Create public message port */
    arx->port = CreateMsgPort();
    if (!arx->port) {
        printf("ERROR: Cannot create ARexx message port\n");
        return -1;
    }

    arx->port->mp_Node.ln_Name = (char *)AREXX_PORT_NAME;
    arx->port->mp_Node.ln_Pri  = 0;
    AddPort(arx->port);

    return 0;
}

void arexx_cleanup(struct ARexxPort *arx)
{
    if (arx->port) {
        struct Message *msg;

        RemPort(arx->port);

        /* Flush any pending messages */
        while ((msg = GetMsg(arx->port)) != NULL)
            ReplyMsg(msg);

        DeleteMsgPort(arx->port);
        arx->port = NULL;
    }

    free(arx->last_response);
    arx->last_response = NULL;
}

unsigned long arexx_signal(struct ARexxPort *arx)
{
    if (!arx->port) return 0;
    return 1UL << arx->port->mp_SigBit;
}

/* Reply to a RexxMsg with a result string and return code */
static void reply_rexx(struct RexxMsg *rmsg, long rc, const char *result)
{
    rmsg->rm_Result1 = rc;
    rmsg->rm_Result2 = 0;

    if (rc == 0 && result && (rmsg->rm_Action & RXFF_RESULT)) {
        rmsg->rm_Result2 = (LONG)CreateArgstring((STRPTR)result, strlen(result));
    }

    ReplyMsg((struct Message *)rmsg);
}

int arexx_handle(struct ARexxPort *arx)
{
    struct RexxMsg *rmsg;
    int quit = 0;

    while ((rmsg = (struct RexxMsg *)GetMsg(arx->port)) != NULL) {
        const char *cmd;
        char *response;
        char *error_msg = NULL;

        /* Only handle RXCOMM messages */
        if ((rmsg->rm_Action & RXCODEMASK) != RXCOMM) {
            ReplyMsg((struct Message *)rmsg);
            continue;
        }

        cmd = (const char *)ARG0(rmsg);
        if (!cmd) {
            reply_rexx(rmsg, 5, "No command");
            continue;
        }

        if (strncasecmp(cmd, "QUIT", 4) == 0) {
            reply_rexx(rmsg, 0, "OK");
            quit = 1;

        } else if (strncasecmp(cmd, "ASK ", 4) == 0) {
            const char *text = cmd + 4;

            /* Skip leading whitespace */
            while (*text == ' ') text++;

            response = claude_send(arx->claude, text, &error_msg);
            if (response) {
                /* Store last response */
                free(arx->last_response);
                arx->last_response = strdup(response);

                reply_rexx(rmsg, 0, response);

                if (arx->on_response)
                    arx->on_response(response);

                free(response);
            } else {
                reply_rexx(rmsg, 10, error_msg ? error_msg : "Error");
                free(error_msg);
            }

        } else if (strncasecmp(cmd, "GETLAST", 7) == 0) {
            reply_rexx(rmsg, 0,
                       arx->last_response ? arx->last_response : "");

        } else if (strncasecmp(cmd, "CLEAR", 5) == 0) {
            if (claude_clear_history(arx->claude) != 0) {
                reply_rexx(rmsg, 20, "Out of memory");
                continue;
            }
            free(arx->last_response);
            arx->last_response = NULL;
            reply_rexx(rmsg, 0, "OK");

        } else if (strncasecmp(cmd, "SETMODEL ", 9) == 0) {
            const char *model = cmd + 9;
            while (*model == ' ') model++;
            strncpy(arx->claude->config->model, model,
                    CONFIG_MAX_MODEL_LEN - 1);
            reply_rexx(rmsg, 0, "OK");

        } else if (strncasecmp(cmd, "SETSYSTEM ", 10) == 0) {
            const char *prompt = cmd + 10;
            while (*prompt == ' ') prompt++;
            strncpy(arx->claude->config->system_prompt, prompt,
                    CONFIG_MAX_PROMPT_LEN - 1);
            reply_rexx(rmsg, 0, "OK");

        } else if (strncasecmp(cmd, "MEMADD ", 7) == 0) {
            const char *text = cmd + 7;
            while (*text == ' ') text++;
            if (arx->claude->memory &&
                memory_add(arx->claude->memory, text) == 0)
            {
                memory_save(arx->claude->memory);
                reply_rexx(rmsg, 0, "OK");
            } else {
                reply_rexx(rmsg, 10, "Memory full");
            }

        } else if (strncasecmp(cmd, "MEMCLEAR", 8) == 0) {
            if (arx->claude->memory) {
                memory_clear(arx->claude->memory);
                memory_save(arx->claude->memory);
            }
            reply_rexx(rmsg, 0, "OK");

        } else if (strncasecmp(cmd, "MEMCOUNT", 8) == 0) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d",
                     arx->claude->memory ? arx->claude->memory->count : 0);
            reply_rexx(rmsg, 0, buf);

        } else if (strncasecmp(cmd, "MEMORY", 6) == 0 &&
                   (cmd[6] == '\0' || cmd[6] == ' ')) {
            if (arx->claude->memory) {
                char *memstr = memory_to_string(arx->claude->memory);
                reply_rexx(rmsg, 0,
                           memstr ? memstr : "No memories stored.");
                free(memstr);
            } else {
                reply_rexx(rmsg, 0, "No memories stored.");
            }

        } else {
            reply_rexx(rmsg, 5, "Unknown command");
        }
    }

    return quit;
}
