#include "gui.h"
#include "texteditor_mcc.h"
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>
#include <libraries/mui.h>
#include <intuition/intuition.h>

/* MUI menu separator - NM_BARLABEL from gadtools.h */
#ifndef NM_BARLABEL
#define NM_BARLABEL ((STRPTR)~0)
#endif

/* MAKE_ID for window IDs */
#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG)(a)<<24|(ULONG)(b)<<16|(ULONG)(c)<<8|(ULONG)(d))
#endif

/* MUI library base (opened/closed in main.c) */
extern struct Library *MUIMasterBase;

/*
 * GCC 13+ generates broken code for variadic Amiga library calls
 * (the &tags trick in inline stubs doesn't work). Use non-variadic
 * SetAttrsA/GetAttr and manual DoMethodA everywhere.
 */
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

/* Rawkey codes for cursor up/down */
#define RAWKEY_UP   0x4C
#define RAWKEY_DOWN 0x4D

/* Key-up bit in IntuiMessage Code */
#ifndef IECODE_UP_PREFIX
#define IECODE_UP_PREFIX 0x80
#endif

/* MUI HandleEvent return value */
#ifndef MUI_EventHandlerRC_Eat
#define MUI_EventHandlerRC_Eat (1<<0)
#endif

/* Global pointer so the custom class can access the Gui struct */
static struct Gui *edit_hook_gui = NULL;

/* --- Custom MUI String subclass for cursor up/down history --- */
/* MUI's built-in String class intercepts cursor keys before the EditHook,
 * so we need a custom subclass that registers an IDCMP_RAWKEY event
 * handler to catch cursor up/down directly. */

struct HistStringData {
    struct MUI_EventHandlerNode ehnode;
};

static struct MUI_CustomClass *hist_string_mcc = NULL;

static ULONG HistString_Setup(struct IClass *cl, Object *obj, Msg msg)
{
    struct HistStringData *data = INST_DATA(cl, obj);

    if (!DoSuperMethodA(cl, obj, msg))
        return FALSE;

    /* Register for raw key events with high priority so we get them
     * before MUI uses cursor keys for cycle chain navigation. */
    memset(&data->ehnode, 0, sizeof(data->ehnode));
    data->ehnode.ehn_Priority = 1;
    data->ehnode.ehn_Flags    = 0;
    data->ehnode.ehn_Object   = obj;
    data->ehnode.ehn_Class    = cl;
    data->ehnode.ehn_Events   = IDCMP_RAWKEY;

    {
        ULONG amsg[] = { MUIM_Window_AddEventHandler,
                         (ULONG)&data->ehnode };
        DoMethodA(_win(obj), (Msg)amsg);
    }

    return TRUE;
}

static ULONG HistString_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
    struct HistStringData *data = INST_DATA(cl, obj);

    {
        ULONG rmsg[] = { MUIM_Window_RemEventHandler,
                         (ULONG)&data->ehnode };
        DoMethodA(_win(obj), (Msg)rmsg);
    }

    return DoSuperMethodA(cl, obj, msg);
}

static ULONG HistString_HandleEvent(struct IClass *cl, Object *obj,
                                     struct MUIP_HandleEvent *msg)
{
    struct Gui *gui = edit_hook_gui;
    UWORD code;

    (void)cl;

    if (!msg->imsg || msg->imsg->Class != IDCMP_RAWKEY || !gui)
        return 0;

    code = msg->imsg->Code;

    /* Ignore key releases */
    if (code & IECODE_UP_PREFIX)
        return 0;

    /* Only handle when this string object has focus */
    {
        ULONG active = xget(gui->win, MUIA_Window_ActiveObject);
        if ((Object *)active != obj)
            return 0;
    }

    if (code == RAWKEY_UP && gui->hist_count > 0) {
        int oldest, idx;

        oldest = gui->hist_count > GUI_HISTORY_SIZE
                 ? gui->hist_count - GUI_HISTORY_SIZE : 0;

        if (gui->hist_pos < 0)
            gui->hist_pos = gui->hist_count - 1;
        else if (gui->hist_pos > oldest)
            gui->hist_pos--;
        else
            return MUI_EventHandlerRC_Eat;

        idx = gui->hist_pos % GUI_HISTORY_SIZE;
        xset(obj, MUIA_String_Contents, (ULONG)gui->history[idx]);
        return MUI_EventHandlerRC_Eat;
    }

    if (code == RAWKEY_DOWN && gui->hist_pos >= 0) {
        if (gui->hist_pos < gui->hist_count - 1) {
            int idx;
            gui->hist_pos++;
            idx = gui->hist_pos % GUI_HISTORY_SIZE;
            xset(obj, MUIA_String_Contents, (ULONG)gui->history[idx]);
        } else {
            gui->hist_pos = -1;
            xset(obj, MUIA_String_Contents, (ULONG)"");
        }
        return MUI_EventHandlerRC_Eat;
    }

    return 0;
}

static ULONG HistString_Dispatch(
    struct IClass *cl __asm("a0"),
    Object *obj __asm("a2"),
    Msg msg __asm("a1"))
{
    switch (msg->MethodID) {
        case MUIM_Setup:       return HistString_Setup(cl, obj, msg);
        case MUIM_Cleanup:     return HistString_Cleanup(cl, obj, msg);
        case MUIM_HandleEvent: return HistString_HandleEvent(cl, obj,
                                   (struct MUIP_HandleEvent *)msg);
    }
    return DoSuperMethodA(cl, obj, msg);
}

/* Helper: create a menu item with title, optional shortcut, and optional userdata.
 * All use MUI_NewObjectA to avoid broken variadic stubs. */
static Object *mk_menuitem(const char *title, const char *shortcut, ULONG udata)
{
    if ((ULONG)title == (ULONG)NM_BARLABEL) {
        struct TagItem tags[] = {
            { MUIA_Menuitem_Title, (ULONG)NM_BARLABEL },
            { TAG_DONE, 0 }
        };
        return MUI_NewObjectA((CONST_STRPTR)MUIC_Menuitem, tags);
    } else if (shortcut) {
        struct TagItem tags[] = {
            { MUIA_Menuitem_Title,    (ULONG)title },
            { MUIA_Menuitem_Shortcut, (ULONG)shortcut },
            { MUIA_UserData,          udata },
            { TAG_DONE, 0 }
        };
        return MUI_NewObjectA((CONST_STRPTR)MUIC_Menuitem, tags);
    } else {
        struct TagItem tags[] = {
            { MUIA_Menuitem_Title, (ULONG)title },
            { MUIA_UserData,       udata },
            { TAG_DONE, 0 }
        };
        return MUI_NewObjectA((CONST_STRPTR)MUIC_Menuitem, tags);
    }
}

static Object *make_menu(void)
{
    Object *strip, *proj, *settings, *memory;

    /* --- Project menu items --- */
    Object *mi_new      = mk_menuitem("New Chat",      "N", GUI_ID_NEW);
    Object *mi_bar1     = mk_menuitem((const char *)NM_BARLABEL, NULL, 0);
    Object *mi_save     = mk_menuitem("Save Chat...",  "S", GUI_ID_CHATSAVE);
    Object *mi_load     = mk_menuitem("Load Chat...",  "L", GUI_ID_CHATLOAD);
    Object *mi_bar2     = mk_menuitem((const char *)NM_BARLABEL, NULL, 0);
    Object *mi_about    = mk_menuitem("About...",      "?", GUI_ID_ABOUT);
    Object *mi_bar3     = mk_menuitem((const char *)NM_BARLABEL, NULL, 0);
    Object *mi_quit     = mk_menuitem("Quit",          "Q", GUI_ID_QUIT);

    /* --- Settings menu items --- */
    Object *mi_apikey   = mk_menuitem("API Key...",       NULL, GUI_ID_APIKEY);
    Object *mi_model    = mk_menuitem("Model...",         NULL, GUI_ID_MODEL);
    Object *mi_system   = mk_menuitem("System Prompt...", NULL, GUI_ID_SYSTEM);

    /* --- Memory menu items --- */
    Object *mi_memview  = mk_menuitem("View Memory...",   "M", GUI_ID_MEMVIEW);
    Object *mi_memadd   = mk_menuitem("Add Memory...",    NULL, GUI_ID_MEMADD);
    Object *mi_bar4     = mk_menuitem((const char *)NM_BARLABEL, NULL, 0);
    Object *mi_memclear = mk_menuitem("Clear Memory",     NULL, GUI_ID_MEMCLEAR);

    /* --- Project menu --- */
    {
        struct TagItem tags[] = {
            { MUIA_Menu_Title,    (ULONG)"Project" },
            { MUIA_Family_Child,  (ULONG)mi_new },
            { MUIA_Family_Child,  (ULONG)mi_bar1 },
            { MUIA_Family_Child,  (ULONG)mi_save },
            { MUIA_Family_Child,  (ULONG)mi_load },
            { MUIA_Family_Child,  (ULONG)mi_bar2 },
            { MUIA_Family_Child,  (ULONG)mi_about },
            { MUIA_Family_Child,  (ULONG)mi_bar3 },
            { MUIA_Family_Child,  (ULONG)mi_quit },
            { TAG_DONE, 0 }
        };
        proj = MUI_NewObjectA((CONST_STRPTR)MUIC_Menu, tags);
    }

    /* --- Settings menu --- */
    {
        struct TagItem tags[] = {
            { MUIA_Menu_Title,    (ULONG)"Settings" },
            { MUIA_Family_Child,  (ULONG)mi_apikey },
            { MUIA_Family_Child,  (ULONG)mi_model },
            { MUIA_Family_Child,  (ULONG)mi_system },
            { TAG_DONE, 0 }
        };
        settings = MUI_NewObjectA((CONST_STRPTR)MUIC_Menu, tags);
    }

    /* --- Memory menu --- */
    {
        struct TagItem tags[] = {
            { MUIA_Menu_Title,    (ULONG)"Memory" },
            { MUIA_Family_Child,  (ULONG)mi_memview },
            { MUIA_Family_Child,  (ULONG)mi_memadd },
            { MUIA_Family_Child,  (ULONG)mi_bar4 },
            { MUIA_Family_Child,  (ULONG)mi_memclear },
            { TAG_DONE, 0 }
        };
        memory = MUI_NewObjectA((CONST_STRPTR)MUIC_Menu, tags);
    }

    /* --- Menustrip --- */
    {
        struct TagItem tags[] = {
            { MUIA_Family_Child, (ULONG)proj },
            { MUIA_Family_Child, (ULONG)settings },
            { MUIA_Family_Child, (ULONG)memory },
            { TAG_DONE, 0 }
        };
        strip = MUI_NewObjectA((CONST_STRPTR)MUIC_Menustrip, tags);
    }

    return strip;
}

int gui_open(struct Gui *gui)
{
    memset(gui, 0, sizeof(*gui));

    printf("  gui: make_menu...\n");
    gui->menustrip = make_menu();
    printf("  gui: menu=%p\n", (void *)gui->menustrip);
    if (!gui->menustrip) {
        printf("ERROR: make_menu() returned NULL\n");
        return -1;
    }

    /* Create window contents step by step for debugging. */

    /* Create scrollbar for TextEditor */
    printf("  gui: creating scrollbar...\n");
    {
        struct TagItem sb_tags[] = { { TAG_DONE, 0 } };
        gui->scrollbar = MUI_NewObjectA((CONST_STRPTR)MUIC_Scrollbar, sb_tags);
    }
    printf("  gui: scrollbar=%p\n", (void *)gui->scrollbar);

    /* Create TextEditor.mcc for chat display */
    printf("  gui: creating TextEditor.mcc...\n");
    {
        struct TagItem te_tags[] = {
            { MUIA_TextEditor_ReadOnly,   TRUE },
            { MUIA_TextEditor_FixedFont,  TRUE },
            { MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_Plain },
            { MUIA_TextEditor_ExportHook, MUIV_TextEditor_ExportHook_Plain },
            { MUIA_TextEditor_Slider,     (ULONG)gui->scrollbar },
            { TAG_DONE, 0 }
        };
        gui->editor = MUI_NewObjectA((CONST_STRPTR)MUIC_TextEditor, te_tags);
    }
    printf("  gui: editor=%p\n", (void *)gui->editor);
    if (!gui->editor) {
        printf("WARNING: TextEditor.mcc not available!\n");
        printf("Please install TextEditor.mcc from Aminet.\n");
    }

    /* Set up global pointer for custom class access */
    edit_hook_gui = gui;
    gui->hist_count = 0;
    gui->hist_pos   = -1;

    /* Create custom String subclass for history navigation */
    printf("  gui: creating custom string class...\n");
    hist_string_mcc = MUI_CreateCustomClass(NULL,
        (CONST_STRPTR)MUIC_String, NULL,
        sizeof(struct HistStringData),
        (APTR)HistString_Dispatch);
    printf("  gui: hist_string_mcc=%p\n", (void *)hist_string_mcc);

    printf("  gui: creating input...\n");
    if (hist_string_mcc) {
        struct TagItem tags[] = {
            { MUIA_Frame, MUIV_Frame_String },
            { MUIA_String_MaxLen, (ULONG)1024 },
            { MUIA_CycleChain, (ULONG)TRUE },
            { TAG_DONE, 0 }
        };
        gui->input = NewObjectA(hist_string_mcc->mcc_Class, NULL, tags);
    } else {
        /* Fallback to standard String if custom class fails */
        struct TagItem tags[] = {
            { MUIA_Frame, MUIV_Frame_String },
            { MUIA_String_MaxLen, (ULONG)1024 },
            { MUIA_CycleChain, (ULONG)TRUE },
            { TAG_DONE, 0 }
        };
        gui->input = MUI_NewObjectA((CONST_STRPTR)MUIC_String, tags);
    }
    printf("  gui: input=%p\n", (void *)gui->input);

    printf("  gui: creating send button...\n");
    {
        ULONG params[1];
        params[0] = (ULONG)"_Send";
        gui->send_btn = MUI_MakeObjectA(MUIO_Button, params);
    }
    printf("  gui: send_btn=%p\n", (void *)gui->send_btn);

    printf("  gui: creating stop button...\n");
    {
        ULONG params[1];
        params[0] = (ULONG)"S_top";
        gui->stop_btn = MUI_MakeObjectA(MUIO_Button, params);
    }
    /* Stop button starts disabled */
    xset(gui->stop_btn, MUIA_Disabled, TRUE);
    printf("  gui: stop_btn=%p\n", (void *)gui->stop_btn);

    printf("  gui: creating status...\n");
    {
        struct TagItem tags[] = {
            { MUIA_Frame, MUIV_Frame_Text },
            { MUIA_Text_Contents, (ULONG)"Ready." },
            { MUIA_Text_SetMin, (ULONG)FALSE },
            { TAG_DONE, 0 }
        };
        gui->status = MUI_NewObjectA((CONST_STRPTR)MUIC_Text, tags);
    }
    printf("  gui: status=%p\n", (void *)gui->status);

    /* Use NewObjectA with explicit TagItem arrays everywhere to avoid
     * broken variadic stubs in GCC 13's m68k backend. */
    printf("  gui: getting Window class...\n");
    {
        struct IClass *wincl = MUI_GetClass((CONST_STRPTR)MUIC_Window);
        printf("  gui: MUI_GetClass(Window.mui) = %p\n", (void *)wincl);

        if (wincl) {
            Object *vgrp, *hgrp;

            /* Build the input + send button HGroup.
             * Give input more weight so it takes most of the width. */
            xset(gui->input,    MUIA_HorizWeight, 200);
            xset(gui->send_btn, MUIA_HorizWeight, 25);
            xset(gui->stop_btn, MUIA_HorizWeight, 25);
            {
                struct TagItem tags[] = {
                    { MUIA_Group_Horiz,  TRUE },
                    { MUIA_Group_Child,  (ULONG)gui->input },
                    { MUIA_Group_Child,  (ULONG)gui->send_btn },
                    { MUIA_Group_Child,  (ULONG)gui->stop_btn },
                    { TAG_DONE, 0 }
                };
                hgrp = MUI_NewObjectA((CONST_STRPTR)MUIC_Group, tags);
            }

            /* Build editor + scrollbar HGroup */
            {
                Object *editor_grp;
                struct TagItem eg_tags[] = {
                    { MUIA_Group_Horiz,  TRUE },
                    { MUIA_Group_Child,  (ULONG)gui->editor },
                    { MUIA_Group_Child,  (ULONG)gui->scrollbar },
                    { TAG_DONE, 0 }
                };
                editor_grp = MUI_NewObjectA((CONST_STRPTR)MUIC_Group, eg_tags);

                /* Build the main VGroup */
                printf("  gui: building VGroup...\n");
                {
                    struct TagItem tags[] = {
                        { MUIA_Group_Child,  (ULONG)editor_grp },
                        { MUIA_Group_Child,  (ULONG)hgrp },
                        { MUIA_Group_Child,  (ULONG)gui->status },
                        { TAG_DONE, 0 }
                    };
                    vgrp = MUI_NewObjectA((CONST_STRPTR)MUIC_Group, tags);
                }
            }
            printf("  gui: vgroup=%p\n", (void *)vgrp);

            /* Create the window */
            printf("  gui: creating Window via NewObjectA...\n");
            {
                struct TagItem tags[] = {
                    { MUIA_Window_Title,         (ULONG)PROGRAM_NAME " " VERSION_STRING },
                    { MUIA_Window_ID,            MAKE_ID('M','A','I','N') },
                    { MUIA_Window_RootObject,    (ULONG)vgrp },
                    { MUIA_Window_DefaultObject, (ULONG)gui->input },
                    { TAG_DONE, 0 }
                };
                gui->win = NewObjectA(wincl, NULL, tags);
            }
            printf("  gui: win=%p\n", (void *)gui->win);

            if (!gui->win) {
                /* Try without contents */
                struct TagItem tags[] = {
                    { MUIA_Window_Title, (ULONG)"AmigaAI" },
                    { TAG_DONE, 0 }
                };
                printf("  gui: trying Window without contents...\n");
                gui->win = NewObjectA(wincl, NULL, tags);
                printf("  gui: win(no contents)=%p\n", (void *)gui->win);
            }

            /* Don't FreeClass - window holds reference */
            if (!gui->win) MUI_FreeClass(wincl);
        }
    }

    printf("  gui: creating ApplicationObject...\n");
    {
        struct TagItem tags[] = {
            { MUIA_Application_Title,       (ULONG)PROGRAM_NAME },
            { MUIA_Application_Version,     (ULONG)VERSTAG + 1 },
            { MUIA_Application_Copyright,   (ULONG)"2026" },
            { MUIA_Application_Author,      (ULONG)"AmigaAI" },
            { MUIA_Application_Description, (ULONG)"Claude AI Agent for AmigaOS" },
            { MUIA_Application_Base,        (ULONG)"AMIGAAI" },
            { MUIA_Application_Menustrip,   (ULONG)gui->menustrip },
            { MUIA_Application_Window,      (ULONG)gui->win },
            { TAG_DONE, 0 }
        };
        gui->app = MUI_NewObjectA((CONST_STRPTR)MUIC_Application, tags);
    }

    printf("  gui: app=%p win=%p\n", (void *)gui->app, (void *)gui->win);

    if (!gui->app) {
        /* Try without menustrip */
        struct TagItem tags[] = {
            { MUIA_Application_Title,       (ULONG)PROGRAM_NAME },
            { MUIA_Application_Version,     (ULONG)VERSTAG + 1 },
            { MUIA_Application_Copyright,   (ULONG)"2026" },
            { MUIA_Application_Author,      (ULONG)"AmigaAI" },
            { MUIA_Application_Description, (ULONG)"Claude AI Agent for AmigaOS" },
            { MUIA_Application_Base,        (ULONG)"AMIGAAI" },
            { MUIA_Application_Window,      (ULONG)gui->win },
            { TAG_DONE, 0 }
        };
        printf("  gui: retrying app without menustrip...\n");
        gui->app = MUI_NewObjectA((CONST_STRPTR)MUIC_Application, tags);
        printf("  gui: app(no menu)=%p\n", (void *)gui->app);
    }

    if (!gui->app) {
        printf("ERROR: Failed to create MUI application.\n");
        printf("Make sure MUI is installed.\n");
        return -1;
    }

    if (!gui->win) {
        printf("ERROR: Failed to create MUI window.\n");
        MUI_DisposeObject(gui->app);
        gui->app = NULL;
        return -1;
    }

    printf("  gui: editor=%p input=%p btn=%p status=%p\n",
           (void *)gui->editor, (void *)gui->input,
           (void *)gui->send_btn, (void *)gui->status);

    /* Notifications - use DoMethodA with ULONG arrays to avoid variadic issues */
    printf("  gui: setting up notifications...\n");

    /* Window close -> Quit */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
                         (ULONG)gui->app, 2,
                         MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit };
        DoMethodA(gui->win, (Msg)msg);
    }

    /* Send button -> GUI_ID_SEND */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Pressed, FALSE,
                         (ULONG)gui->app, 2,
                         MUIM_Application_ReturnID, GUI_ID_SEND };
        DoMethodA(gui->send_btn, (Msg)msg);
    }

    /* Stop button -> GUI_ID_STOP */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_Pressed, FALSE,
                         (ULONG)gui->app, 2,
                         MUIM_Application_ReturnID, GUI_ID_STOP };
        DoMethodA(gui->stop_btn, (Msg)msg);
    }

    /* String gadget Enter/Acknowledge -> GUI_ID_SEND */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
                         (ULONG)gui->app, 2,
                         MUIM_Application_ReturnID, GUI_ID_SEND };
        DoMethodA(gui->input, (Msg)msg);
    }

    printf("  gui: menu notifications...\n");

/* Helper macro: use DoMethodA to find menu items and set notifications */
#define MENU_NOTIFY(id, ret_id) \
    do { \
        ULONG _find[] = { MUIM_FindUData, (id) }; \
        Object *_mi = (Object *)DoMethodA(gui->menustrip, (Msg)_find); \
        if (_mi) { \
            ULONG _nmsg[] = { MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, \
                              (ULONG)gui->app, 2, MUIM_Application_ReturnID, (ret_id) }; \
            DoMethodA(_mi, (Msg)_nmsg); \
        } else \
            printf("WARNING: menu item %lu not found\n", (unsigned long)(id)); \
    } while(0)

    /* Menu notifications - Project */
    MENU_NOTIFY(GUI_ID_NEW,      GUI_ID_NEW);
    MENU_NOTIFY(GUI_ID_CHATSAVE, GUI_ID_CHATSAVE);
    MENU_NOTIFY(GUI_ID_CHATLOAD, GUI_ID_CHATLOAD);
    MENU_NOTIFY(GUI_ID_ABOUT,    GUI_ID_ABOUT);
    MENU_NOTIFY(GUI_ID_QUIT,     MUIV_Application_ReturnID_Quit);

    /* Menu notifications - Settings */
    MENU_NOTIFY(GUI_ID_APIKEY, GUI_ID_APIKEY);
    MENU_NOTIFY(GUI_ID_MODEL,  GUI_ID_MODEL);
    MENU_NOTIFY(GUI_ID_SYSTEM, GUI_ID_SYSTEM);

    /* Menu notifications - Memory */
    MENU_NOTIFY(GUI_ID_MEMVIEW,  GUI_ID_MEMVIEW);
    MENU_NOTIFY(GUI_ID_MEMADD,   GUI_ID_MEMADD);
    MENU_NOTIFY(GUI_ID_MEMCLEAR, GUI_ID_MEMCLEAR);

#undef MENU_NOTIFY

    printf("  gui: opening window...\n");
    /* Open the window */
    xset(gui->win, MUIA_Window_Open, TRUE);

    {
        ULONG open = xget(gui->win, MUIA_Window_Open);
        if (!open) {
            printf("ERROR: Cannot open MUI window\n");
            MUI_DisposeObject(gui->app);
            gui->app = NULL;
            return -2;
        }
    }

    /* Activate the input field */
    gui_focus_input(gui);

    /* Auto-reactivate input after Enter (Acknowledge). */
    {
        ULONG msg[] = { MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
                         (ULONG)gui->win, 3,
                         MUIM_Set, MUIA_Window_ActiveObject, (ULONG)gui->input };
        DoMethodA(gui->input, (Msg)msg);
    }

    /* Welcome message */
    gui_add_line(gui, "Welcome to " PROGRAM_NAME " " VERSION_STRING);
    gui_add_line(gui, "Type a message and press Enter or click Send.");
    gui_add_line(gui, "");

    return 0;
}

void gui_close(struct Gui *gui)
{
    if (gui->app) {
        MUI_DisposeObject(gui->app);
        gui->app = NULL;
    }
    /* Delete custom class after all objects using it are disposed */
    if (hist_string_mcc) {
        MUI_DeleteCustomClass(hist_string_mcc);
        hist_string_mcc = NULL;
    }
    edit_hook_gui = NULL;
    memset(gui, 0, sizeof(*gui));
}

ULONG gui_process(struct Gui *gui, ULONG *signals)
{
    ULONG id;

    if (!gui->app) {
        *signals = 0;
        return MUIV_Application_ReturnID_Quit;
    }

    {
        ULONG msg[] = { MUIM_Application_NewInput, (ULONG)signals };
        id = DoMethodA(gui->app, (Msg)msg);
    }
    return id;
}

const char *gui_get_input(struct Gui *gui)
{
    char *str = NULL;
    if (gui->input)
        str = (char *)xget(gui->input, MUIA_String_Contents);
    return str ? str : "";
}

void gui_clear_input(struct Gui *gui)
{
    if (gui->input) {
        /* Use MUIA_NoNotify to minimise side effects that could
         * deactivate the string gadget. */
        struct TagItem tags[3];
        tags[0].ti_Tag  = MUIA_NoNotify;
        tags[0].ti_Data = TRUE;
        tags[1].ti_Tag  = MUIA_String_Contents;
        tags[1].ti_Data = (ULONG)"";
        tags[2].ti_Tag  = TAG_DONE;
        SetAttrsA(gui->input, tags);
    }
}

void gui_add_line(struct Gui *gui, const char *text)
{
    char buf[1024];

    if (!gui->editor) return;

    snprintf(buf, sizeof(buf), "%s\n", text);

    {
        ULONG msg[] = { MUIM_TextEditor_InsertText,
                         (ULONG)buf,
                         MUIV_TextEditor_InsertText_Bottom };
        DoMethodA(gui->editor, (Msg)msg);
    }
}

/* Emit color escape: \033P[rrggbb] for RGB, \033p[0] for reset */
#define EMIT_BLUE(d, end) do { \
    if ((d) + 10 <= (end)) { \
        *(d)++ = '\033'; *(d)++ = 'P'; *(d)++ = '['; \
        *(d)++ = '0'; *(d)++ = '0'; *(d)++ = '0'; *(d)++ = '0'; \
        *(d)++ = 'c'; *(d)++ = 'c'; *(d)++ = ']'; \
    } } while(0)
#define EMIT_COLOR_RESET(d, end) do { \
    if ((d) + 4 <= (end)) { \
        *(d)++ = '\033'; *(d)++ = 'p'; *(d)++ = '['; \
        *(d)++ = '0'; *(d)++ = ']'; \
    } } while(0)

/* Convert Markdown formatting to MUI TextEditor escape codes.
 * **bold** -> \033b...\033n, *italic* -> \033i...\033n,
 * # headings -> bold+underline, `code` -> blue, ``` blocks -> blue.
 * in_code_block is persistent state for multi-line ``` fences. */
static void strip_markdown(const char *src, char *dst, int dstsize,
                           int *in_code_block)
{
    const char *s = src;
    char *d = dst;
    char *end = dst + dstsize - 12; /* leave room for escape codes */
    int in_bold = 0;
    int in_italic = 0;

    /* Check for ``` code fence toggle */
    if (s[0] == '`' && s[1] == '`' && s[2] == '`') {
        *in_code_block = !*in_code_block;
        /* Skip the fence line entirely (including language tag) */
        *dst = '\0';
        return;
    }

    /* Inside code block: output entire line in blue */
    if (*in_code_block) {
        EMIT_BLUE(d, end);
        while (*s && d < end)
            *d++ = *s++;
        EMIT_COLOR_RESET(d, end);
        *d = '\0';
        return;
    }

    /* Strip leading # headings -> make bold + underline */
    if (*s == '#') {
        while (*s == '#') s++;
        if (*s == ' ') s++;
        /* Skip converted emoji (UTF-8 emoji become ? in Latin1) */
        while (*s == '?' || *s == ' ') s++;
        if (d + 4 <= end) { *d++ = '\033'; *d++ = 'b'; *d++ = '\033'; *d++ = 'u'; }
        in_bold = 1;
    }

    while (*s && d < end) {
        /* **bold** or __bold__ -> toggle MUI bold */
        if ((s[0] == '*' && s[1] == '*') || (s[0] == '_' && s[1] == '_')) {
            s += 2;
            if (!in_bold) {
                if (d + 2 <= end) { *d++ = '\033'; *d++ = 'b'; }
                in_bold = 1;
            } else {
                if (d + 2 <= end) { *d++ = '\033'; *d++ = 'n'; }
                in_bold = 0;
            }
            continue;
        }
        /* *italic* or _italic_ - only at word boundaries to avoid
         * false matches in filenames like my_file or *.txt */
        if ((*s == '*' || *s == '_') && s[1] != ' ' && s[1] != '\0') {
            char marker = *s;
            /* Don't match if next char is also the marker (handled as bold above) */
            if (s[1] != marker) {
                /* Opening marker must be at start of line or after space/punctuation */
                int at_boundary = (s == src || s[-1] == ' ' || s[-1] == '('
                                   || s[-1] == '\033');
                if (at_boundary) {
                    const char *close = strchr(s + 1, marker);
                    /* Closing marker must be followed by space/punctuation/EOL */
                    if (close && close > s + 1 && *(close - 1) != ' '
                        && (close[1] == '\0' || close[1] == ' '
                            || close[1] == '.' || close[1] == ','
                            || close[1] == ')' || close[1] == ':'
                            || close[1] == ';' || close[1] == '!')) {
                        s++;
                        if (!in_italic) {
                            if (d + 2 <= end) { *d++ = '\033'; *d++ = 'i'; }
                            in_italic = 1;
                        } else {
                            if (d + 2 <= end) { *d++ = '\033'; *d++ = 'n'; }
                            in_italic = 0;
                        }
                        continue;
                    }
                }
            }
        }
        /* `inline code` -> blue text */
        if (*s == '`') {
            const char *close = strchr(s + 1, '`');
            if (close && close > s + 1) {
                s++;
                EMIT_BLUE(d, end);
                while (s < close && d < end)
                    *d++ = *s++;
                EMIT_COLOR_RESET(d, end);
                s++; /* skip closing backtick */
                continue;
            }
        }
        *d++ = *s++;
    }
    /* Close unclosed styles */
    if ((in_bold || in_italic) && d + 2 <= end) {
        *d++ = '\033'; *d++ = 'n';
    }
    *d = '\0';
}

void gui_add_text(struct Gui *gui, const char *prefix, const char *text)
{
    const char *p = text;
    char line_buf[1024];
    char clean_buf[2048]; /* larger: escape codes expand text */
    int first = 1;
    int in_code_block = 0;

    while (*p) {
        const char *nl = strchr(p, '\n');
        int len;

        if (nl)
            len = nl - p;
        else
            len = strlen(p);

        if (first && prefix) {
            snprintf(line_buf, sizeof(line_buf), "%s%.*s", prefix, len, p);
            first = 0;
        } else {
            snprintf(line_buf, sizeof(line_buf), "%.*s", len, p);
        }

        /* Convert Markdown formatting to MUI escape codes */
        strip_markdown(line_buf, clean_buf, sizeof(clean_buf), &in_code_block);
        /* Skip empty lines from ``` fence markers */
        if (clean_buf[0] != '\0' || (line_buf[0] != '`'))
            gui_add_line(gui, clean_buf);

        p += len;
        if (*p == '\n') p++;
    }
}

void gui_set_status(struct Gui *gui, const char *text)
{
    if (gui->status)
        xset(gui->status, MUIA_Text_Contents, (ULONG)text);
}

void gui_set_busy(struct Gui *gui, int busy)
{
    gui->busy = busy;

    if (busy) {
        gui->abort_requested = 0;
        /* Disable input and Send, enable Stop */
        if (gui->input)    xset(gui->input,    MUIA_Disabled, TRUE);
        if (gui->send_btn) xset(gui->send_btn, MUIA_Disabled, TRUE);
        if (gui->stop_btn) xset(gui->stop_btn, MUIA_Disabled, FALSE);
    } else {
        /* Enable input and Send, disable Stop */
        if (gui->stop_btn) xset(gui->stop_btn, MUIA_Disabled, TRUE);
        if (gui->send_btn) xset(gui->send_btn, MUIA_Disabled, FALSE);
        if (gui->input)    xset(gui->input,    MUIA_Disabled, FALSE);
        gui_focus_input(gui);
    }
}

void gui_clear_chat(struct Gui *gui)
{
    if (gui->editor)
    {
        ULONG msg[] = { MUIM_TextEditor_ClearText };
        DoMethodA(gui->editor, (Msg)msg);
    }
}

void gui_about(struct Gui *gui, const char *title, const char *body)
{
    if (gui->app)
        MUI_RequestA(gui->app, gui->win, 0,
                     (CONST_STRPTR)title,
                     (CONST_STRPTR)"OK",
                     (CONST_STRPTR)body,
                     NULL);
}

void gui_focus_input(struct Gui *gui)
{
    /* Tab/CycleChain works reliably for focus, so simulate it:
     * first deactivate, then cycle to the next (=first) chain member.
     * Since our input field is the only CycleChain object, this
     * effectively activates it. */
    if (gui->win && gui->input) {
        xset(gui->win, MUIA_Window_ActiveObject,
             MUIV_Window_ActiveObject_None);
        xset(gui->win, MUIA_Window_ActiveObject,
             MUIV_Window_ActiveObject_Next);
    }
}

int gui_check_abort(struct Gui *gui)
{
    ULONG sigs = 0;
    ULONG id;

    if (!gui->app || gui->abort_requested)
        return gui->abort_requested;

    /* Process pending MUI events without blocking */
    {
        ULONG msg[] = { MUIM_Application_NewInput, (ULONG)&sigs };
        id = DoMethodA(gui->app, (Msg)msg);
    }

    if (id == GUI_ID_STOP || id == MUIV_Application_ReturnID_Quit) {
        gui->abort_requested = 1;
    }

    return gui->abort_requested;
}

void gui_history_push(struct Gui *gui, const char *text)
{
    int idx;

    if (!text || !text[0]) return;

    /* Store at next slot in ring buffer */
    idx = gui->hist_count % GUI_HISTORY_SIZE;
    strncpy(gui->history[idx], text, GUI_HISTORY_LEN - 1);
    gui->history[idx][GUI_HISTORY_LEN - 1] = '\0';
    gui->hist_count++;

    /* Reset browse position */
    gui->hist_pos = -1;
}
