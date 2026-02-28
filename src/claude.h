#ifndef AMIGAAI_CLAUDE_H
#define AMIGAAI_CLAUDE_H

#include "config.h"
#include "memory.h"
#include "cJSON.h"

#define CLAUDE_API_HOST    "api.anthropic.com"
#define CLAUDE_API_PATH    "/v1/messages"
#define CLAUDE_API_VERSION "2023-06-01"

/* Callback for tool use status updates.
 * status: "executing", "done", "error"
 * detail: tool input summary (executing) or result text (done/error) */
typedef void (*ToolStatusCallback)(const char *tool_name,
                                   const char *status,
                                   const char *detail,
                                   void *userdata);

struct Claude {
    struct Config   *config;
    struct Memory   *memory;       /* Persistent memory for system prompt */
    cJSON           *messages;     /* JSON array of conversation messages */
    cJSON           *tools;        /* Tool definitions for API (NULL = no tools) */
    int              last_input_tokens;
    int              last_output_tokens;

    /* Optional callback for tool use status updates */
    ToolStatusCallback tool_cb;
    void              *tool_cb_data;
};

/* Initialize Claude API context. Returns 0 on success. */
int claude_init(struct Claude *ctx, struct Config *cfg, struct Memory *mem);

/* Free Claude API context resources. */
void claude_cleanup(struct Claude *ctx);

/* Set tool status callback (called during tool execution). */
void claude_set_tool_callback(struct Claude *ctx,
                              ToolStatusCallback cb, void *userdata);

/* Send a user message and get the assistant's reply.
 * Automatically handles tool use loops (up to TOOLS_MAX_ITERATIONS).
 * Returns a newly allocated string (caller must free) or NULL on error.
 * On error, *error_msg (if not NULL) is set to an error description. */
char *claude_send(struct Claude *ctx, const char *user_message, char **error_msg);

/* Clear conversation history. Returns 0 on success, -1 on alloc failure. */
int claude_clear_history(struct Claude *ctx);

/* Get number of messages in conversation. */
int claude_message_count(struct Claude *ctx);

#endif /* AMIGAAI_CLAUDE_H */
