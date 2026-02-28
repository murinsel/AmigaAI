#ifndef AMIGAAI_AREXX_H
#define AMIGAAI_AREXX_H

#include "claude.h"

#define AREXX_PORT_NAME "AMIGAAI"

/* ARexx command handler callback.
 * Returns 1 if QUIT was received. */
typedef void (*ARexxCallback)(const char *response);

struct ARexxPort {
    struct MsgPort *port;
    struct Claude  *claude;
    ARexxCallback   on_response;  /* Called when a response is received */
    char           *last_response;
};

/* Create ARexx port. Returns 0 on success. */
int arexx_init(struct ARexxPort *arx, struct Claude *claude, ARexxCallback cb);

/* Free ARexx port. */
void arexx_cleanup(struct ARexxPort *arx);

/* Get the signal mask for Wait(). */
unsigned long arexx_signal(struct ARexxPort *arx);

/* Handle pending ARexx messages. Returns 1 if QUIT was received. */
int arexx_handle(struct ARexxPort *arx);

#endif /* AMIGAAI_AREXX_H */
