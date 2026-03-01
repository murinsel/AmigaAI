#include "claude.h"
#include "http.h"
#include "json_utils.h"
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int claude_init(struct Claude *ctx, struct Config *cfg, struct Memory *mem)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->config = cfg;
    ctx->memory = mem;

    /* Create empty messages array */
    ctx->messages = cJSON_CreateArray();
    if (!ctx->messages) return -1;

    /* Build tool definitions */
    ctx->tools = tools_build_json();

    return 0;
}

void claude_cleanup(struct Claude *ctx)
{
    if (ctx->messages) {
        cJSON_Delete(ctx->messages);
        ctx->messages = NULL;
    }
    if (ctx->tools) {
        cJSON_Delete(ctx->tools);
        ctx->tools = NULL;
    }
}

void claude_set_tool_callback(struct Claude *ctx,
                              ToolStatusCallback cb, void *userdata)
{
    ctx->tool_cb = cb;
    ctx->tool_cb_data = userdata;
}

int claude_clear_history(struct Claude *ctx)
{
    cJSON *new_arr;

    if (ctx->messages)
        cJSON_Delete(ctx->messages);

    new_arr = cJSON_CreateArray();
    if (!new_arr) {
        ctx->messages = NULL;
        return -1;
    }
    ctx->messages = new_arr;
    return 0;
}

int claude_message_count(struct Claude *ctx)
{
    if (!ctx->messages) return 0;
    return cJSON_GetArraySize(ctx->messages);
}

/* Build the effective system prompt from memory + config + tool hints */
static const char *build_system_prompt(struct Claude *ctx, char *buf, int bufsize)
{
    int pos = 0;

    buf[0] = '\0';

    if (ctx->memory && ctx->memory->count > 0) {
        pos = memory_format(ctx->memory, buf, bufsize);
    }

    if (ctx->config->system_prompt[0]) {
        snprintf(buf + pos, bufsize - pos, "%s", ctx->config->system_prompt);
        pos += strlen(buf + pos);
    }

    /* Add Amiga context hint for the agent */
    if (ctx->tools && pos < bufsize - 256) {
        const char *hint =
            "\n\nYou are running on an Amiga computer with AmigaOS 3.x. "
            "You have tools to execute AmigaDOS commands, send ARexx commands "
            "to running applications, and read/write files. "
            "Use AmigaDOS paths (SYS:, WORK:, RAM:, S:, AmigaAI:, etc). "
            "AmigaAI: is an assign pointing to the application directory. "
            "When using identify_file: always set max_results when the user "
            "asks for a specific number of files (e.g. 'show 10 images' "
            "-> max_results=10). Always set filter when the user asks for "
            "a specific file type (e.g. 'images' -> filter='picture').";
        snprintf(buf + pos, bufsize - pos, "%s", hint);
        pos += strlen(buf + pos);
    }

    return pos > 0 ? buf : NULL;
}

/* Perform a single API call and return the raw response body.
 * Caller must free the returned body string. */
static char *api_call(struct Claude *ctx, char **error_msg)
{
    char *request_json;
    struct HttpResponse response;
    char api_key_header[256];
    int rc;

    static char effective_system[CONFIG_MAX_PROMPT_LEN + MEMORY_MAX_SIZE + 512];
    const char *sys_ptr;

    const char *headers[] = {
        "Content-Type: application/json",
        api_key_header,
        "anthropic-version: " CLAUDE_API_VERSION,
        NULL
    };

    /* Build x-api-key header */
    snprintf(api_key_header, sizeof(api_key_header),
             "x-api-key: %s", ctx->config->api_key);

    /* Build system prompt */
    sys_ptr = build_system_prompt(ctx, effective_system, sizeof(effective_system));

    /* Build request JSON with tools */
    request_json = json_build_request(
        ctx->config->model,
        ctx->config->max_tokens,
        sys_ptr,
        ctx->messages,
        ctx->tools
    );
    if (!request_json) {
        if (error_msg) *error_msg = strdup("Failed to build request JSON");
        return NULL;
    }

    /* Perform HTTPS POST */
    rc = http_post(CLAUDE_API_HOST, CLAUDE_API_PATH,
                   headers, request_json, &response);

    cJSON_free(request_json);

    if (rc != 0) {
        if (error_msg) *error_msg = strdup("HTTPS request failed");
        return NULL;
    }

    /* Check HTTP status */
    if (response.status_code != 200) {
        char buf[256];
        char *api_err = NULL;

        if (response.body)
            json_parse_response(response.body, &api_err);

        snprintf(buf, sizeof(buf), "HTTP %d: %s",
                 response.status_code,
                 api_err ? api_err : "Request failed");

        if (error_msg) *error_msg = strdup(buf);
        free(api_err);
        free(response.body);
        return NULL;
    }

    /* Parse token usage */
    json_parse_usage(response.body,
                     &ctx->last_input_tokens,
                     &ctx->last_output_tokens);

    return response.body;  /* caller frees */
}

char *claude_send(struct Claude *ctx, const char *user_message, char **error_msg)
{
    cJSON *user_msg = NULL;
    int iteration;
    char *final_text = NULL;
    int final_text_len = 0;
    int final_text_cap = 0;
    int initial_msg_count;

    if (error_msg) *error_msg = NULL;

    /* Check API key */
    if (!ctx->config->api_key[0]) {
        if (error_msg) *error_msg = strdup("No API key configured");
        return NULL;
    }

    /* Remember message count so we can roll back on failure */
    initial_msg_count = cJSON_GetArraySize(ctx->messages);

    /* Add user message to conversation */
    user_msg = json_make_message("user", user_message);
    if (!user_msg) {
        if (error_msg) *error_msg = strdup("Out of memory");
        return NULL;
    }
    cJSON_AddItemToArray(ctx->messages, user_msg);

    /* Tool use loop */
    for (iteration = 0; iteration < TOOLS_MAX_ITERATIONS; iteration++) {
        char *body;
        char *stop_reason = NULL;
        char *text = NULL;
        char *err = NULL;
        cJSON *content;
        int has_tool_use = 0;

        /* API call */
        body = api_call(ctx, &err);
        if (!body) {
            if (error_msg) *error_msg = err;
            goto fail;
        }

        /* Parse full response */
        content = json_parse_full_response(body, &stop_reason, &text, &err);
        free(body);

        if (!content) {
            if (error_msg) *error_msg = err;
            goto fail;
        }

        /* Accumulate any text from this response */
        if (text && text[0]) {
            int tlen = strlen(text);
            int needed = final_text_len + tlen + 2;
            if (needed > final_text_cap) {
                final_text_cap = needed + 256;
                final_text = realloc(final_text, final_text_cap);
            }
            if (final_text) {
                if (final_text_len > 0)
                    final_text[final_text_len++] = '\n';
                memcpy(final_text + final_text_len, text, tlen);
                final_text_len += tlen;
                final_text[final_text_len] = '\0';
            }
        }
        free(text);

        /* Add assistant response to conversation history */
        {
            cJSON *asst_msg = json_make_content_message("assistant",
                                  cJSON_Duplicate(content, 1));
            if (asst_msg)
                cJSON_AddItemToArray(ctx->messages, asst_msg);
        }

        /* Check if we need to execute tools */
        if (stop_reason && strcmp(stop_reason, "tool_use") == 0) {
            cJSON *tool_results = cJSON_CreateArray();
            int i, count = cJSON_GetArraySize(content);

            for (i = 0; i < count; i++) {
                cJSON *block = cJSON_GetArrayItem(content, i);
                cJSON *type = cJSON_GetObjectItemCaseSensitive(block, "type");

                if (type && cJSON_IsString(type) &&
                    strcmp(type->valuestring, "tool_use") == 0)
                {
                    cJSON *id_obj   = cJSON_GetObjectItemCaseSensitive(block, "id");
                    cJSON *name_obj = cJSON_GetObjectItemCaseSensitive(block, "name");
                    cJSON *inp_obj  = cJSON_GetObjectItemCaseSensitive(block, "input");

                    if (id_obj && name_obj && inp_obj &&
                        cJSON_IsString(id_obj) && cJSON_IsString(name_obj))
                    {
                        const char *tool_id   = id_obj->valuestring;
                        const char *tool_name = name_obj->valuestring;
                        int is_error = 0;
                        char *result;

                        /* Build a summary of the tool input */
                        {
                            char *inp_summary = cJSON_PrintUnformatted(inp_obj);

                            /* Notify callback with input detail */
                            if (ctx->tool_cb)
                                ctx->tool_cb(tool_name, "executing",
                                             inp_summary, ctx->tool_cb_data);

                            printf("  [agent] tool_use: %s (id=%s)\n",
                                   tool_name, tool_id);

                            /* Execute the tool */
                            result = tool_execute(tool_name, inp_obj, &is_error);

                            printf("  [agent] result: %s%s\n",
                                   is_error ? "ERROR: " : "",
                                   result ? result : "(null)");

                            /* Notify callback with result */
                            if (ctx->tool_cb)
                                ctx->tool_cb(tool_name,
                                             is_error ? "error" : "done",
                                             result, ctx->tool_cb_data);

                            cJSON_free(inp_summary);
                        }

                        /* Build tool_result block */
                        {
                            cJSON *tr = json_make_tool_result(
                                tool_id, result, is_error);
                            if (tr)
                                cJSON_AddItemToArray(tool_results, tr);
                        }

                        free(result);
                        has_tool_use = 1;
                    }
                }
            }

            cJSON_Delete(content);
            free(stop_reason);

            if (has_tool_use && cJSON_GetArraySize(tool_results) > 0) {
                /* Add tool results as a user message */
                cJSON *tr_msg = json_make_content_message("user", tool_results);
                if (tr_msg)
                    cJSON_AddItemToArray(ctx->messages, tr_msg);
                else
                    cJSON_Delete(tool_results);

                /* Continue the loop for next API call */
                continue;
            }

            cJSON_Delete(tool_results);
            break;  /* No tool use blocks found despite stop_reason */
        }

        /* Not a tool_use response — we're done */
        cJSON_Delete(content);
        free(stop_reason);
        break;
    }

    /* Return accumulated text */
    if (final_text && final_text[0])
        return final_text;

    /* No text at all? */
    free(final_text);
    if (error_msg && !*error_msg)
        *error_msg = strdup("No text in response");
    return NULL;

fail:
    free(final_text);

    /* Remove ALL messages added during this call (user msg, assistant msgs,
     * tool results) so the conversation history stays consistent.
     * This is critical for abort: without it, tool_use blocks remain
     * without matching tool_result blocks, breaking all future API calls. */
    {
        int len = cJSON_GetArraySize(ctx->messages);
        while (len > initial_msg_count) {
            cJSON *item = cJSON_DetachItemFromArray(ctx->messages, len - 1);
            if (item)
                cJSON_Delete(item);
            len--;
        }
    }

    return NULL;
}
