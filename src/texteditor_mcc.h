/*
 * Minimal TextEditor.mcc header for AmigaAI.
 *
 * Only the defines actually used by gui.c are included here.
 * Full header: https://github.com/amiga-mui/texteditor
 */
#ifndef TEXTEDITOR_MCC_H
#define TEXTEDITOR_MCC_H

#define MUIC_TextEditor            "TextEditor.mcc"
#define TextEditor_Dummy           (0xad000000UL)

/* Attributes */
#define MUIA_TextEditor_Contents   (TextEditor_Dummy + 0x02)
#define MUIA_TextEditor_ExportHook (TextEditor_Dummy + 0x08)
#define MUIA_TextEditor_FixedFont  (TextEditor_Dummy + 0x0a)
#define MUIA_TextEditor_ImportHook (TextEditor_Dummy + 0x0e)
#define MUIA_TextEditor_Quiet      (TextEditor_Dummy + 0x17)
#define MUIA_TextEditor_ReadOnly   (TextEditor_Dummy + 0x19)
#define MUIA_TextEditor_Slider     (TextEditor_Dummy + 0x1a)

/* Methods */
#define MUIM_TextEditor_ClearText  (TextEditor_Dummy + 0x24)
#define MUIM_TextEditor_InsertText (TextEditor_Dummy + 0x26)

/* InsertText position constants */
#define MUIV_TextEditor_InsertText_Cursor 0x00000000UL
#define MUIV_TextEditor_InsertText_Top    0x00000001UL
#define MUIV_TextEditor_InsertText_Bottom 0x00000002UL

/* Import/Export hook type constants */
#define MUIV_TextEditor_ImportHook_Plain  0x00000000UL
#define MUIV_TextEditor_ExportHook_Plain  0x00000000UL

#endif /* TEXTEDITOR_MCC_H */
