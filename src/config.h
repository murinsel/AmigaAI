#ifndef AMIGAAI_CONFIG_H
#define AMIGAAI_CONFIG_H

#define CONFIG_DIR_ENV      "ENV:AmigaAI"
#define CONFIG_DIR_ENVARC   "ENVARC:AmigaAI"

#define CONFIG_MAX_KEY_LEN     128
#define CONFIG_MAX_MODEL_LEN    64
#define CONFIG_MAX_PROMPT_LEN 2048

struct Config {
    char api_key[CONFIG_MAX_KEY_LEN];
    char model[CONFIG_MAX_MODEL_LEN];
    char system_prompt[CONFIG_MAX_PROMPT_LEN];
    int  max_tokens;
};

/* Load config from ENV:AmigaAI/ */
int config_load(struct Config *cfg);

/* Save config to ENV:AmigaAI/ and optionally ENVARC:AmigaAI/ */
int config_save(const struct Config *cfg, int save_permanent);

/* Set defaults */
void config_defaults(struct Config *cfg);

#endif /* AMIGAAI_CONFIG_H */
