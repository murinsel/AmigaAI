#ifndef AMIGAAI_LOCALE_H
#define AMIGAAI_LOCALE_H

/* String IDs for AmigaAI localization */

/* Menu titles */
#define MSG_MENU_PROJECT           0
#define MSG_MENU_SETTINGS          1
#define MSG_MENU_MEMORY            2

/* Menu items - Project */
#define MSG_MENU_NEW_CHAT          3
#define MSG_MENU_SAVE_CHAT         4
#define MSG_MENU_LOAD_CHAT         5
#define MSG_MENU_ABOUT             6
#define MSG_MENU_QUIT              7

/* Menu items - Settings */
#define MSG_MENU_APIKEY            8
#define MSG_MENU_MODEL             9
#define MSG_MENU_SYSTEM_PROMPT     10

/* Menu items - Memory */
#define MSG_MENU_VIEW_MEMORY       11
#define MSG_MENU_ADD_MEMORY        12
#define MSG_MENU_CLEAR_MEMORY      13

/* Buttons */
#define MSG_BTN_SEND               14
#define MSG_BTN_STOP               15
#define MSG_BTN_OK                 16
#define MSG_BTN_CANCEL             17

/* Status bar */
#define MSG_STATUS_READY           18
#define MSG_STATUS_SENDING         19
#define MSG_STATUS_ABORTED         20
#define MSG_STATUS_ERROR           21
#define MSG_STATUS_CHAT_CLEARED    22
#define MSG_STATUS_TYPE_CMD        23
#define MSG_STATUS_EXECUTING       24
#define MSG_STATUS_AREXX_SENDING   25
#define MSG_STATUS_NO_APIKEY       26

/* Welcome */
#define MSG_WELCOME                27
#define MSG_WELCOME_HINT           28

/* Labels */
#define MSG_LABEL_YOU              29
#define MSG_LABEL_CLAUDE           30
#define MSG_LABEL_ABORTED          31
#define MSG_LABEL_NEW_CHAT         32
#define MSG_LABEL_ERROR            33

/* Token status */
#define MSG_STATUS_TOKENS          34

/* Help text */
#define MSG_HELP_TITLE             35
#define MSG_HELP_CMD_HELP          36
#define MSG_HELP_CMD_SHELL         37
#define MSG_HELP_CMD_AREXX         38
#define MSG_HELP_CMD_READ          39
#define MSG_HELP_CMD_WRITE         40
#define MSG_HELP_CMD_PORTS         41
#define MSG_HELP_CMD_REMEMBER      42
#define MSG_HELP_CMD_MEMORY        43
#define MSG_HELP_FOOTER1           44
#define MSG_HELP_FOOTER2           45

/* About dialog */
#define MSG_ABOUT_DESCRIPTION      46
#define MSG_ABOUT_STACK            47

/* Memory dialogs */
#define MSG_MEM_TITLE_VIEW         48
#define MSG_MEM_TITLE_ADD          49
#define MSG_MEM_TITLE_CLEAR        50
#define MSG_MEM_ENTER_FACT         51
#define MSG_MEM_ADDED              52
#define MSG_MEM_FULL               53
#define MSG_MEM_NONE               54
#define MSG_MEM_NONE_BODY          55
#define MSG_MEM_CLEAR_CONFIRM      56
#define MSG_MEM_CLEARED            57
#define MSG_MEM_CLEAR_BUTTONS      58

/* Model dialog */
#define MSG_MODEL_TITLE            59
#define MSG_MODEL_SET              60

/* Chat save/load */
#define MSG_CHAT_SAVE_TITLE        61
#define MSG_CHAT_SAVE_NONE         62
#define MSG_CHAT_SAVE_FAIL         63
#define MSG_CHAT_SAVE_OK           64
#define MSG_CHAT_SAVE_WRITE_FAIL   65
#define MSG_CHAT_LOAD_NONE         66
#define MSG_CHAT_LOAD_TOO_LARGE    67
#define MSG_CHAT_LOAD_OOM          68
#define MSG_CHAT_LOAD_READ_FAIL    69
#define MSG_CHAT_LOADED            70
#define MSG_CHAT_LOADED_LINE       71
#define MSG_CHAT_LOAD_PARSE_FAIL   72

/* Errors */
#define MSG_ERR_OOM_HISTORY        73
#define MSG_ERR_UNKNOWN            74

/* Slash command feedback */
#define MSG_CMD_SHELL              75
#define MSG_CMD_AREXX              76
#define MSG_CMD_AREXX_USAGE        77
#define MSG_CMD_READ               78
#define MSG_CMD_WRITE_USAGE        79
#define MSG_CMD_DONE               80
#define MSG_CMD_FAILED             81
#define MSG_CMD_PORTS_TITLE        82
#define MSG_CMD_PORTS_LISTED       83
#define MSG_CMD_MEM_ENTRIES        84

/* Startup warnings */
#define MSG_WARN_NO_APIKEY         85
#define MSG_WARN_SET_APIKEY        86
#define MSG_WARN_MEM_LOADED        87

/* System prompt editor */
#define MSG_SYSTEM_COMING_SOON     88

/* Application description */
#define MSG_APP_DESCRIPTION        89

/* API key hint */
#define MSG_APIKEY_HINT            90

#define MSG_COUNT                  91

/* Locale functions */
void locale_open(void);
void locale_close(void);
const char *GetString(int id);

#endif /* AMIGAAI_LOCALE_H */
