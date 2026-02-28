#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* AmigaOS includes */
#include <proto/exec.h>
#include <proto/socket.h>

/* AmiSSL includes */
#include <libraries/amisslmaster.h>
#include <libraries/amissl.h>
#include <proto/amisslmaster.h>
#include <proto/amissl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

/* Amiga socket includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/filio.h>
#include <netinet/in.h>
#include <netdb.h>

/* Library bases */
struct Library *AmiSSLMasterBase = NULL;
struct Library *AmiSSLBase       = NULL;
struct Library *SocketBase       = NULL;

static SSL_CTX *ssl_ctx = NULL;

/* Event callback for non-blocking I/O */
static HttpEventCallback http_event_cb = NULL;
static void *http_event_data = NULL;

/* API request/response log file path (NULL = disabled) */
static const char *api_log_path = NULL;

void http_set_event_callback(HttpEventCallback cb, void *userdata)
{
    http_event_cb = cb;
    http_event_data = userdata;
}

void http_set_api_log(const char *path)
{
    api_log_path = path;
}

/* Write a separator + label + text to the API log file */
static void api_log_write(const char *label, const char *text, long len)
{
    FILE *f;
    if (!api_log_path) return;

    f = fopen(api_log_path, "a");
    if (!f) return;

    fprintf(f, "==== %s ====\n", label);
    if (len > 0)
        fwrite(text, 1, len, f);
    else
        fputs(text, f);
    fputs("\n\n", f);
    fclose(f);
}

int http_init(void)
{
    /* Open bsdsocket.library (Roadshow) */
    SocketBase = OpenLibrary("bsdsocket.library", 4);
    if (!SocketBase) {
        printf("ERROR: Cannot open bsdsocket.library\n");
        return -1;
    }

    /* Open AmiSSL master library */
    AmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_MIN_VERSION);
    if (!AmiSSLMasterBase) {
        printf("ERROR: Cannot open amisslmaster.library\n");
        return -2;
    }

    if (!InitAmiSSLMaster(AMISSL_V3xx, TRUE)) {
        printf("ERROR: InitAmiSSLMaster failed\n");
        printf("Please install AmiSSL v5.1 or newer\n");
        return -3;
    }

    /* Open AmiSSL itself */
    AmiSSLBase = OpenAmiSSL();
    if (!AmiSSLBase) {
        printf("ERROR: Cannot open AmiSSL\n");
        return -4;
    }

    if (InitAmiSSL(AmiSSL_SocketBase, (ULONG)SocketBase,
                   AmiSSL_ErrNoPtr, (ULONG)&errno,
                   TAG_DONE) != 0)
    {
        printf("ERROR: InitAmiSSL failed\n");
        return -5;
    }

    /* Create SSL context */
    ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!ssl_ctx) {
        printf("ERROR: SSL_CTX_new failed\n");
        return -6;
    }

    /* Load default CA certificates */
    SSL_CTX_set_default_verify_paths(ssl_ctx);
    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);

    return 0;
}

void http_cleanup(void)
{
    if (ssl_ctx) {
        SSL_CTX_free(ssl_ctx);
        ssl_ctx = NULL;
    }

    if (AmiSSLBase) {
        CleanupAmiSSLA(NULL);
        CloseAmiSSL();
        AmiSSLBase = NULL;
    }

    if (AmiSSLMasterBase) {
        CloseLibrary(AmiSSLMasterBase);
        AmiSSLMasterBase = NULL;
    }

    if (SocketBase) {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
    }
}

/* Resolve hostname and connect a TCP socket */
static int tcp_connect(const char *host, int port)
{
    struct hostent *he;
    struct sockaddr_in addr;
    int sock;

    he = gethostbyname((char *)host);
    if (!he) {
        printf("ERROR: Cannot resolve %s\n", host);
        return -1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("ERROR: socket() failed\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("ERROR: connect() failed\n");
        CloseSocket(sock);
        return -1;
    }

    return sock;
}

/* Read all data from SSL connection into a dynamically growing buffer.
 * Uses non-blocking I/O with WaitSelect to allow periodic event
 * processing (GUI updates, abort checking). */
static char *ssl_read_all(SSL *ssl, int sock, long *out_len, int *aborted)
{
    char *buf;
    long  buf_size = HTTP_INITIAL_BUF_SIZE;
    long  total    = 0;
    int   n;
    long  one = 1;

    if (aborted) *aborted = 0;

    buf = malloc(buf_size);
    if (!buf) return NULL;

    /* Set socket to non-blocking so SSL_read returns immediately
     * when no data is available */
    IoctlSocket(sock, FIONBIO, (char *)&one);

    for (;;) {
        if (total + HTTP_READ_CHUNK_SIZE >= buf_size) {
            char *new_buf;
            buf_size *= 2;
            new_buf = realloc(buf, buf_size);
            if (!new_buf) { free(buf); return NULL; }
            buf = new_buf;
        }

        n = SSL_read(ssl, buf + total, HTTP_READ_CHUNK_SIZE);
        if (n > 0) {
            total += n;
            continue;
        }

        {
            int ssl_err = SSL_get_error(ssl, n);

            if (ssl_err == SSL_ERROR_WANT_READ ||
                ssl_err == SSL_ERROR_WANT_WRITE)
            {
                /* No data yet - wait with timeout then check for abort */
                fd_set rfds;
                struct timeval tv;

                /* Check abort callback */
                if (http_event_cb &&
                    http_event_cb(http_event_data))
                {
                    if (aborted) *aborted = 1;
                    free(buf);
                    /* Restore blocking mode */
                    one = 0;
                    IoctlSocket(sock, FIONBIO, (char *)&one);
                    return NULL;
                }

                /* Wait up to 1 second for data */
                FD_ZERO(&rfds);
                FD_SET(sock, &rfds);
                tv.tv_sec  = 1;
                tv.tv_usec = 0;
                WaitSelect(sock + 1, &rfds, NULL, NULL, &tv, NULL);
                continue;
            }

            /* Connection closed or error */
            break;
        }
    }

    /* Restore blocking mode */
    one = 0;
    IoctlSocket(sock, FIONBIO, (char *)&one);

    buf[total] = '\0';
    if (out_len) *out_len = total;
    return buf;
}

/* Parse HTTP status code from first response line */
static int parse_status_code(const char *response)
{
    /* "HTTP/1.1 200 OK\r\n" */
    const char *p = strstr(response, " ");
    if (!p) return -1;
    return atoi(p + 1);
}

/* Find the body start (after \r\n\r\n) */
static const char *find_body(const char *response, long len)
{
    const char *p;
    (void)len;

    p = strstr(response, "\r\n\r\n");
    if (p) return p + 4;

    /* Fallback: some servers use \n\n */
    p = strstr(response, "\n\n");
    if (p) return p + 2;

    return NULL;
}

/* Decode chunked transfer encoding in-place */
static long decode_chunked(char *data, long data_len)
{
    char *src = data;
    char *dst = data;
    char *end = data + data_len;

    while (src < end) {
        long chunk_size;
        char *line_end;

        /* Read chunk size (hex) */
        chunk_size = strtol(src, &line_end, 16);
        if (chunk_size == 0) break;

        /* Skip past \r\n after chunk size */
        while (line_end < end && (*line_end == '\r' || *line_end == '\n'))
            line_end++;

        if (line_end + chunk_size > end)
            chunk_size = end - line_end;

        /* Copy chunk data */
        if (dst != line_end)
            memmove(dst, line_end, chunk_size);
        dst += chunk_size;
        src = line_end + chunk_size;

        /* Skip trailing \r\n */
        while (src < end && (*src == '\r' || *src == '\n'))
            src++;
    }

    *dst = '\0';
    return dst - data;
}

/* Check if response uses chunked transfer encoding */
static int is_chunked(const char *response)
{
    const char *p = strstr(response, "Transfer-Encoding:");
    if (!p) p = strstr(response, "transfer-encoding:");
    if (!p) return 0;
    return strstr(p, "chunked") != NULL;
}

int http_post(const char *host,
              const char *path,
              const char **headers,
              const char *body,
              struct HttpResponse *response)
{
    int   sock = -1;
    SSL  *ssl  = NULL;
    char *request = NULL;
    char *raw_response = NULL;
    int   request_len;
    int   ret = -1;
    long  raw_len = 0;
    const char *body_start;
    int i;

    memset(response, 0, sizeof(*response));

    /* Build HTTP request */
    request = malloc(HTTP_MAX_HEADER_SIZE + strlen(body) + 256);
    if (!request) goto done;

    request_len = snprintf(request, HTTP_MAX_HEADER_SIZE,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n",
        path, host, (long)strlen(body));

    /* Append custom headers */
    if (headers) {
        for (i = 0; headers[i] != NULL; i++) {
            request_len += snprintf(request + request_len,
                HTTP_MAX_HEADER_SIZE - request_len,
                "%s\r\n", headers[i]);
        }
    }

    /* End of headers */
    request_len += snprintf(request + request_len,
        HTTP_MAX_HEADER_SIZE - request_len, "\r\n");

    /* Append body */
    memcpy(request + request_len, body, strlen(body));
    request_len += strlen(body);

    /* TCP connect */
    sock = tcp_connect(host, HTTPS_PORT);
    if (sock < 0) goto done;

    /* SSL handshake */
    ssl = SSL_new(ssl_ctx);
    if (!ssl) {
        printf("ERROR: SSL_new failed\n");
        goto done;
    }

    SSL_set_fd(ssl, sock);
    SSL_set_tlsext_host_name(ssl, host);

    {
        int ssl_rc = SSL_connect(ssl);
        if (ssl_rc <= 0) {
            int ssl_err = SSL_get_error(ssl, ssl_rc);
            unsigned long ossl_err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(ossl_err, err_buf, sizeof(err_buf));
            printf("ERROR: SSL handshake failed (ssl_err=%d)\n", ssl_err);
            printf("  OpenSSL: %s\n", err_buf);

            /* If certificate verification failed, retry without verify */
            if (ssl_err == SSL_ERROR_SSL) {
                printf("  Retrying without certificate verification...\n");
                SSL_free(ssl);
                ssl = NULL;

                /* Create a new context without verification for this connection */
                SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);

                ssl = SSL_new(ssl_ctx);
                if (ssl) {
                    SSL_set_fd(ssl, sock);
                    SSL_set_tlsext_host_name(ssl, host);
                    ssl_rc = SSL_connect(ssl);
                    if (ssl_rc <= 0) {
                        ssl_err = SSL_get_error(ssl, ssl_rc);
                        ossl_err = ERR_get_error();
                        ERR_error_string_n(ossl_err, err_buf, sizeof(err_buf));
                        printf("ERROR: SSL retry also failed (ssl_err=%d)\n", ssl_err);
                        printf("  OpenSSL: %s\n", err_buf);
                        /* Restore verify */
                        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
                        goto done;
                    }
                    printf("  SSL connected (without cert verify)\n");
                } else {
                    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
                    goto done;
                }
                /* Restore verify for future connections */
                SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
            } else {
                goto done;
            }
        }
    }

    /* Log outgoing request body */
    api_log_write("REQUEST", body, 0);

    /* Send request */
    if (SSL_write(ssl, request, request_len) != request_len) {
        printf("ERROR: SSL_write failed\n");
        goto done;
    }

    /* Read response (non-blocking with event callback) */
    {
        int aborted = 0;
        raw_response = ssl_read_all(ssl, sock, &raw_len, &aborted);
        if (aborted) {
            printf("  [http] Request aborted by user\n");
            ret = -2;  /* Distinguish abort from error */
            goto done;
        }
        if (!raw_response || raw_len == 0) {
            printf("ERROR: Empty response from server\n");
            goto done;
        }
    }

    /* Parse status code */
    response->status_code = parse_status_code(raw_response);

    /* Find body */
    body_start = find_body(raw_response, raw_len);
    if (!body_start) {
        printf("ERROR: Malformed HTTP response\n");
        goto done;
    }

    /* Extract body */
    {
        long body_len = raw_len - (body_start - raw_response);
        response->body = malloc(body_len + 1);
        if (!response->body) goto done;
        memcpy(response->body, body_start, body_len);
        response->body[body_len] = '\0';
        response->body_length = body_len;

        /* Handle chunked encoding */
        if (is_chunked(raw_response)) {
            response->body_length = decode_chunked(response->body, body_len);
        }
    }

    /* Log response body */
    if (response->body)
        api_log_write("RESPONSE", response->body, response->body_length);

    ret = 0;

done:
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock >= 0) CloseSocket(sock);
    free(request);
    free(raw_response);
    return ret;
}
