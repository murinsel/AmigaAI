#include "json_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Convert ISO-8859-1 (AmigaOS) to UTF-8 (API).
 * Caller must free() the result. */
static char *iso8859_to_utf8(const char *src)
{
    const unsigned char *s = (const unsigned char *)src;
    int len = 0, i;
    char *dst, *d;

    /* Calculate output length */
    for (i = 0; s[i]; i++) {
        if (s[i] < 0x80)
            len++;
        else
            len += 2;
    }

    dst = malloc(len + 1);
    if (!dst) return NULL;
    d = dst;

    for (i = 0; s[i]; i++) {
        if (s[i] < 0x80) {
            *d++ = s[i];
        } else {
            *d++ = 0xC0 | (s[i] >> 6);
            *d++ = 0x80 | (s[i] & 0x3F);
        }
    }
    *d = '\0';
    return dst;
}

/* Convert UTF-8 (API response) to ISO-8859-1 (AmigaOS).
 * Characters outside ISO-8859-1 are replaced with '?'.
 * Caller must free() the result. */
static char *utf8_to_iso8859(const char *src)
{
    const unsigned char *s = (const unsigned char *)src;
    int len = strlen(src);
    char *dst, *d;
    int i;

    dst = malloc(len + 1);
    if (!dst) return NULL;
    d = dst;

    for (i = 0; s[i]; ) {
        if (s[i] < 0x80) {
            *d++ = s[i++];
        } else if ((s[i] & 0xE0) == 0xC0 && s[i+1]) {
            /* 2-byte UTF-8: U+0080..U+07FF */
            unsigned int cp = ((s[i] & 0x1F) << 6) | (s[i+1] & 0x3F);
            *d++ = (cp <= 0xFF) ? (char)cp : '?';
            i += 2;
        } else if ((s[i] & 0xF0) == 0xE0 && s[i+1] && s[i+2]) {
            *d++ = '?'; /* 3-byte: outside ISO-8859-1 */
            i += 3;
        } else if ((s[i] & 0xF8) == 0xF0 && s[i+1] && s[i+2] && s[i+3]) {
            *d++ = '?'; /* 4-byte: outside ISO-8859-1 */
            i += 4;
        } else {
            *d++ = '?'; /* invalid */
            i++;
        }
    }
    *d = '\0';
    return dst;
}

char *json_build_request(const char *model,
                         int max_tokens,
                         const char *system,
                         cJSON *messages_array,
                         cJSON *tools)
{
    cJSON *root;
    char  *json_str;

    root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON_AddStringToObject(root, "model", model);
    cJSON_AddNumberToObject(root, "max_tokens", max_tokens);

    if (system && system[0]) {
        char *sys_utf8 = iso8859_to_utf8(system);
        cJSON_AddStringToObject(root, "system", sys_utf8 ? sys_utf8 : system);
        free(sys_utf8);
    }

    /* Add tools array if provided */
    if (tools && cJSON_GetArraySize(tools) > 0) {
        cJSON *dup = cJSON_Duplicate(tools, 1);
        if (dup)
            cJSON_AddItemToObject(root, "tools", dup);
    }

    /* Add a duplicate of the messages array so the caller keeps ownership */
    {
        cJSON *dup = cJSON_Duplicate(messages_array, 1);
        if (!dup) { cJSON_Delete(root); return NULL; }
        cJSON_AddItemToObject(root, "messages", dup);
    }

    json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_str; /* Caller must free() / cJSON_free() */
}

cJSON *json_make_message(const char *role, const char *content)
{
    cJSON *msg;
    char *utf8;

    msg = cJSON_CreateObject();
    if (!msg) return NULL;

    cJSON_AddStringToObject(msg, "role", role);

    /* Convert ISO-8859-1 content to UTF-8 for the API */
    utf8 = iso8859_to_utf8(content);
    cJSON_AddStringToObject(msg, "content", utf8 ? utf8 : content);
    free(utf8);

    return msg;
}

cJSON *json_make_content_message(const char *role, cJSON *content_array)
{
    cJSON *msg;

    msg = cJSON_CreateObject();
    if (!msg) return NULL;

    cJSON_AddStringToObject(msg, "role", role);
    cJSON_AddItemToObject(msg, "content", content_array);

    return msg;
}

cJSON *json_make_tool_result(const char *tool_use_id,
                             const char *result,
                             int is_error)
{
    cJSON *block = cJSON_CreateObject();
    if (!block) return NULL;

    cJSON_AddStringToObject(block, "type", "tool_result");
    cJSON_AddStringToObject(block, "tool_use_id", tool_use_id);
    /* Convert tool result from ISO-8859-1 to UTF-8 */
    {
        char *utf8 = result ? iso8859_to_utf8(result) : NULL;
        cJSON_AddStringToObject(block, "content", utf8 ? utf8 : (result ? result : ""));
        free(utf8);
    }

    if (is_error)
        cJSON_AddTrueToObject(block, "is_error");

    return block;
}

char *json_parse_response(const char *json_str, char **error_msg)
{
    cJSON *root, *err_obj, *content, *item, *text_obj;
    const char *text;
    char *result;

    if (error_msg) *error_msg = NULL;

    root = cJSON_Parse(json_str);
    if (!root) {
        if (error_msg) *error_msg = strdup("Failed to parse JSON response");
        return NULL;
    }

    /* Check for error response */
    err_obj = cJSON_GetObjectItemCaseSensitive(root, "error");
    if (err_obj) {
        cJSON *msg_obj = cJSON_GetObjectItemCaseSensitive(err_obj, "message");
        if (msg_obj && cJSON_IsString(msg_obj)) {
            if (error_msg)
                *error_msg = strdup(msg_obj->valuestring);
        } else {
            if (error_msg)
                *error_msg = strdup("Unknown API error");
        }
        cJSON_Delete(root);
        return NULL;
    }

    /* Extract first text block from content array */
    content = cJSON_GetObjectItemCaseSensitive(root, "content");
    if (!content || !cJSON_IsArray(content) || cJSON_GetArraySize(content) == 0) {
        if (error_msg) *error_msg = strdup("No content in response");
        cJSON_Delete(root);
        return NULL;
    }

    /* Find first text block */
    result = NULL;
    {
        int i, count = cJSON_GetArraySize(content);
        for (i = 0; i < count; i++) {
            item = cJSON_GetArrayItem(content, i);
            cJSON *type = cJSON_GetObjectItemCaseSensitive(item, "type");
            if (type && cJSON_IsString(type) &&
                strcmp(type->valuestring, "text") == 0)
            {
                text_obj = cJSON_GetObjectItemCaseSensitive(item, "text");
                if (text_obj && cJSON_IsString(text_obj)) {
                    text = text_obj->valuestring;
                    result = strdup(text ? text : "");
                    break;
                }
            }
        }
    }

    if (!result) {
        if (error_msg) *error_msg = strdup("No text in content blocks");
        cJSON_Delete(root);
        return NULL;
    }

    /* Convert UTF-8 response to ISO-8859-1 for AmigaOS display */
    {
        char *iso = utf8_to_iso8859(result);
        if (iso) { free(result); result = iso; }
    }

    cJSON_Delete(root);
    return result;
}

cJSON *json_parse_full_response(const char *json_str,
                                char **stop_reason,
                                char **text_out,
                                char **error_msg)
{
    cJSON *root, *err_obj, *content, *sr_obj;
    cJSON *result_content = NULL;

    if (stop_reason) *stop_reason = NULL;
    if (text_out) *text_out = NULL;
    if (error_msg) *error_msg = NULL;

    root = cJSON_Parse(json_str);
    if (!root) {
        if (error_msg) *error_msg = strdup("Failed to parse JSON response");
        return NULL;
    }

    /* Check for error response */
    err_obj = cJSON_GetObjectItemCaseSensitive(root, "error");
    if (err_obj) {
        cJSON *msg_obj = cJSON_GetObjectItemCaseSensitive(err_obj, "message");
        if (error_msg) {
            if (msg_obj && cJSON_IsString(msg_obj))
                *error_msg = strdup(msg_obj->valuestring);
            else
                *error_msg = strdup("Unknown API error");
        }
        cJSON_Delete(root);
        return NULL;
    }

    /* Extract stop_reason */
    sr_obj = cJSON_GetObjectItemCaseSensitive(root, "stop_reason");
    if (sr_obj && cJSON_IsString(sr_obj) && stop_reason)
        *stop_reason = strdup(sr_obj->valuestring);

    /* Extract content array */
    content = cJSON_GetObjectItemCaseSensitive(root, "content");
    if (!content || !cJSON_IsArray(content)) {
        if (error_msg) *error_msg = strdup("No content in response");
        cJSON_Delete(root);
        return NULL;
    }

    /* Extract all text blocks concatenated */
    if (text_out) {
        int i, count = cJSON_GetArraySize(content);
        char *buf = NULL;
        int buf_len = 0, buf_cap = 0;

        for (i = 0; i < count; i++) {
            cJSON *item = cJSON_GetArrayItem(content, i);
            cJSON *type = cJSON_GetObjectItemCaseSensitive(item, "type");
            if (type && cJSON_IsString(type) &&
                strcmp(type->valuestring, "text") == 0)
            {
                cJSON *text_obj = cJSON_GetObjectItemCaseSensitive(item, "text");
                if (text_obj && cJSON_IsString(text_obj)) {
                    int tlen = strlen(text_obj->valuestring);
                    int needed = buf_len + tlen + 2;
                    if (needed > buf_cap) {
                        buf_cap = needed + 256;
                        buf = realloc(buf, buf_cap);
                    }
                    if (buf) {
                        if (buf_len > 0) buf[buf_len++] = '\n';
                        memcpy(buf + buf_len, text_obj->valuestring, tlen);
                        buf_len += tlen;
                        buf[buf_len] = '\0';
                    }
                }
            }
        }
        /* Convert UTF-8 response to ISO-8859-1 for AmigaOS display */
        if (buf) {
            char *iso = utf8_to_iso8859(buf);
            if (iso) { free(buf); buf = iso; }
        }
        *text_out = buf;
    }

    /* Duplicate content array so caller owns it */
    result_content = cJSON_Duplicate(content, 1);

    cJSON_Delete(root);
    return result_content;
}

int json_parse_usage(const char *json_str, int *input_tokens, int *output_tokens)
{
    cJSON *root, *usage, *val;

    root = cJSON_Parse(json_str);
    if (!root) return -1;

    usage = cJSON_GetObjectItemCaseSensitive(root, "usage");
    if (!usage) {
        cJSON_Delete(root);
        return -1;
    }

    if (input_tokens) {
        val = cJSON_GetObjectItemCaseSensitive(usage, "input_tokens");
        if (val && cJSON_IsNumber(val))
            *input_tokens = (int)val->valuedouble;
    }

    if (output_tokens) {
        val = cJSON_GetObjectItemCaseSensitive(usage, "output_tokens");
        if (val && cJSON_IsNumber(val))
            *output_tokens = (int)val->valuedouble;
    }

    cJSON_Delete(root);
    return 0;
}
