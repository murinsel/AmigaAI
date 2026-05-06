#ifndef AMIGAAI_MODELS_H
#define AMIGAAI_MODELS_H

#include "config.h"

#define MODELS_MAX_COUNT 64
#define MODELS_MAX_NAME  CONFIG_MAX_MODEL_LEN
/* +5 for "/api_" prefix limit on cache filename, kept simple */

struct ModelList {
    char  names[MODELS_MAX_COUNT][MODELS_MAX_NAME];
    int   count;
};

/* Load cached models for the given realm from
 * ENV:AmigaAI/models.<realm>. Returns 0 on success, -1 if no cache. */
int models_load_cache(const char *realm, struct ModelList *out);

/* Save model list to ENV:AmigaAI/models.<realm> (and ENVARC). */
int models_save_cache(const char *realm, const struct ModelList *list);

/* Fetch the live model list from the API and store in *out.
 * Uses cfg->api_host/port/path/auth/key to talk to /v1/models
 * (or /api/v1/models on OpenRouter — derived from the configured path).
 * Returns 0 on success, negative on failure (with *err filled in). */
int models_fetch(const struct Config *cfg, struct ModelList *out,
                 char **err_msg);

/* Build a NULL-terminated array of pointers into list->names suitable
 * for MUI's MUIA_List_SourceArray. The returned pointer must be freed
 * with free() after the list object no longer references it. */
const char **models_to_source_array(struct ModelList *list);

#endif /* AMIGAAI_MODELS_H */
