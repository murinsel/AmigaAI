/*
 * input.c - Input device simulation for AmigaAI
 *
 * Provides mouse positioning, mouse clicks, and keyboard input
 * via AmigaOS input.device. Used for "computer use" capabilities.
 */

#include "input.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/timer.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/keymap.h>

#include <string.h>

struct Library *KeymapBase = NULL;

static struct MsgPort  *input_port = NULL;
static struct IOStdReq *input_io   = NULL;
static int              input_open_flag = 0;

int input_open(void)
{
    if (input_open_flag)
        return 0;

    input_port = CreateMsgPort();
    if (!input_port)
        return -1;

    input_io = (struct IOStdReq *)CreateIORequest(input_port,
                                                   sizeof(struct IOStdReq));
    if (!input_io) {
        DeleteMsgPort(input_port);
        input_port = NULL;
        return -1;
    }

    if (OpenDevice("input.device", 0,
                   (struct IORequest *)input_io, 0) != 0)
    {
        DeleteIORequest((struct IORequest *)input_io);
        DeleteMsgPort(input_port);
        input_io   = NULL;
        input_port = NULL;
        return -1;
    }

    KeymapBase = OpenLibrary("keymap.library", 36);
    if (!KeymapBase) {
        CloseDevice((struct IORequest *)input_io);
        DeleteIORequest((struct IORequest *)input_io);
        DeleteMsgPort(input_port);
        input_io   = NULL;
        input_port = NULL;
        return -1;
    }

    input_open_flag = 1;
    return 0;
}

void input_close(void)
{
    if (!input_open_flag)
        return;

    CloseDevice((struct IORequest *)input_io);
    DeleteIORequest((struct IORequest *)input_io);
    DeleteMsgPort(input_port);

    if (KeymapBase) {
        CloseLibrary(KeymapBase);
        KeymapBase = NULL;
    }

    input_io   = NULL;
    input_port = NULL;
    input_open_flag = 0;
}

/* Send an input event via IND_WRITEEVENT (synchronous). */
static int send_event(struct InputEvent *ie)
{
    if (!input_open_flag) {
        if (input_open() != 0)
            return -1;
    }

    input_io->io_Command = IND_WRITEEVENT;
    input_io->io_Data    = (APTR)ie;
    input_io->io_Length  = sizeof(struct InputEvent);
    input_io->io_Flags   = 0;

    DoIO((struct IORequest *)input_io);

    return (input_io->io_Error == 0) ? 0 : -1;
}

int input_mouse_move(int x, int y)
{
    struct InputEvent ie;
    struct IEPointerPixel pix;
    struct Screen *scr;
    int rc;

    scr = LockPubScreen(NULL);
    if (!scr)
        return -1;

    memset(&pix, 0, sizeof(pix));
    pix.iepp_Screen     = scr;
    pix.iepp_Position.X = (WORD)x;
    pix.iepp_Position.Y = (WORD)y;

    memset(&ie, 0, sizeof(ie));
    ie.ie_Class        = IECLASS_NEWPOINTERPOS;
    ie.ie_SubClass     = IESUBCLASS_PIXEL;
    ie.ie_Code         = 0;
    ie.ie_Qualifier    = 0;
    ie.ie_EventAddress = &pix;
    CurrentTime((ULONG *)&ie.ie_TimeStamp.tv_secs,
                (ULONG *)&ie.ie_TimeStamp.tv_micro);

    rc = send_event(&ie);

    UnlockPubScreen(NULL, scr);
    return rc;
}

int input_mouse_click(int button, int action)
{
    struct InputEvent ie;
    UWORD code;

    switch (button) {
        case 0:  code = IECODE_LBUTTON; break;
        case 1:  code = IECODE_RBUTTON; break;
        case 2:  code = IECODE_MBUTTON; break;
        default: return -1;
    }

    memset(&ie, 0, sizeof(ie));
    ie.ie_Class = IECLASS_RAWMOUSE;

    /* Press */
    if (action == 0 || action == 1) {
        ie.ie_Code      = code;
        ie.ie_Qualifier = 0;
        CurrentTime((ULONG *)&ie.ie_TimeStamp.tv_secs,
                    (ULONG *)&ie.ie_TimeStamp.tv_micro);
        if (send_event(&ie) != 0)
            return -1;
    }

    /* Release */
    if (action == 0 || action == 2) {
        ie.ie_Code      = code | IECODE_UP_PREFIX;
        ie.ie_Qualifier = 0;
        CurrentTime((ULONG *)&ie.ie_TimeStamp.tv_secs,
                    (ULONG *)&ie.ie_TimeStamp.tv_micro);
        if (send_event(&ie) != 0)
            return -1;
    }

    return 0;
}

int input_key(int rawkey, int qualifiers)
{
    struct InputEvent ie;

    memset(&ie, 0, sizeof(ie));
    ie.ie_Class     = IECLASS_RAWKEY;
    ie.ie_Code      = (UWORD)rawkey;
    ie.ie_Qualifier = (UWORD)qualifiers;
    CurrentTime((ULONG *)&ie.ie_TimeStamp.tv_secs,
                (ULONG *)&ie.ie_TimeStamp.tv_micro);

    /* Key press */
    if (send_event(&ie) != 0)
        return -1;

    /* Key release */
    ie.ie_Code = (UWORD)(rawkey | IECODE_UP_PREFIX);
    CurrentTime((ULONG *)&ie.ie_TimeStamp.tv_secs,
                (ULONG *)&ie.ie_TimeStamp.tv_micro);

    return send_event(&ie);
}

int input_type_text(const char *text)
{
    struct InputEvent ie;
    int i;

    if (!text)
        return -1;

    if (!input_open_flag) {
        if (input_open() != 0)
            return -1;
    }

    for (i = 0; text[i]; i++) {
        unsigned char ch = (unsigned char)text[i];
        unsigned char rbuf[6];  /* up to 3 code/qualifier pairs (dead keys) */
        LONG actual;
        UWORD code, qual;

        /* Handle special control characters explicitly */
        if (ch == '\n' || ch == '\r') {
            if (input_key(0x44, 0) != 0) return -1;
            continue;
        }
        if (ch == '\t') {
            if (input_key(0x42, 0) != 0) return -1;
            continue;
        }
        if (ch == '\b') {
            if (input_key(0x41, 0) != 0) return -1;
            continue;
        }
        if (ch == 0x1b) {
            if (input_key(0x45, 0) != 0) return -1;
            continue;
        }

        /* Use MapANSI to convert character to rawkey code/qualifier */
        actual = MapANSI((STRPTR)&ch, 1, (STRPTR)rbuf, 3, NULL);
        if (actual <= 0)
            continue;  /* unmappable character, skip */

        /* Send the key events (may include dead key prefixes) */
        {
            int j;
            for (j = 0; j < actual; j++) {
                code = rbuf[j * 2];
                qual = rbuf[j * 2 + 1];

                memset(&ie, 0, sizeof(ie));
                ie.ie_Class     = IECLASS_RAWKEY;
                ie.ie_Code      = code;
                ie.ie_Qualifier = qual;
                CurrentTime((ULONG *)&ie.ie_TimeStamp.tv_secs,
                            (ULONG *)&ie.ie_TimeStamp.tv_micro);

                if (send_event(&ie) != 0)
                    return -1;

                /* Release */
                ie.ie_Code = code | IECODE_UP_PREFIX;
                CurrentTime((ULONG *)&ie.ie_TimeStamp.tv_secs,
                            (ULONG *)&ie.ie_TimeStamp.tv_micro);

                if (send_event(&ie) != 0)
                    return -1;
            }
        }
    }

    return 0;
}
