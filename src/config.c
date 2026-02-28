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
    cfg->max_tokens = 1024;
    cfg->system_prompt[0] = '\0';
    cfg->api_key[0] = '\0';
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

    config_defaults(cfg);

    read_file_string(CONFIG_DIR_ENV "/api_key", cfg->api_key, CONFIG_MAX_KEY_LEN);
    read_file_string(CONFIG_DIR_ENV "/model", cfg->model, CONFIG_MAX_MODEL_LEN);
    read_file_string(CONFIG_DIR_ENV "/system_prompt", cfg->system_prompt, CONFIG_MAX_PROMPT_LEN);

    if (read_file_string(CONFIG_DIR_ENV "/max_tokens", buf, sizeof(buf))) {
        int val = atoi(buf);
        if (val > 0 && val <= 8192)
            cfg->max_tokens = val;
    }

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

    snprintf(path, sizeof(path), "%s/api_key", dir);
    write_file_string(path, cfg->api_key);

    snprintf(path, sizeof(path), "%s/model", dir);
    write_file_string(path, cfg->model);

    snprintf(path, sizeof(path), "%s/max_tokens", dir);
    write_file_int(path, cfg->max_tokens);

    if (cfg->system_prompt[0]) {
        snprintf(path, sizeof(path), "%s/system_prompt", dir);
        write_file_string(path, cfg->system_prompt);
    }

    return 1;
}

int config_save(const struct Config *cfg, int save_permanent)
{
    int ok;

    ok = save_to_dir(cfg, CONFIG_DIR_ENV);
    if (ok && save_permanent)
        ok = save_to_dir(cfg, CONFIG_DIR_ENVARC);

    return ok;
}
