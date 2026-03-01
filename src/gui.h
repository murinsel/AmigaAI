#ifndef AMIGAAI_GUI_H
#define AMIGAAI_GUI_H

#include <libraries/mui.h>

#define GUI_HISTORY_SIZE   10
#define GUI_HISTORY_LEN   512

/* MUI Return IDs */
#define GUI_ID_SEND      1
#define GUI_ID_NEW       2
#define GUI_ID_ABOUT     3
#define GUI_ID_APIKEY    4
#define GUI_ID_MODEL     5
#define GUI_ID_SYSTEM    6
#define GUI_ID_MEMVIEW   7
#define GUI_ID_MEMADD    8
#define GUI_ID_MEMCLEAR  9
#define GUI_ID_CHATSAVE 10
#define GUI_ID_CHATLOAD 11
#define GUI_ID_QUIT     12
#define GUI_ID_STOP     13

struct Gui {
    Object *app;
    Object *win;
    Object *editor;      /* TextEditor.mcc for chat display */
    Object *scrollbar;   /* Scrollbar for editor */
    Object *input;       /* String gadget for input */
    Object *send_btn;    /* Send button */
    Object *stop_btn;    /* Stop/abort button */
    Object *status;      /* Status text */
    Object *menustrip;

    int     busy;
    int     abort_requested;  /* Set by Stop button */

    /* Input history (ring buffer) */
    char    history[GUI_HISTORY_SIZE][GUI_HISTORY_LEN];
    int     hist_count;    /* total entries stored */
    int     hist_pos;      /* current browse position (-1 = not browsing) */
};

/* Open MUI application and window. Returns 0 on success. */
int gui_open(struct Gui *gui);

/* Dispose MUI application and free resources. */
void gui_close(struct Gui *gui);

/* Process MUI input. Call in main loop.
 * *signals is in/out: on input the signals from Wait(), on output
 * the signals to pass to the next Wait().
 * Returns a GUI_ID_* constant, or MUIV_Application_ReturnID_Quit, or 0. */
ULONG gui_process(struct Gui *gui, ULONG *signals);

/* Get the current input text. */
const char *gui_get_input(struct Gui *gui);

/* Clear the input string gadget and reactivate it. */
void gui_clear_input(struct Gui *gui);

/* Add a single line to the chat list. */
void gui_add_line(struct Gui *gui, const char *text);

/* Add a multiline text block, splitting at newlines.
 * prefix is prepended to the first line (may be NULL). */
void gui_add_text(struct Gui *gui, const char *prefix, const char *text);

/* Set the status bar text. */
void gui_set_status(struct Gui *gui, const char *text);

/* Set/clear busy state (sleep/wake the application). */
void gui_set_busy(struct Gui *gui, int busy);

/* Clear the entire chat display. */
void gui_clear_chat(struct Gui *gui);

/* Show an About requester. */
void gui_about(struct Gui *gui, const char *title, const char *body);

/* Save current input to history (call before clearing). */
void gui_history_push(struct Gui *gui, const char *text);

/* Activate the input field (set keyboard focus). */
void gui_focus_input(struct Gui *gui);

/* Process MUI events and check if Stop was clicked.
 * Returns non-zero if abort requested. */
int gui_check_abort(struct Gui *gui);

#endif /* AMIGAAI_GUI_H */
