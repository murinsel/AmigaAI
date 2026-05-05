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
char *json_utf8_to_iso8859(const char *src)
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

/* ===== OpenAI Chat Completions format conversion =====
 *
 * The agent loop and stored conversation history use Anthropic-internal
 * format (content blocks with type tags). When sending to an OpenAI-
 * compatible endpoint we convert at the JSON boundary so the rest of
 * the codebase doesn't need to know about the OpenAI format.
 *
 * Likewise, OpenAI responses are converted back to Anthropic-style
 * content blocks (tool_calls -> tool_use blocks, choices[0].message
 * -> top-level content array).
 */

/* Convert an Anthropic tools array to OpenAI functions array.
 * Returns a new cJSON array (caller owns), or NULL on alloc failure. */
static cJSON *convert_tools_to_openai(cJSON *anth_tools)
{
    cJSON *openai_tools, *t, *fn, *out, *schema_dup;
    int i, n;

    if (!anth_tools) return NULL;

    openai_tools = cJSON_CreateArray();
    if (!openai_tools) return NULL;

    n = cJSON_GetArraySize(anth_tools);
    for (i = 0; i < n; i++) {
        cJSON *src = cJSON_GetArrayItem(anth_tools, i);
        cJSON *name_obj = cJSON_GetObjectItemCaseSensitive(src, "name");
        cJSON *desc_obj = cJSON_GetObjectItemCaseSensitive(src, "description");
        cJSON *schema_obj = cJSON_GetObjectItemCaseSensitive(src, "input_schema");

        out = cJSON_CreateObject();
        if (!out) continue;
        cJSON_AddStringToObject(out, "type", "function");

        fn = cJSON_CreateObject();
        if (!fn) { cJSON_Delete(out); continue; }

        if (name_obj && cJSON_IsString(name_obj))
            cJSON_AddStringToObject(fn, "name", name_obj->valuestring);
        if (desc_obj && cJSON_IsString(desc_obj))
            cJSON_AddStringToObject(fn, "description", desc_obj->valuestring);
        if (schema_obj) {
            schema_dup = cJSON_Duplicate(schema_obj, 1);
            if (schema_dup)
                cJSON_AddItemToObject(fn, "parameters", schema_dup);
        }

        cJSON_AddItemToObject(out, "function", fn);
        cJSON_AddItemToArray(openai_tools, out);
    }

    return openai_tools;
}

/* Build a "data:image/png;base64,..." URI for OpenAI image_url.
 * Caller must cJSON_free the returned string (or use within a cJSON tree). */
static char *make_data_uri(const char *media_type, const char *base64_data)
{
    int len = strlen(media_type) + strlen(base64_data) + 32;
    char *uri = malloc(len);
    if (!uri) return NULL;
    snprintf(uri, len, "data:%s;base64,%s", media_type, base64_data);
    return uri;
}

/* Convert one Anthropic content block (within a user/assistant message)
 * to OpenAI form. Appends the converted block(s) to out_content (an array),
 * and may also produce extra messages (e.g. tool result image splits)
 * which are appended to extra_messages.
 *
 * Returns 0 on success, -1 on alloc failure. */
static int convert_content_block(cJSON *block, const char *role,
                                  cJSON *out_content,
                                  cJSON *extra_messages,
                                  cJSON *out_tool_calls)
{
    cJSON *type_obj;
    const char *type;

    type_obj = cJSON_GetObjectItemCaseSensitive(block, "type");
    if (!type_obj || !cJSON_IsString(type_obj)) return 0;
    type = type_obj->valuestring;

    if (strcmp(type, "text") == 0) {
        cJSON *txt = cJSON_GetObjectItemCaseSensitive(block, "text");
        cJSON *out;
        if (!txt || !cJSON_IsString(txt)) return 0;
        out = cJSON_CreateObject();
        if (!out) return -1;
        cJSON_AddStringToObject(out, "type", "text");
        cJSON_AddStringToObject(out, "text", txt->valuestring);
        cJSON_AddItemToArray(out_content, out);
        return 0;
    }

    if (strcmp(type, "image") == 0) {
        cJSON *src = cJSON_GetObjectItemCaseSensitive(block, "source");
        cJSON *media_obj, *data_obj, *out, *img_url;
        char *uri;

        if (!src) return 0;
        media_obj = cJSON_GetObjectItemCaseSensitive(src, "media_type");
        data_obj  = cJSON_GetObjectItemCaseSensitive(src, "data");
        if (!media_obj || !cJSON_IsString(media_obj) ||
            !data_obj || !cJSON_IsString(data_obj))
            return 0;

        uri = make_data_uri(media_obj->valuestring, data_obj->valuestring);
        if (!uri) return -1;

        out = cJSON_CreateObject();
        img_url = cJSON_CreateObject();
        if (!out || !img_url) {
            free(uri);
            if (out) cJSON_Delete(out);
            if (img_url) cJSON_Delete(img_url);
            return -1;
        }
        cJSON_AddStringToObject(out, "type", "image_url");
        cJSON_AddStringToObject(img_url, "url", uri);
        cJSON_AddItemToObject(out, "image_url", img_url);
        cJSON_AddItemToArray(out_content, out);
        free(uri);
        return 0;
    }

    if (strcmp(type, "tool_use") == 0 && out_tool_calls) {
        /* Anthropic tool_use -> OpenAI tool_calls entry */
        cJSON *id_obj   = cJSON_GetObjectItemCaseSensitive(block, "id");
        cJSON *name_obj = cJSON_GetObjectItemCaseSensitive(block, "name");
        cJSON *inp_obj  = cJSON_GetObjectItemCaseSensitive(block, "input");
        cJSON *call, *fn;
        char *args_json;

        if (!id_obj || !name_obj) return 0;

        call = cJSON_CreateObject();
        fn = cJSON_CreateObject();
        if (!call || !fn) {
            if (call) cJSON_Delete(call);
            if (fn) cJSON_Delete(fn);
            return -1;
        }

        cJSON_AddStringToObject(call, "id", id_obj->valuestring);
        cJSON_AddStringToObject(call, "type", "function");
        cJSON_AddStringToObject(fn, "name", name_obj->valuestring);

        /* OpenAI expects arguments as a JSON-encoded string */
        if (inp_obj) {
            args_json = cJSON_PrintUnformatted(inp_obj);
            cJSON_AddStringToObject(fn, "arguments", args_json ? args_json : "{}");
            if (args_json) cJSON_free(args_json);
        } else {
            cJSON_AddStringToObject(fn, "arguments", "{}");
        }

        cJSON_AddItemToObject(call, "function", fn);
        cJSON_AddItemToArray(out_tool_calls, call);
        (void)role;
        return 0;
    }

    if (strcmp(type, "tool_result") == 0 && extra_messages) {
        /* Anthropic tool_result block -> separate {role:"tool"} message.
         * If content contains an image, emit a text-only tool message
         * plus a follow-up user message with the image (OpenAI tool
         * messages can't carry images). */
        cJSON *id_obj = cJSON_GetObjectItemCaseSensitive(block, "tool_use_id");
        cJSON *content_obj = cJSON_GetObjectItemCaseSensitive(block, "content");
        cJSON *tool_msg, *image_block = NULL;
        char *text_part = NULL;

        if (!id_obj || !cJSON_IsString(id_obj)) return 0;

        if (content_obj && cJSON_IsString(content_obj)) {
            text_part = strdup(content_obj->valuestring);
        } else if (content_obj && cJSON_IsArray(content_obj)) {
            int j, m = cJSON_GetArraySize(content_obj);
            for (j = 0; j < m; j++) {
                cJSON *sub = cJSON_GetArrayItem(content_obj, j);
                cJSON *sub_type = cJSON_GetObjectItemCaseSensitive(sub, "type");
                if (!sub_type || !cJSON_IsString(sub_type)) continue;
                if (strcmp(sub_type->valuestring, "text") == 0) {
                    cJSON *t = cJSON_GetObjectItemCaseSensitive(sub, "text");
                    if (t && cJSON_IsString(t) && !text_part)
                        text_part = strdup(t->valuestring);
                } else if (strcmp(sub_type->valuestring, "image") == 0) {
                    image_block = sub;  /* remember to split out */
                }
            }
        }

        tool_msg = cJSON_CreateObject();
        if (!tool_msg) { free(text_part); return -1; }
        cJSON_AddStringToObject(tool_msg, "role", "tool");
        cJSON_AddStringToObject(tool_msg, "tool_call_id", id_obj->valuestring);
        cJSON_AddStringToObject(tool_msg, "content",
            image_block ? "Screenshot captured (see next message)"
                        : (text_part ? text_part : ""));
        free(text_part);
        cJSON_AddItemToArray(extra_messages, tool_msg);

        /* If there was an image, append a separate user message with it */
        if (image_block) {
            cJSON *src = cJSON_GetObjectItemCaseSensitive(image_block, "source");
            if (src) {
                cJSON *media = cJSON_GetObjectItemCaseSensitive(src, "media_type");
                cJSON *data  = cJSON_GetObjectItemCaseSensitive(src, "data");
                if (media && cJSON_IsString(media) &&
                    data && cJSON_IsString(data))
                {
                    char *uri = make_data_uri(media->valuestring,
                                              data->valuestring);
                    if (uri) {
                        cJSON *user_msg = cJSON_CreateObject();
                        cJSON *content_arr = cJSON_CreateArray();
                        cJSON *img_block = cJSON_CreateObject();
                        cJSON *img_url = cJSON_CreateObject();
                        if (user_msg && content_arr && img_block && img_url) {
                            cJSON_AddStringToObject(img_block, "type", "image_url");
                            cJSON_AddStringToObject(img_url, "url", uri);
                            cJSON_AddItemToObject(img_block, "image_url", img_url);
                            cJSON_AddItemToArray(content_arr, img_block);
                            cJSON_AddStringToObject(user_msg, "role", "user");
                            cJSON_AddItemToObject(user_msg, "content", content_arr);
                            cJSON_AddItemToArray(extra_messages, user_msg);
                        } else {
                            if (user_msg) cJSON_Delete(user_msg);
                            if (content_arr) cJSON_Delete(content_arr);
                            if (img_block) cJSON_Delete(img_block);
                            if (img_url) cJSON_Delete(img_url);
                        }
                        free(uri);
                    }
                }
            }
        }
        return 0;
    }

    return 0;
}

/* Convert an Anthropic messages array (with system inserted as first
 * "system" message if non-NULL) to OpenAI chat completions format.
 * Returns a new cJSON array (caller owns). */
static cJSON *convert_messages_to_openai(cJSON *anth_messages,
                                          const char *system)
{
    cJSON *out_messages;
    int i, n;

    out_messages = cJSON_CreateArray();
    if (!out_messages) return NULL;

    /* System prompt becomes a system message */
    if (system && system[0]) {
        cJSON *sys_msg = cJSON_CreateObject();
        if (sys_msg) {
            char *utf8 = iso8859_to_utf8(system);
            cJSON_AddStringToObject(sys_msg, "role", "system");
            cJSON_AddStringToObject(sys_msg, "content",
                                    utf8 ? utf8 : system);
            free(utf8);
            cJSON_AddItemToArray(out_messages, sys_msg);
        }
    }

    n = cJSON_GetArraySize(anth_messages);
    for (i = 0; i < n; i++) {
        cJSON *msg = cJSON_GetArrayItem(anth_messages, i);
        cJSON *role_obj = cJSON_GetObjectItemCaseSensitive(msg, "role");
        cJSON *content_obj = cJSON_GetObjectItemCaseSensitive(msg, "content");
        const char *role;

        if (!role_obj || !cJSON_IsString(role_obj)) continue;
        role = role_obj->valuestring;

        /* Plain string content: copy as-is (works for both formats) */
        if (content_obj && cJSON_IsString(content_obj)) {
            cJSON *out = cJSON_CreateObject();
            if (!out) continue;
            cJSON_AddStringToObject(out, "role", role);
            cJSON_AddStringToObject(out, "content", content_obj->valuestring);
            cJSON_AddItemToArray(out_messages, out);
            continue;
        }

        if (!content_obj || !cJSON_IsArray(content_obj)) continue;

        if (strcmp(role, "user") == 0) {
            /* Build user message; tool_result blocks get split off into
             * separate role:tool messages emitted *before* further user
             * blocks if they appear together. */
            cJSON *user_content = cJSON_CreateArray();
            cJSON *extra = cJSON_CreateArray();
            int m, j, has_user_blocks = 0;

            if (!user_content || !extra) {
                if (user_content) cJSON_Delete(user_content);
                if (extra) cJSON_Delete(extra);
                continue;
            }

            m = cJSON_GetArraySize(content_obj);
            for (j = 0; j < m; j++) {
                cJSON *block = cJSON_GetArrayItem(content_obj, j);
                cJSON *btype = cJSON_GetObjectItemCaseSensitive(block, "type");
                if (!btype || !cJSON_IsString(btype)) continue;

                if (strcmp(btype->valuestring, "tool_result") == 0) {
                    convert_content_block(block, role, NULL, extra, NULL);
                } else {
                    convert_content_block(block, role, user_content, NULL, NULL);
                    has_user_blocks = 1;
                }
            }

            /* Emit accumulated tool result messages first */
            {
                int em = cJSON_GetArraySize(extra);
                for (j = 0; j < em; j++) {
                    cJSON *em_msg = cJSON_DetachItemFromArray(extra, 0);
                    if (em_msg) cJSON_AddItemToArray(out_messages, em_msg);
                }
                cJSON_Delete(extra);
            }

            /* Then the actual user message if it had non-tool-result blocks */
            if (has_user_blocks) {
                cJSON *out = cJSON_CreateObject();
                if (out) {
                    cJSON_AddStringToObject(out, "role", "user");
                    cJSON_AddItemToObject(out, "content", user_content);
                    cJSON_AddItemToArray(out_messages, out);
                } else {
                    cJSON_Delete(user_content);
                }
            } else {
                cJSON_Delete(user_content);
            }

        } else if (strcmp(role, "assistant") == 0) {
            /* Assistant: text blocks join into content; tool_use blocks
             * become tool_calls. */
            cJSON *out, *tool_calls;
            char *combined_text = NULL;
            int combined_len = 0, combined_cap = 0;
            int m, j, has_calls;

            tool_calls = cJSON_CreateArray();
            if (!tool_calls) continue;

            m = cJSON_GetArraySize(content_obj);
            for (j = 0; j < m; j++) {
                cJSON *block = cJSON_GetArrayItem(content_obj, j);
                cJSON *btype = cJSON_GetObjectItemCaseSensitive(block, "type");
                if (!btype || !cJSON_IsString(btype)) continue;

                if (strcmp(btype->valuestring, "text") == 0) {
                    cJSON *t = cJSON_GetObjectItemCaseSensitive(block, "text");
                    if (t && cJSON_IsString(t)) {
                        int tlen = strlen(t->valuestring);
                        int needed = combined_len + tlen + 2;
                        if (needed > combined_cap) {
                            combined_cap = needed + 256;
                            combined_text = realloc(combined_text, combined_cap);
                        }
                        if (combined_text) {
                            if (combined_len > 0)
                                combined_text[combined_len++] = '\n';
                            memcpy(combined_text + combined_len,
                                   t->valuestring, tlen);
                            combined_len += tlen;
                            combined_text[combined_len] = '\0';
                        }
                    }
                } else if (strcmp(btype->valuestring, "tool_use") == 0) {
                    convert_content_block(block, role, NULL, NULL, tool_calls);
                }
            }

            has_calls = (cJSON_GetArraySize(tool_calls) > 0);

            out = cJSON_CreateObject();
            if (out) {
                cJSON_AddStringToObject(out, "role", "assistant");
                if (combined_text) {
                    cJSON_AddStringToObject(out, "content", combined_text);
                } else {
                    /* OpenAI requires content field; null is acceptable */
                    cJSON_AddNullToObject(out, "content");
                }
                if (has_calls) {
                    cJSON_AddItemToObject(out, "tool_calls", tool_calls);
                } else {
                    cJSON_Delete(tool_calls);
                }
                cJSON_AddItemToArray(out_messages, out);
            } else {
                cJSON_Delete(tool_calls);
            }
            free(combined_text);
        }
    }

    return out_messages;
}

/* Build OpenAI Chat Completions request body.
 * Returns malloc'd JSON string (caller frees via cJSON_free). */
static char *build_request_openai(const char *model,
                                   int max_tokens,
                                   const char *system,
                                   cJSON *messages_array,
                                   cJSON *tools)
{
    cJSON *root, *msgs, *tool_array;
    char *json_str;

    root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON_AddStringToObject(root, "model", model);
    cJSON_AddNumberToObject(root, "max_tokens", max_tokens);
    cJSON_AddFalseToObject(root, "stream");

    msgs = convert_messages_to_openai(messages_array, system);
    if (!msgs) { cJSON_Delete(root); return NULL; }
    cJSON_AddItemToObject(root, "messages", msgs);

    if (tools && cJSON_GetArraySize(tools) > 0) {
        tool_array = convert_tools_to_openai(tools);
        if (tool_array)
            cJSON_AddItemToObject(root, "tools", tool_array);
    }

    json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}

char *json_build_request(const char *provider,
                         const char *model,
                         int max_tokens,
                         const char *system,
                         cJSON *messages_array,
                         cJSON *tools)
{
    cJSON *root;
    char  *json_str;

    /* OpenAI Chat Completions branch — different schema entirely */
    if (provider && strcmp(provider, "openai") == 0) {
        return build_request_openai(model, max_tokens, system,
                                    messages_array, tools);
    }

    /* Anthropic Messages API (default) */
    root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON_AddStringToObject(root, "model", model);
    cJSON_AddNumberToObject(root, "max_tokens", max_tokens);
    cJSON_AddFalseToObject(root, "stream");

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

cJSON *json_make_tool_result_with_image(const char *tool_use_id,
                                         const char *image_base64,
                                         const char *media_type,
                                         const char *alt_text)
{
    cJSON *block, *content, *img, *source, *text;

    block = cJSON_CreateObject();
    if (!block) return NULL;

    cJSON_AddStringToObject(block, "type", "tool_result");
    cJSON_AddStringToObject(block, "tool_use_id", tool_use_id);

    content = cJSON_CreateArray();
    if (!content) { cJSON_Delete(block); return NULL; }

    /* Image block */
    img = cJSON_CreateObject();
    if (img) {
        cJSON_AddStringToObject(img, "type", "image");
        source = cJSON_CreateObject();
        if (source) {
            cJSON_AddStringToObject(source, "type", "base64");
            cJSON_AddStringToObject(source, "media_type",
                                    media_type ? media_type : "image/png");
            cJSON_AddStringToObject(source, "data", image_base64);
            cJSON_AddItemToObject(img, "source", source);
        }
        cJSON_AddItemToArray(content, img);
    }

    /* Text block */
    if (alt_text) {
        text = cJSON_CreateObject();
        if (text) {
            cJSON_AddStringToObject(text, "type", "text");
            cJSON_AddStringToObject(text, "text", alt_text);
            cJSON_AddItemToArray(content, text);
        }
    }

    cJSON_AddItemToObject(block, "content", content);

    return block;
}

cJSON *json_make_user_image_message(const char *image_base64,
                                     const char *media_type,
                                     const char *text)
{
    cJSON *msg, *content, *img, *source, *txt;

    msg = cJSON_CreateObject();
    if (!msg) return NULL;

    cJSON_AddStringToObject(msg, "role", "user");

    content = cJSON_CreateArray();
    if (!content) { cJSON_Delete(msg); return NULL; }

    /* Image block */
    img = cJSON_CreateObject();
    if (img) {
        cJSON_AddStringToObject(img, "type", "image");
        source = cJSON_CreateObject();
        if (source) {
            cJSON_AddStringToObject(source, "type", "base64");
            cJSON_AddStringToObject(source, "media_type",
                                    media_type ? media_type : "image/png");
            cJSON_AddStringToObject(source, "data", image_base64);
            cJSON_AddItemToObject(img, "source", source);
        }
        cJSON_AddItemToArray(content, img);
    }

    /* Text block */
    if (text) {
        char *utf8 = iso8859_to_utf8(text);
        txt = cJSON_CreateObject();
        if (txt) {
            cJSON_AddStringToObject(txt, "type", "text");
            cJSON_AddStringToObject(txt, "text", utf8 ? utf8 : text);
            cJSON_AddItemToArray(content, txt);
        }
        free(utf8);
    }

    cJSON_AddItemToObject(msg, "content", content);
    return msg;
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
        char *iso = json_utf8_to_iso8859(result);
        if (iso) { free(result); result = iso; }
    }

    cJSON_Delete(root);
    return result;
}

/* Parse an OpenAI Chat Completions response and synthesize an Anthropic-
 * style content array (so the agent loop in claude.c works unchanged).
 * Sets *stop_reason and *text_out like the Anthropic path. */
static cJSON *parse_full_response_openai(cJSON *root,
                                          char **stop_reason,
                                          char **text_out,
                                          char **error_msg)
{
    cJSON *choices, *first, *message, *finish_obj;
    cJSON *content_obj, *tool_calls;
    cJSON *result_content;
    char *combined_text = NULL;
    int combined_len = 0, combined_cap = 0;

    choices = cJSON_GetObjectItemCaseSensitive(root, "choices");
    if (!choices || !cJSON_IsArray(choices) ||
        cJSON_GetArraySize(choices) == 0) {
        if (error_msg) *error_msg = strdup("No choices in response");
        return NULL;
    }
    first = cJSON_GetArrayItem(choices, 0);
    message = cJSON_GetObjectItemCaseSensitive(first, "message");
    if (!message) {
        if (error_msg) *error_msg = strdup("No message in choice");
        return NULL;
    }

    /* Map finish_reason to Anthropic-style stop_reason */
    finish_obj = cJSON_GetObjectItemCaseSensitive(first, "finish_reason");
    if (finish_obj && cJSON_IsString(finish_obj) && stop_reason) {
        const char *fr = finish_obj->valuestring;
        if (strcmp(fr, "tool_calls") == 0)
            *stop_reason = strdup("tool_use");
        else if (strcmp(fr, "length") == 0)
            *stop_reason = strdup("max_tokens");
        else if (strcmp(fr, "stop") == 0)
            *stop_reason = strdup("end_turn");
        else
            *stop_reason = strdup(fr);
    }

    result_content = cJSON_CreateArray();
    if (!result_content) return NULL;

    /* Text content */
    content_obj = cJSON_GetObjectItemCaseSensitive(message, "content");
    if (content_obj && cJSON_IsString(content_obj) && content_obj->valuestring[0]) {
        cJSON *text_block = cJSON_CreateObject();
        if (text_block) {
            cJSON_AddStringToObject(text_block, "type", "text");
            cJSON_AddStringToObject(text_block, "text",
                                    content_obj->valuestring);
            cJSON_AddItemToArray(result_content, text_block);
        }
        if (text_out) {
            int tlen = strlen(content_obj->valuestring);
            combined_text = malloc(tlen + 1);
            if (combined_text) {
                memcpy(combined_text, content_obj->valuestring, tlen);
                combined_text[tlen] = '\0';
                combined_len = tlen;
                combined_cap = tlen + 1;
            }
        }
    }

    /* Tool calls -> tool_use blocks */
    tool_calls = cJSON_GetObjectItemCaseSensitive(message, "tool_calls");
    if (tool_calls && cJSON_IsArray(tool_calls)) {
        int i, n = cJSON_GetArraySize(tool_calls);
        for (i = 0; i < n; i++) {
            cJSON *call = cJSON_GetArrayItem(tool_calls, i);
            cJSON *id_obj = cJSON_GetObjectItemCaseSensitive(call, "id");
            cJSON *fn = cJSON_GetObjectItemCaseSensitive(call, "function");
            cJSON *name_obj, *args_obj, *block, *input_obj;

            if (!fn) continue;
            name_obj = cJSON_GetObjectItemCaseSensitive(fn, "name");
            args_obj = cJSON_GetObjectItemCaseSensitive(fn, "arguments");

            block = cJSON_CreateObject();
            if (!block) continue;
            cJSON_AddStringToObject(block, "type", "tool_use");
            if (id_obj && cJSON_IsString(id_obj))
                cJSON_AddStringToObject(block, "id", id_obj->valuestring);
            else
                cJSON_AddStringToObject(block, "id", "");
            if (name_obj && cJSON_IsString(name_obj))
                cJSON_AddStringToObject(block, "name", name_obj->valuestring);
            else
                cJSON_AddStringToObject(block, "name", "");

            /* arguments comes as a JSON-encoded string — parse it */
            input_obj = NULL;
            if (args_obj && cJSON_IsString(args_obj) && args_obj->valuestring) {
                input_obj = cJSON_Parse(args_obj->valuestring);
            }
            if (!input_obj) input_obj = cJSON_CreateObject();
            cJSON_AddItemToObject(block, "input", input_obj);

            cJSON_AddItemToArray(result_content, block);
        }
    }

    /* If the model finished with tool_calls but produced no tool_calls
     * array (some OpenRouter models drop tool support silently), don't
     * pretend it was a tool use turn. */
    if (stop_reason && *stop_reason &&
        strcmp(*stop_reason, "tool_use") == 0 &&
        (!tool_calls || cJSON_GetArraySize(tool_calls) == 0))
    {
        free(*stop_reason);
        *stop_reason = strdup("end_turn");
    }

    if (text_out) {
        if (combined_text) {
            char *iso = json_utf8_to_iso8859(combined_text);
            if (iso) { free(combined_text); combined_text = iso; }
        }
        *text_out = combined_text;
    } else {
        free(combined_text);
    }

    (void)combined_len;
    (void)combined_cap;
    return result_content;
}

cJSON *json_parse_full_response(const char *provider,
                                const char *json_str,
                                char **stop_reason,
                                char **text_out,
                                char **error_msg)
{
    cJSON *root, *err_obj, *content, *sr_obj;
    cJSON *result_content = NULL;
    int is_openai;

    if (stop_reason) *stop_reason = NULL;
    if (text_out) *text_out = NULL;
    if (error_msg) *error_msg = NULL;

    is_openai = (provider && strcmp(provider, "openai") == 0);

    root = cJSON_Parse(json_str);
    if (!root) {
        if (error_msg) *error_msg = strdup("Failed to parse JSON response");
        return NULL;
    }

    /* Check for error response (same shape for both providers) */
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

    if (is_openai) {
        result_content = parse_full_response_openai(root, stop_reason,
                                                    text_out, error_msg);
        cJSON_Delete(root);
        return result_content;
    }

    /* === Anthropic Messages API path (default) === */

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
            char *iso = json_utf8_to_iso8859(buf);
            if (iso) { free(buf); buf = iso; }
        }
        *text_out = buf;
    }

    /* Duplicate content array so caller owns it */
    result_content = cJSON_Duplicate(content, 1);

    cJSON_Delete(root);
    return result_content;
}

int json_parse_usage(const char *provider, const char *json_str,
                     int *input_tokens, int *output_tokens)
{
    cJSON *root, *usage, *val;
    int is_openai;

    is_openai = (provider && strcmp(provider, "openai") == 0);

    root = cJSON_Parse(json_str);
    if (!root) return -1;

    usage = cJSON_GetObjectItemCaseSensitive(root, "usage");
    if (!usage) {
        cJSON_Delete(root);
        return -1;
    }

    if (input_tokens) {
        val = cJSON_GetObjectItemCaseSensitive(usage,
                is_openai ? "prompt_tokens" : "input_tokens");
        if (val && cJSON_IsNumber(val))
            *input_tokens = (int)val->valuedouble;
    }

    if (output_tokens) {
        val = cJSON_GetObjectItemCaseSensitive(usage,
                is_openai ? "completion_tokens" : "output_tokens");
        if (val && cJSON_IsNumber(val))
            *output_tokens = (int)val->valuedouble;
    }

    cJSON_Delete(root);
    return 0;
}

/* Recursively convert all string values in a cJSON tree from UTF-8 to
 * ISO-8859-1.  Modifies the tree in-place by replacing valuestring
 * pointers.  Use on tool input objects before passing to AmigaOS. */
void json_convert_strings_to_iso8859(cJSON *obj)
{
    cJSON *child;

    if (!obj) return;

    if (cJSON_IsString(obj) && obj->valuestring) {
        char *iso = json_utf8_to_iso8859(obj->valuestring);
        if (iso) {
            cJSON_free(obj->valuestring);
            obj->valuestring = iso;
        }
    }

    /* Recurse into children (arrays and objects) */
    for (child = obj->child; child; child = child->next)
        json_convert_strings_to_iso8859(child);
}
