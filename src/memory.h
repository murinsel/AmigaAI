#ifndef AMIGAAI_MEMORY_H
#define AMIGAAI_MEMORY_H

#define MEMORY_MAX_SIZE       4096
#define MEMORY_MAX_ENTRY_LEN   256
#define MEMORY_MAX_ENTRIES      64

#define MEMORY_FILE_ENV    "ENV:AmigaAI/memory"
#define MEMORY_FILE_ENVARC "ENVARC:AmigaAI/memory"

struct Memory {
    char entries[MEMORY_MAX_ENTRIES][MEMORY_MAX_ENTRY_LEN];
    int  count;
};

/* Load memory from ENVARC:AmigaAI/memory (or ENV: fallback).
 * Returns number of entries loaded. */
int memory_load(struct Memory *mem);

/* Save memory to both ENV: and ENVARC:.
 * Returns 0 on success, -1 on error. */
int memory_save(const struct Memory *mem);

/* Add a single entry. Returns 0 on success, -1 if full. */
int memory_add(struct Memory *mem, const char *entry);

/* Clear all memory entries. */
void memory_clear(struct Memory *mem);

/* Build the <memory> block for system prompt injection.
 * Writes into buf (max bufsize bytes).
 * Returns number of bytes written, 0 if memory is empty. */
int memory_format(const struct Memory *mem, char *buf, int bufsize);

/* Get a displayable string of all entries (numbered list).
 * Returns a newly allocated string (caller must free), or NULL if empty. */
char *memory_to_string(const struct Memory *mem);

#endif /* AMIGAAI_MEMORY_H */
