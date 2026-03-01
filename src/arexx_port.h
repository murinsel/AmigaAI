#ifndef AMIGAAI_AREXX_H
#define AMIGAAI_AREXX_H

#include "claude.h"

#include <libraries/mui.h>

#define AREXX_PORT_NAME "AMIGAAI"

/* Callback for GUI updates when ARexx receives a response */
typedef void (*ARexxCallback)(const char *response);

/* ARexx context - holds state for MUI ARexx command hooks */
struct ARexxContext {
    struct Claude  *claude;
    ARexxCallback   on_response;
    char           *last_response;
    Object         *win;           /* MUI Window for MOVE/RESIZE */
    Object         *app;           /* MUI Application for local exec */
};

/* Initialize ARexx context and hooks. Call before gui_open(). */
void arexx_setup(struct ARexxContext *ctx, struct Claude *claude,
                 ARexxCallback cb);

/* Free ARexx resources. */
void arexx_cleanup(struct ARexxContext *ctx);

/* Get the MUI_Command array for MUIA_Application_Commands.
 * Must call arexx_setup() first. */
struct MUI_Command *arexx_get_commands(void);

/* Execute a command locally (bypass ARexx message passing).
 * Used when sending commands to our own port to avoid deadlock.
 * Returns result string (caller frees) or NULL on error.
 * Sets *rc to the return code (0=success). */
char *arexx_exec_local(const char *command, int *rc);

#endif /* AMIGAAI_AREXX_H */
