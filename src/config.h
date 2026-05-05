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

#define CONFIG_AUTH_XAPIKEY  "x-api-key"
#define CONFIG_AUTH_BEARER   "bearer"
#define CONFIG_MAX_AUTH_LEN  16

/* Key realm = the service the key belongs to (independent of request format).
 * Determines which file the API key is read from / written to:
 *   ENV:AmigaAI/api_key.<realm>
 * E.g. OpenRouter uses one key for both Anthropic and OpenAI request formats. */
#define CONFIG_REALM_ANTHROPIC   "anthropic"
#define CONFIG_REALM_OPENROUTER  "openrouter"
#define CONFIG_REALM_OPENAI      "openai"
#define CONFIG_MAX_REALM_LEN     16

struct Config {
    char api_key[CONFIG_MAX_KEY_LEN];
    char model[CONFIG_MAX_MODEL_LEN];
    char system_prompt[CONFIG_MAX_PROMPT_LEN];
    int  max_tokens;
    char api_host[CONFIG_MAX_HOST_LEN];
    int  api_port;
    int  api_ssl;         /* 1 = HTTPS, 0 = plain HTTP (proxy) */
    char api_path[CONFIG_MAX_PATH_LEN];  /* API path, e.g. /v1/messages */
    char api_provider[CONFIG_MAX_PROVIDER_LEN];  /* "anthropic" or "openai" — request format */
    char api_auth[CONFIG_MAX_AUTH_LEN];  /* "x-api-key" or "bearer" — auth scheme */
    char api_key_realm[CONFIG_MAX_REALM_LEN];  /* "anthropic" | "openrouter" | "openai" */
};

/* Load config from ENV:AmigaAI/ */
int config_load(struct Config *cfg);

/* Save config to ENV:AmigaAI/ and optionally ENVARC:AmigaAI/ */
int config_save(const struct Config *cfg, int save_permanent);

/* Set defaults */
void config_defaults(struct Config *cfg);

/* Load the API key for a specific realm from ENV:AmigaAI/api_key.<realm>
 * (with fallback to ENV:AmigaAI/api_key for legacy installs when realm is anthropic).
 * Used to switch keys when the user changes provider in the UI.
 * out_key must be at least CONFIG_MAX_KEY_LEN bytes. */
void config_load_realm_key(const char *realm, char *out_key);

#endif /* AMIGAAI_CONFIG_H */
