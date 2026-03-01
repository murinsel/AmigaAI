/*
 * base64.c - Base64 encoding for AmigaAI
 *
 * Used to encode PNG screenshot data for the Claude API.
 */

#include "base64.h"
#include <stdlib.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(const unsigned char *data, size_t len, size_t *out_len)
{
    size_t olen, i, j;
    char *out;

    olen = 4 * ((len + 2) / 3);
    out = (char *)malloc(olen + 1);
    if (!out)
        return NULL;

    for (i = 0, j = 0; i + 2 < len; i += 3) {
        out[j++] = b64_table[(data[i] >> 2) & 0x3F];
        out[j++] = b64_table[((data[i] & 0x03) << 4) | ((data[i+1] >> 4) & 0x0F)];
        out[j++] = b64_table[((data[i+1] & 0x0F) << 2) | ((data[i+2] >> 6) & 0x03)];
        out[j++] = b64_table[data[i+2] & 0x3F];
    }

    if (i < len) {
        out[j++] = b64_table[(data[i] >> 2) & 0x3F];
        if (i + 1 < len) {
            out[j++] = b64_table[((data[i] & 0x03) << 4) | ((data[i+1] >> 4) & 0x0F)];
            out[j++] = b64_table[((data[i+1] & 0x0F) << 2)];
        } else {
            out[j++] = b64_table[((data[i] & 0x03) << 4)];
            out[j++] = '=';
        }
        out[j++] = '=';
    }

    out[j] = '\0';

    if (out_len)
        *out_len = j;

    return out;
}
