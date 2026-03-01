#include "locale.h"
#include "version.h"

#include <proto/exec.h>

#ifdef __amigaos__
#include <proto/locale.h>
#include <libraries/locale.h>
#else
/* Cross-compile stub */
#endif

#ifdef __amigaos__
struct LocaleBase *LocaleBase = NULL;
#endif
static void *catalog = NULL;

static const char * const default_strings[MSG_COUNT] = {
    /* MSG_MENU_PROJECT       */  "Project",
    /* MSG_MENU_SETTINGS      */  "Settings",
    /* MSG_MENU_MEMORY        */  "Memory",
    /* MSG_MENU_NEW_CHAT      */  "New Chat",
    /* MSG_MENU_SAVE_CHAT     */  "Save Chat...",
    /* MSG_MENU_LOAD_CHAT     */  "Load Chat...",
    /* MSG_MENU_ABOUT         */  "About...",
    /* MSG_MENU_QUIT          */  "Quit",
    /* MSG_MENU_APIKEY        */  "API Key...",
    /* MSG_MENU_MODEL         */  "Model...",
    /* MSG_MENU_SYSTEM_PROMPT */  "System Prompt...",
    /* MSG_MENU_VIEW_MEMORY   */  "View Memory...",
    /* MSG_MENU_ADD_MEMORY    */  "Add Memory...",
    /* MSG_MENU_CLEAR_MEMORY  */  "Clear Memory",
    /* MSG_BTN_SEND           */  "_Send",
    /* MSG_BTN_STOP           */  "S_top",
    /* MSG_BTN_OK             */  "_OK",
    /* MSG_BTN_CANCEL         */  "_Cancel",
    /* MSG_STATUS_READY       */  "Ready.",
    /* MSG_STATUS_SENDING     */  "Sending to Claude...",
    /* MSG_STATUS_ABORTED     */  "Request aborted.",
    /* MSG_STATUS_ERROR       */  "Error",
    /* MSG_STATUS_CHAT_CLEARED */ "Chat cleared. Ready.",
    /* MSG_STATUS_TYPE_CMD    */  "Type a command or message",
    /* MSG_STATUS_EXECUTING   */  "Executing shell command...",
    /* MSG_STATUS_AREXX_SENDING */ "Sending ARexx command...",
    /* MSG_STATUS_NO_APIKEY   */  "No API key - configure in Settings",
    /* MSG_WELCOME            */  "Welcome to " PROGRAM_NAME " " VERSION_STRING,
    /* MSG_WELCOME_HINT       */  "Type a message and press Enter or click Send.",
    /* MSG_LABEL_YOU          */  "\033bYou:\033n ",
    /* MSG_LABEL_CLAUDE       */  "\033bClaude:\033n ",
    /* MSG_LABEL_ABORTED      */  "\033b\033c--- Aborted ---\033n",
    /* MSG_LABEL_NEW_CHAT     */  "\033c--- New Chat ---",
    /* MSG_LABEL_ERROR        */  "\033b\033cERROR:\033n ",
    /* MSG_STATUS_TOKENS      */  "Tokens: %d in / %d out | Messages: %d",
    /* MSG_HELP_TITLE         */  "\033b\033cAvailable Commands\033n",
    /* MSG_HELP_CMD_HELP      */  "\033b/help\033n              - Show this help",
    /* MSG_HELP_CMD_SHELL     */  "\033b/shell <cmd>\033n      - Run AmigaDOS command",
    /* MSG_HELP_CMD_AREXX     */  "\033b/arexx <port> <cmd>\033n - Send ARexx command",
    /* MSG_HELP_CMD_READ      */  "\033b/read <path>\033n      - Read a file",
    /* MSG_HELP_CMD_WRITE     */  "\033b/write <path> <text>\033n - Write to a file",
    /* MSG_HELP_CMD_PORTS     */  "\033b/ports\033n            - List public message ports",
    /* MSG_HELP_CMD_REMEMBER  */  "\033b/remember <text>\033n  - Save a memory entry",
    /* MSG_HELP_CMD_MEMORY    */  "\033b/memory\033n           - View all memory entries",
    /* MSG_HELP_FOOTER1       */  "Any other text is sent to Claude as a message.",
    /* MSG_HELP_FOOTER2       */  "Use cursor up/down to browse input history.",
    /* MSG_ABOUT_DESCRIPTION  */  "Claude AI Agent for AmigaOS",
    /* MSG_ABOUT_STACK        */  "Stack: AmiSSL v5, cJSON, MUI",
    /* MSG_MEM_TITLE_VIEW     */  "Persistent Memory",
    /* MSG_MEM_TITLE_ADD      */  "Add Memory Entry",
    /* MSG_MEM_TITLE_CLEAR    */  "Clear Memory",
    /* MSG_MEM_ENTER_FACT     */  "Enter a fact or preference to remember:",
    /* MSG_MEM_ADDED          */  "Memory entry added.",
    /* MSG_MEM_FULL           */  "Memory is full!",
    /* MSG_MEM_NONE           */  "No memories stored yet.",
    /* MSG_MEM_NONE_BODY      */  "No memories stored yet.\n\nUse Memory > Add Memory or type:\n/remember <fact>",
    /* MSG_MEM_CLEAR_CONFIRM  */  "Clear all memory entries?\nThis cannot be undone.",
    /* MSG_MEM_CLEARED        */  "All memory cleared.",
    /* MSG_MEM_CLEAR_BUTTONS  */  "*_Clear|_Cancel",
    /* MSG_MODEL_TITLE        */  "Select Model",
    /* MSG_MODEL_SET          */  "Model: %s",
    /* MSG_CHAT_SAVE_TITLE    */  "Save Chat",
    /* MSG_CHAT_SAVE_NONE     */  "No messages to save.",
    /* MSG_CHAT_SAVE_FAIL     */  "Failed to serialize chat.",
    /* MSG_CHAT_SAVE_OK       */  "Chat saved to chat.json",
    /* MSG_CHAT_SAVE_WRITE_FAIL */ "Failed to save chat!",
    /* MSG_CHAT_LOAD_NONE     */  "No saved chat found (chat.json).",
    /* MSG_CHAT_LOAD_TOO_LARGE */ "Chat file too large or empty.",
    /* MSG_CHAT_LOAD_OOM      */  "Out of memory.",
    /* MSG_CHAT_LOAD_READ_FAIL */ "Failed to read chat file.",
    /* MSG_CHAT_LOADED        */  "Chat loaded (%d messages)",
    /* MSG_CHAT_LOADED_LINE   */  "\033c--- Chat Loaded ---",
    /* MSG_CHAT_LOAD_PARSE_FAIL */ "Failed to parse saved chat!",
    /* MSG_ERR_OOM_HISTORY    */  "ERROR: Out of memory clearing history",
    /* MSG_ERR_UNKNOWN        */  "Unknown error",
    /* MSG_CMD_SHELL          */  "\033bShell:\033n %s",
    /* MSG_CMD_AREXX          */  "\033bARexx:\033n %s > %s",
    /* MSG_CMD_AREXX_USAGE    */  "Usage: /arexx <PORT> <command>",
    /* MSG_CMD_READ           */  "\033bRead:\033n %s",
    /* MSG_CMD_WRITE_USAGE    */  "Usage: /write <path> <content>",
    /* MSG_CMD_DONE           */  "Done",
    /* MSG_CMD_FAILED         */  "Command failed",
    /* MSG_CMD_PORTS_TITLE    */  "\033bPublic Ports:\033n",
    /* MSG_CMD_PORTS_LISTED   */  "Ports listed",
    /* MSG_CMD_MEM_ENTRIES    */  "Memory: %d entries",
    /* MSG_WARN_NO_APIKEY     */  "\033bWARNING:\033n No API key configured!",
    /* MSG_WARN_SET_APIKEY    */  "Use Settings > API Key or set ENV:AmigaAI/api_key",
    /* MSG_WARN_MEM_LOADED    */  "Memory: %d entries loaded",
    /* MSG_SYSTEM_COMING_SOON */  "System prompt editor - coming soon",
    /* MSG_APP_DESCRIPTION    */  "Claude AI Agent for AmigaOS",
    /* MSG_APIKEY_HINT        */  "Set API key via: echo \"key\" > ENV:AmigaAI/api_key",
};

const char *GetString(int id)
{
    if (id < 0 || id >= MSG_COUNT)
        return "";

#ifdef __amigaos__
    if (catalog)
        return (const char *)GetCatalogStr(
            (struct Catalog *)catalog, id,
            (CONST_STRPTR)default_strings[id]);
#endif

    return default_strings[id];
}

void locale_open(void)
{
#ifdef __amigaos__
    LocaleBase = (struct LocaleBase *)OpenLibrary((CONST_STRPTR)"locale.library", 38);
    if (LocaleBase) {
        struct TagItem tags[] = {
            { OC_BuiltInLanguage, (ULONG)"english" },
            { TAG_DONE, 0 }
        };
        catalog = OpenCatalogA(NULL, (CONST_STRPTR)"AmigaAI.catalog", tags);
    }
#endif
}

void locale_close(void)
{
#ifdef __amigaos__
    if (catalog) {
        CloseCatalog((struct Catalog *)catalog);
        catalog = NULL;
    }
    if (LocaleBase) {
        CloseLibrary((struct Library *)LocaleBase);
        LocaleBase = NULL;
    }
#endif
}
