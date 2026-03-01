/*
 * mkcatalog - Build AmigaOS .catalog files from text translations
 *
 * Reads a simple text file with ID/string pairs and produces
 * an IFF CTLG binary catalog file for AmigaOS locale.library.
 *
 * Input format:
 *   ; comment
 *   <id number>
 *   <translated string>
 *
 * Escape sequences in strings:
 *   \xNN  - hex byte (e.g. \xe4 for ä in Latin-1)
 *   \n    - newline
 *   \033  - ESC (0x1B, for MUI formatting)
 *
 * Usage: mkcatalog <input.txt> <output.catalog> <language> [version]
 *
 * IFF CTLG format:
 *   FORM .... CTLG
 *     CSET ....   (code set chunk - 4 bytes, usually 0 for ISO-8859-1)
 *     FVER ....   (version string)
 *     STRS ....   (string data: pairs of [4-byte ID][NUL-terminated string][pad])
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRINGS 256
#define MAX_STR_LEN 512

struct CatEntry {
    int id;
    char str[MAX_STR_LEN];
};

/* Parse escape sequences in a string, modifying it in place.
 * Returns the new length. */
static int parse_escapes(char *s)
{
    char *r = s, *w = s;
    while (*r) {
        if (*r == '\\') {
            r++;
            if (*r == 'n') {
                *w++ = '\n';
                r++;
            } else if (*r == '0' && r[1] == '3' && r[2] == '3') {
                *w++ = '\033';
                r += 3;
            } else if (*r == 'x' || *r == 'X') {
                /* \xNN hex byte */
                unsigned int val = 0;
                r++;
                if (*r) {
                    char hex[3] = { r[0], r[1] ? r[1] : 0, 0 };
                    val = (unsigned int)strtoul(hex, NULL, 16);
                    r++;
                    if (*r && hex[1]) r++;
                }
                *w++ = (char)val;
            } else {
                *w++ = *r++;
            }
        } else {
            *w++ = *r++;
        }
    }
    *w = '\0';
    return (int)(w - s);
}

/* Write a big-endian 32-bit value */
static void write_be32(FILE *f, unsigned int val)
{
    unsigned char b[4];
    b[0] = (val >> 24) & 0xFF;
    b[1] = (val >> 16) & 0xFF;
    b[2] = (val >>  8) & 0xFF;
    b[3] = val & 0xFF;
    fwrite(b, 1, 4, f);
}

int main(int argc, char *argv[])
{
    FILE *fin, *fout;
    struct CatEntry entries[MAX_STRINGS];
    int count = 0;
    char line[MAX_STR_LEN];
    const char *language;
    const char *version_str = "$VER: AmigaAI.catalog 0.2 (01.03.2026)";
    unsigned int strs_size, fver_len, form_size;
    int i;

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <input.txt> <output.catalog> <language> [version]\n",
                argv[0]);
        return 1;
    }

    language = argv[3];
    if (argc > 4)
        version_str = argv[4];

    fin = fopen(argv[1], "r");
    if (!fin) {
        fprintf(stderr, "Cannot open input: %s\n", argv[1]);
        return 1;
    }

    /* Parse input file */
    while (fgets(line, sizeof(line), fin)) {
        char *nl;
        int id;

        /* Strip newline */
        nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        nl = strchr(line, '\r');
        if (nl) *nl = '\0';

        /* Skip comments and empty lines */
        if (line[0] == ';' || line[0] == '\0')
            continue;

        /* Try to parse as ID number */
        if (line[0] >= '0' && line[0] <= '9') {
            id = atoi(line);

            /* Read the next line as the string */
            if (!fgets(line, sizeof(line), fin))
                break;
            nl = strchr(line, '\n');
            if (nl) *nl = '\0';
            nl = strchr(line, '\r');
            if (nl) *nl = '\0';

            if (count < MAX_STRINGS) {
                entries[count].id = id;
                strncpy(entries[count].str, line, MAX_STR_LEN - 1);
                entries[count].str[MAX_STR_LEN - 1] = '\0';
                parse_escapes(entries[count].str);
                count++;
            }
        }
    }
    fclose(fin);

    printf("Parsed %d strings for language '%s'\n", count, language);

    /* Calculate STRS chunk size.
     * Each entry: 4-byte ID + 4-byte length + string data padded to 4 bytes.
     * FlexCat/CatComp pad string data to LONG (4-byte) boundary. */
    strs_size = 0;
    for (i = 0; i < count; i++) {
        int slen = (int)strlen(entries[i].str) + 1; /* include NUL */
        int padded = (slen + 3) & ~3;               /* round up to 4 */
        strs_size += 4;       /* ID */
        strs_size += 4;       /* string length */
        strs_size += padded;  /* string data padded to 4-byte boundary */
    }

    /* FVER string (NUL-terminated) */
    fver_len = (unsigned int)strlen(version_str) + 1;

    /* FORM size = CTLG(4) + FVER chunk(8+fver) + CSET chunk(8+32) + STRS chunk(8+strs) */
    form_size = 4;                      /* "CTLG" */
    form_size += 8 + fver_len;          /* FVER chunk */
    if (fver_len & 1) form_size++;      /* pad */
    form_size += 8 + 32;               /* CSET chunk (32 bytes data) */
    form_size += 8 + strs_size;         /* STRS chunk */

    /* Write output */
    fout = fopen(argv[2], "wb");
    if (!fout) {
        fprintf(stderr, "Cannot create output: %s\n", argv[2]);
        return 1;
    }

    /* FORM header */
    fwrite("FORM", 1, 4, fout);
    write_be32(fout, form_size);
    fwrite("CTLG", 1, 4, fout);

    /* FVER chunk (must come before CSET per FlexCat/CatComp convention) */
    fwrite("FVER", 1, 4, fout);
    write_be32(fout, fver_len);
    fwrite(version_str, 1, fver_len, fout);
    if (fver_len & 1)
        fputc(0, fout);  /* pad to even */

    /* CSET chunk - 32 bytes: CodeSet(4) + Reserved[7](28) */
    {
        unsigned char cset_data[32];
        memset(cset_data, 0, 32);
        /* cset_data[0..3] = 0 = ISO-8859-1 */
        fwrite("CSET", 1, 4, fout);
        write_be32(fout, 32);
        fwrite(cset_data, 1, 32, fout);
    }

    /* STRS chunk */
    fwrite("STRS", 1, 4, fout);
    write_be32(fout, strs_size);

    for (i = 0; i < count; i++) {
        int slen = (int)strlen(entries[i].str) + 1;
        int padded = (slen + 3) & ~3;  /* round up to 4 */
        int pad = padded - slen;
        write_be32(fout, (unsigned int)entries[i].id);
        write_be32(fout, (unsigned int)padded);
        fwrite(entries[i].str, 1, slen, fout);
        while (pad-- > 0)
            fputc(0, fout);  /* pad to 4-byte boundary */
    }

    fclose(fout);
    printf("Wrote %s (%u bytes STRS data)\n", argv[2], strs_size);

    return 0;
}
