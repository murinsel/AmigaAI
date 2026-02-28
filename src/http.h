#ifndef AMIGAAI_HTTP_H
#define AMIGAAI_HTTP_H

#define HTTP_MAX_HEADER_SIZE  4096
#define HTTP_INITIAL_BUF_SIZE 8192
#define HTTP_READ_CHUNK_SIZE  4096

#define HTTPS_PORT 443

/* Callback for periodic event processing during long I/O.
 * Called every ~1 second during SSL reads.
 * Return non-zero to abort the request. */
typedef int (*HttpEventCallback)(void *userdata);

struct HttpResponse {
    int   status_code;
    char *body;           /* Null-terminated response body (caller must free) */
    long  body_length;
    int   input_tokens;   /* Parsed from response, 0 if unavailable */
    int   output_tokens;
};

/* Initialize the HTTP subsystem (AmiSSL + bsdsocket.library).
 * Must be called once at startup. Returns 0 on success. */
int http_init(void);

/* Cleanup the HTTP subsystem. Call at shutdown. */
void http_cleanup(void);

/* Perform an HTTPS POST request.
 * host:     Hostname (e.g. "api.anthropic.com")
 * path:     Request path (e.g. "/v1/messages")
 * headers:  Array of "Key: Value" strings, NULL-terminated
 * body:     POST body (JSON string)
 * response: Output structure (caller must free response->body)
 * Returns 0 on success, negative on error. */
int http_post(const char *host,
              const char *path,
              const char **headers,
              const char *body,
              struct HttpResponse *response);

/* Set event callback for non-blocking I/O.
 * The callback is called periodically during SSL reads
 * to allow GUI event processing and abort checking. */
void http_set_event_callback(HttpEventCallback cb, void *userdata);

/* Enable API request/response logging to a file.
 * Pass NULL to disable. The path string must remain valid. */
void http_set_api_log(const char *path);

#endif /* AMIGAAI_HTTP_H */
