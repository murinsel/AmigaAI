/*
 * models.c - Dynamic model list fetching for AmigaAI
 *
 * Both Anthropic (/v1/models) and OpenAI/OpenRouter (/v1/models or
 * /api/v1/models) expose a JSON list of available models. The schemas
 * differ slightly but both put a "data" array at the top level with
 * objects that have an "id" string. We extract those id strings.
 */

#include "models.h"
#include "http.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>

/* Read raw lines from ENV:AmigaAI/models.<realm> into the list. */
int models_load_cache(const char *realm, struct ModelList *out)
{
    char path[64];
    FILE *f;
    char line[MODELS_MAX_NAME];

    out->count = 0;

    snprintf(path, sizeof(path),
             "ENV:AmigaAI/models.%s", realm);
    f = fopen(path, "r");
    if (!f) return -1;

    while (out->count < MODELS_MAX_COUNT &&
           fgets(line, sizeof(line), f))
    {
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' ||
                           line[len-1] == ' '))
            line[--len] = '\0';
        if (len == 0) continue;

        strncpy(out->names[out->count], line, MODELS_MAX_NAME - 1);
        out->names[out->count][MODELS_MAX_NAME - 1] = '\0';
        out->count++;
    }
    fclose(f);
    return out->count > 0 ? 0 : -1;
}

static int write_list_to(const char *path, const struct ModelList *list)
{
    FILE *f;
    int i;
    BPTR lock;
    char dir[64];
    char *slash;

    /* Ensure directory exists */
    strncpy(dir, path, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        lock = Lock((CONST_STRPTR)dir, ACCESS_READ);
        if (!lock) {
            lock = CreateDir((CONST_STRPTR)dir);
            if (!lock) return -1;
        }
        UnLock(lock);
    }

    f = fopen(path, "w");
    if (!f) return -1;
    for (i = 0; i < list->count; i++) {
        fputs(list->names[i], f);
        fputc('\n', f);
    }
    fclose(f);
    return 0;
}

int models_save_cache(const char *realm, const struct ModelList *list)
{
    char path_env[64], path_arc[64];

    snprintf(path_env, sizeof(path_env),
             "ENV:AmigaAI/models.%s", realm);
    snprintf(path_arc, sizeof(path_arc),
             "ENVARC:AmigaAI/models.%s", realm);

    if (write_list_to(path_env, list) < 0) return -1;
    /* ENVARC failure is non-fatal (drive may be full / not writable) */
    write_list_to(path_arc, list);
    return 0;
}

/* Derive the GET /models path from the configured POST path.
 *   /v1/messages              -> /v1/models   (Anthropic)
 *   /api/v1/messages          -> /api/v1/models  (OpenRouter Anthropic)
 *   /api/v1/chat/completions  -> /api/v1/models  (OpenRouter OpenAI)
 *   /v1/chat/completions      -> /v1/models   (OpenAI native)
 */
static void derive_models_path(const char *api_path, char *out, int outlen)
{
    const char *slash;
    int prefix_len;

    if (!api_path || !*api_path) {
        snprintf(out, outlen, "/v1/models");
        return;
    }
    slash = strrchr(api_path, '/');
    if (!slash) {
        snprintf(out, outlen, "/v1/models");
        return;
    }
    prefix_len = (int)(slash - api_path);
    if (prefix_len >= outlen - 8) prefix_len = outlen - 8;
    memcpy(out, api_path, prefix_len);
    out[prefix_len] = '\0';

    /* Strip trailing /chat if present (so /api/v1/chat -> /api/v1) */
    {
        int n = strlen(out);
        if (n >= 5 && strcmp(out + n - 5, "/chat") == 0)
            out[n - 5] = '\0';
    }

    strncat(out, "/models", outlen - strlen(out) - 1);
}

int models_fetch(const struct Config *cfg, struct ModelList *out,
                 char **err_msg)
{
    char auth_header[256];
    char models_path[128];
    const char *headers[4];
    int hdr_idx = 0;
    int is_bearer;
    int is_anthropic_format;
    struct HttpResponse response;
    cJSON *root, *data;
    int rc;

    out->count = 0;
    if (err_msg) *err_msg = NULL;

    is_bearer = (strcmp(cfg->api_auth, CONFIG_AUTH_BEARER) == 0);
    is_anthropic_format = (strcmp(cfg->api_provider,
                                  CONFIG_PROVIDER_ANTHROPIC) == 0);

    if (is_bearer) {
        snprintf(auth_header, sizeof(auth_header),
                 "Authorization: Bearer %s", cfg->api_key);
    } else {
        snprintf(auth_header, sizeof(auth_header),
                 "x-api-key: %s", cfg->api_key);
    }

    headers[hdr_idx++] = auth_header;
    if (is_anthropic_format)
        headers[hdr_idx++] = "anthropic-version: 2023-06-01";
    headers[hdr_idx] = NULL;

    derive_models_path(cfg->api_path, models_path, sizeof(models_path));

    printf("  [models] GET %s%s\n", cfg->api_host, models_path);

    rc = http_get(cfg->api_host, cfg->api_port, cfg->api_ssl,
                  models_path, headers, &response);
    if (rc != 0) {
        if (err_msg) *err_msg = strdup("HTTPS request failed");
        free(response.body);
        return -1;
    }

    if (response.status_code != 200) {
        char buf[128];
        snprintf(buf, sizeof(buf), "HTTP %d", response.status_code);
        if (err_msg) *err_msg = strdup(buf);
        free(response.body);
        return -1;
    }

    root = cJSON_Parse(response.body);
    free(response.body);
    if (!root) {
        if (err_msg) *err_msg = strdup("Invalid JSON response");
        return -1;
    }

    data = cJSON_GetObjectItemCaseSensitive(root, "data");
    if (!data || !cJSON_IsArray(data)) {
        if (err_msg) *err_msg = strdup("No data array in response");
        cJSON_Delete(root);
        return -1;
    }

    {
        int i, n = cJSON_GetArraySize(data);
        for (i = 0; i < n && out->count < MODELS_MAX_COUNT; i++) {
            cJSON *item = cJSON_GetArrayItem(data, i);
            cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
            if (id && cJSON_IsString(id) && id->valuestring[0]) {
                strncpy(out->names[out->count], id->valuestring,
                        MODELS_MAX_NAME - 1);
                out->names[out->count][MODELS_MAX_NAME - 1] = '\0';
                out->count++;
            }
        }
    }

    cJSON_Delete(root);

    if (out->count == 0) {
        if (err_msg) *err_msg = strdup("No models in response");
        return -1;
    }
    return 0;
}

const char **models_to_source_array(struct ModelList *list)
{
    const char **arr;
    int i;

    arr = malloc((list->count + 1) * sizeof(const char *));
    if (!arr) return NULL;
    for (i = 0; i < list->count; i++)
        arr[i] = list->names[i];
    arr[list->count] = NULL;
    return arr;
}
