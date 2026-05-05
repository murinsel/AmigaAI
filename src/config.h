#ifndef AMIGAAI_CONFIG_H
#define AMIGAAI_CONFIG_H

#define CONFIG_DIR_ENV      "ENV:AmigaAI"
#define CONFIG_DIR_ENVARC   "ENVARC:AmigaAI"

#define CONFIG_MAX_KEY_LEN       128
#define CONFIG_MAX_MODEL_LEN      64
#define CONFIG_MAX_HOST_LEN      128
#define CONFIG_MAX_PATH_LEN      128
#define CONFIG_MAX_PROVIDER_LEN   16
#define CONFIG_MAX_PROMPT_LEN   2048

#define CONFIG_PROVIDER_ANTHROPIC "anthropic"
#define CONFIG_PROVIDER_OPENAI    "openai"

struct Config {
    char api_key[CONFIG_MAX_KEY_LEN];
    char model[CONFIG_MAX_MODEL_LEN];
    char system_prompt[CONFIG_MAX_PROMPT_LEN];
    int  max_tokens;
    char api_host[CONFIG_MAX_HOST_LEN];
    int  api_port;
    int  api_ssl;         /* 1 = HTTPS, 0 = plain HTTP (proxy) */
    char api_path[CONFIG_MAX_PATH_LEN];  /* API path, e.g. /v1/messages */
    char api_provider[CONFIG_MAX_PROVIDER_LEN];  /* "anthropic" or "openai" */
};

/* Load config from ENV:AmigaAI/ */
int config_load(struct Config *cfg);

/* Save config to ENV:AmigaAI/ and optionally ENVARC:AmigaAI/ */
int config_save(const struct Config *cfg, int save_permanent);

/* Set defaults */
void config_defaults(struct Config *cfg);

#endif /* AMIGAAI_CONFIG_H */
