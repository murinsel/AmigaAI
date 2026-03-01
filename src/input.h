#ifndef AMIGAAI_INPUT_H
#define AMIGAAI_INPUT_H

/* Open input.device and keymap.library.
 * Called lazily on first use. Returns 0 on success, -1 on failure. */
int input_open(void);

/* Close input.device and free resources. */
void input_close(void);

/* Move mouse to absolute screen coordinates (pixels). */
int input_mouse_move(int x, int y);

/* Click or release a mouse button.
 * button: 0=left, 1=right, 2=middle
 * action: 0=click (press+release), 1=press only, 2=release only */
int input_mouse_click(int button, int action);

/* Send a raw key event (press + release).
 * rawkey: Amiga rawkey code (e.g. 0x44=Return, 0x45=Escape)
 * qualifiers: IEQUALIFIER_xxx flags OR'd together */
int input_key(int rawkey, int qualifiers);

/* Type a string by converting each character to rawkey events
 * via MapANSI (respects current system keymap). */
int input_type_text(const char *text);

#endif /* AMIGAAI_INPUT_H */
