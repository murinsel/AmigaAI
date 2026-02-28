#ifndef AMIGAAI_JSON_UTILS_H
#define AMIGAAI_JSON_UTILS_H

#include "cJSON.h"

/* Build the JSON request body for the Claude Messages API.
 * messages_array is a cJSON array containing the conversation.
 * system may be NULL. tools may be NULL (no tool use).
 * Returns a newly allocated JSON string (caller must free). */
char *json_build_request(const char *model,
                         int max_tokens,
                         const char *system,
                         cJSON *messages_array,
                         cJSON *tools);

/* Parse a Claude API response and extract the assistant's text reply.
 * Returns a newly allocated string (caller must free()) or NULL on error.
 * If error_msg is not NULL, it will be set to an error description on failure. */
char *json_parse_response(const char *json_str, char **error_msg);

/* Parse full response: return the content array and stop_reason.
 * Returns a duplicated cJSON content array (caller must cJSON_Delete).
 * Sets *stop_reason to "end_turn", "tool_use", etc. (caller must free).
 * Also extracts any text blocks into *text_out (caller must free, may be NULL). */
cJSON *json_parse_full_response(const char *json_str,
                                char **stop_reason,
                                char **text_out,
                                char **error_msg);

/* Create a message object {"role":"...", "content":"..."} */
cJSON *json_make_message(const char *role, const char *content);

/* Create a message with a content array: {"role":"...", "content":[...]}
 * content_array is consumed (added to message, not duplicated). */
cJSON *json_make_content_message(const char *role, cJSON *content_array);

/* Build a tool_result content block.
 * Returns: {"type":"tool_result", "tool_use_id":"...", "content":"..."} */
cJSON *json_make_tool_result(const char *tool_use_id,
                             const char *result,
                             int is_error);

/* Parse usage info from response. Returns 0 on success. */
int json_parse_usage(const char *json_str, int *input_tokens, int *output_tokens);

#endif /* AMIGAAI_JSON_UTILS_H */
