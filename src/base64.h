#ifndef AMIGAAI_BASE64_H
#define AMIGAAI_BASE64_H

#include <stddef.h>

/* Encode binary data to base64 string.
 * Returns newly allocated null-terminated string (caller must free).
 * If out_len is not NULL, the encoded length (excluding NUL) is stored there.
 * Returns NULL on allocation failure. */
char *base64_encode(const unsigned char *data, size_t len, size_t *out_len);

#endif /* AMIGAAI_BASE64_H */
