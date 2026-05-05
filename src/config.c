#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>

void config_defaults(struct Config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    strncpy(cfg->model, "claude-sonnet-4-6", CONFIG_MAX_MODEL_LEN - 1);
    cfg->max_tokens = 4096;
    cfg->system_prompt[0] = '\0';
    cfg->api_key[0] = '\0';
    strncpy(cfg->api_host, "api.anthropic.com", CONFIG_MAX_HOST_LEN - 1);
    cfg->api_port = 443;
    cfg->api_ssl = 1;
    strncpy(cfg->api_path, "/v1/messages", CONFIG_MAX_PATH_LEN - 1);
    strncpy(cfg->api_provider, CONFIG_PROVIDER_ANTHROPIC,
            CONFIG_MAX_PROVIDER_LEN - 1);
    strncpy(cfg->api_auth, CONFIG_AUTH_XAPIKEY, CONFIG_MAX_AUTH_LEN - 1);
    strncpy(cfg->api_key_realm, CONFIG_REALM_ANTHROPIC,
            CONFIG_MAX_REALM_LEN - 1);
}

/* Read a single line from a file, strip trailing newline */
static int read_file_string(const char *path, char *buf, int maxlen)
{
    FILE *f;
    int len;

    f = fopen(path, "r");
    if (!f) return 0;

    if (!fgets(buf, maxlen, f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    /* Strip trailing whitespace/newline */
    len = strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r' || buf[len-1] == ' '))
        buf[--len] = '\0';

    return len > 0;
}

static int write_file_string(const char *path, const char *str)
{
    FILE *f;

    f = fopen(path, "w");
    if (!f) return 0;

    fputs(str, f);
    fputc('\n', f);
    fclose(f);
    return 1;
}

static int write_file_int(const char *path, int val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", val);
    return write_file_string(path, buf);
}

int config_load(struct Config *cfg)
{
    char buf[32];
    char key_path[64];

    config_defaults(cfg);

    /* Read api_provider (request format). Old configs default to "anthropic". */
    read_file_string(CONFIG_DIR_ENV "/api_provider",
                     cfg->api_provider, CONFIG_MAX_PROVIDER_LEN);
    if (cfg->api_provider[0] == '\0') {
        strncpy(cfg->api_provider, CONFIG_PROVIDER_ANTHROPIC,
                CONFIG_MAX_PROVIDER_LEN - 1);
    }

    /* Auth scheme: x-api-key (Anthropic native) or bearer (OpenRouter, OpenAI). */
    read_file_string(CONFIG_DIR_ENV "/api_auth",
                     cfg->api_auth, CONFIG_MAX_AUTH_LEN);
    if (cfg->api_auth[0] == '\0') {
        strncpy(cfg->api_auth, CONFIG_AUTH_XAPIKEY, CONFIG_MAX_AUTH_LEN - 1);
    }

    /* Key realm = which key file to use. Decoupled from provider because
     * OpenRouter serves both Anthropic and OpenAI formats with one key. */
    read_file_string(CONFIG_DIR_ENV "/api_key_realm",
                     cfg->api_key_realm, CONFIG_MAX_REALM_LEN);
    if (cfg->api_key_realm[0] == '\0') {
        strncpy(cfg->api_key_realm, CONFIG_REALM_ANTHROPIC,
                CONFIG_MAX_REALM_LEN - 1);
    }

    /* Load the actual key from the realm-specific file, with fallback
     * chain: api_key.<realm> -> api_key.<provider> (legacy v1) ->
     * api_key (legacy v0 single-key file). */
    snprintf(key_path, sizeof(key_path),
             CONFIG_DIR_ENV "/api_key.%s", cfg->api_key_realm);
    if (!read_file_string(key_path, cfg->api_key, CONFIG_MAX_KEY_LEN)) {
        snprintf(key_path, sizeof(key_path),
                 CONFIG_DIR_ENV "/api_key.%s", cfg->api_provider);
        if (!read_file_string(key_path, cfg->api_key, CONFIG_MAX_KEY_LEN)) {
            read_file_string(CONFIG_DIR_ENV "/api_key",
                             cfg->api_key, CONFIG_MAX_KEY_LEN);
        }
    }

    read_file_string(CONFIG_DIR_ENV "/model", cfg->model, CONFIG_MAX_MODEL_LEN);
    read_file_string(CONFIG_DIR_ENV "/system_prompt", cfg->system_prompt, CONFIG_MAX_PROMPT_LEN);

    if (read_file_string(CONFIG_DIR_ENV "/max_tokens", buf, sizeof(buf))) {
        int val = atoi(buf);
        if (val > 0 && val <= 16384) {
            /* Enforce minimum of 4096 for tool use to work properly */
            if (val < 4096) val = 4096;
            cfg->max_tokens = val;
        }
    }

    /* API endpoint settings */
    read_file_string(CONFIG_DIR_ENV "/api_host", cfg->api_host, CONFIG_MAX_HOST_LEN);

    if (read_file_string(CONFIG_DIR_ENV "/api_port", buf, sizeof(buf))) {
        int val = atoi(buf);
        if (val > 0 && val <= 65535)
            cfg->api_port = val;
    }

    if (read_file_string(CONFIG_DIR_ENV "/api_ssl", buf, sizeof(buf))) {
        cfg->api_ssl = atoi(buf) ? 1 : 0;
    }

    read_file_string(CONFIG_DIR_ENV "/api_path", cfg->api_path, CONFIG_MAX_PATH_LEN);

    /* Check if we have an API key */
    return cfg->api_key[0] != '\0';
}

static int save_to_dir(const struct Config *cfg, const char *dir)
{
    char path[256];
    BPTR lock;

    /* Create directory if needed */
    lock = Lock((CONST_STRPTR)dir, ACCESS_READ);
    if (!lock) {
        lock = CreateDir((CONST_STRPTR)dir);
        if (!lock) return 0;
    }
    UnLock(lock);

    /* Write to realm-specific key file */
    snprintf(path, sizeof(path), "%s/api_key.%s", dir, cfg->api_key_realm);
    write_file_string(path, cfg->api_key);

    /* Also write to legacy "api_key" file when realm is anthropic,
     * for backwards compat with shell scripts that read ENV:AmigaAI/api_key */
    if (strcmp(cfg->api_key_realm, CONFIG_REALM_ANTHROPIC) == 0) {
        snprintf(path, sizeof(path), "%s/api_key", dir);
        write_file_string(path, cfg->api_key);
    }

    snprintf(path, sizeof(path), "%s/model", dir);
    write_file_string(path, cfg->model);

    snprintf(path, sizeof(path), "%s/max_tokens", dir);
    write_file_int(path, cfg->max_tokens);

    if (cfg->system_prompt[0]) {
        snprintf(path, sizeof(path), "%s/system_prompt", dir);
        write_file_string(path, cfg->system_prompt);
    }

    snprintf(path, sizeof(path), "%s/api_host", dir);
    write_file_string(path, cfg->api_host);

    snprintf(path, sizeof(path), "%s/api_port", dir);
    write_file_int(path, cfg->api_port);

    snprintf(path, sizeof(path), "%s/api_ssl", dir);
    write_file_int(path, cfg->api_ssl);

    snprintf(path, sizeof(path), "%s/api_path", dir);
    write_file_string(path, cfg->api_path);

    snprintf(path, sizeof(path), "%s/api_provider", dir);
    write_file_string(path, cfg->api_provider);

    snprintf(path, sizeof(path), "%s/api_auth", dir);
    write_file_string(path, cfg->api_auth);

    snprintf(path, sizeof(path), "%s/api_key_realm", dir);
    write_file_string(path, cfg->api_key_realm);

    return 1;
}

void config_load_realm_key(const char *realm, char *out_key)
{
    char path[64];

    out_key[0] = '\0';

    snprintf(path, sizeof(path), CONFIG_DIR_ENV "/api_key.%s", realm);
    if (read_file_string(path, out_key, CONFIG_MAX_KEY_LEN))
        return;

    /* Legacy fallback: ENV:AmigaAI/api_key is treated as anthropic */
    if (strcmp(realm, CONFIG_REALM_ANTHROPIC) == 0)
        read_file_string(CONFIG_DIR_ENV "/api_key", out_key, CONFIG_MAX_KEY_LEN);
}

int config_save(const struct Config *cfg, int save_permanent)
{
    int ok;

    ok = save_to_dir(cfg, CONFIG_DIR_ENV);
    if (ok && save_permanent)
        ok = save_to_dir(cfg, CONFIG_DIR_ENVARC);

    return ok;
}
