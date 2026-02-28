#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>

/* Ensure directory exists, create if needed (same pattern as config.c) */
static int ensure_dir(const char *dir)
{
    BPTR lock;

    lock = Lock((CONST_STRPTR)dir, ACCESS_READ);
    if (!lock) {
        lock = CreateDir((CONST_STRPTR)dir);
        if (!lock) return -1;
    }
    UnLock(lock);
    return 0;
}

int memory_load(struct Memory *mem)
{
    FILE *f;
    char line[MEMORY_MAX_ENTRY_LEN];
    int len;

    memset(mem, 0, sizeof(*mem));

    /* Try permanent storage first, then session */
    f = fopen(MEMORY_FILE_ENVARC, "r");
    if (!f)
        f = fopen(MEMORY_FILE_ENV, "r");
    if (!f)
        return 0;

    while (mem->count < MEMORY_MAX_ENTRIES &&
           fgets(line, sizeof(line), f) != NULL)
    {
        /* Strip trailing whitespace/newline */
        len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' ||
               line[len - 1] == '\r' || line[len - 1] == ' '))
            line[--len] = '\0';

        /* Skip empty lines */
        if (len == 0) continue;

        strncpy(mem->entries[mem->count], line, MEMORY_MAX_ENTRY_LEN - 1);
        mem->entries[mem->count][MEMORY_MAX_ENTRY_LEN - 1] = '\0';
        mem->count++;
    }

    fclose(f);
    return mem->count;
}

static int save_to_file(const struct Memory *mem, const char *path)
{
    FILE *f;
    int i;

    f = fopen(path, "w");
    if (!f) return -1;

    for (i = 0; i < mem->count; i++) {
        fputs(mem->entries[i], f);
        fputc('\n', f);
    }

    fclose(f);
    return 0;
}

int memory_save(const struct Memory *mem)
{
    int ok = 0;

    /* Ensure directories exist */
    ensure_dir("ENV:AmigaAI");
    ensure_dir("ENVARC:AmigaAI");

    if (save_to_file(mem, MEMORY_FILE_ENV) != 0)
        ok = -1;
    if (save_to_file(mem, MEMORY_FILE_ENVARC) != 0)
        ok = -1;

    return ok;
}

int memory_add(struct Memory *mem, const char *entry)
{
    if (!entry || !entry[0]) return -1;
    if (mem->count >= MEMORY_MAX_ENTRIES) return -1;

    strncpy(mem->entries[mem->count], entry, MEMORY_MAX_ENTRY_LEN - 1);
    mem->entries[mem->count][MEMORY_MAX_ENTRY_LEN - 1] = '\0';
    mem->count++;

    return 0;
}

void memory_clear(struct Memory *mem)
{
    memset(mem, 0, sizeof(*mem));
}

int memory_format(const struct Memory *mem, char *buf, int bufsize)
{
    int pos = 0;
    int i;

    if (mem->count == 0) return 0;

    pos += snprintf(buf + pos, bufsize - pos,
        "<memory>\n"
        "The following are facts and preferences you remember "
        "about the user from previous conversations. "
        "Use this knowledge naturally:\n");

    for (i = 0; i < mem->count && pos < bufsize - 4; i++) {
        pos += snprintf(buf + pos, bufsize - pos,
                        "- %s\n", mem->entries[i]);
    }

    pos += snprintf(buf + pos, bufsize - pos, "</memory>\n\n");
    return pos;
}

char *memory_to_string(const struct Memory *mem)
{
    char *buf;
    int pos = 0;
    int bufsize;
    int i;

    if (mem->count == 0) return NULL;

    /* Allocate enough for numbered entries */
    bufsize = mem->count * (MEMORY_MAX_ENTRY_LEN + 8);
    buf = malloc(bufsize);
    if (!buf) return NULL;

    for (i = 0; i < mem->count; i++) {
        pos += snprintf(buf + pos, bufsize - pos,
                        "%d. %s\n", i + 1, mem->entries[i]);
    }

    return buf;
}
