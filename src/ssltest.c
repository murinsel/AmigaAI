/*
 * SSLTest - AmiSSL/Network Diagnostic Tool for AmigaAI
 *
 * Tests each step of the HTTPS connection pipeline and reports
 * exactly where things go wrong. Users can post the output when
 * reporting SSL issues.
 *
 * Usage: SSLTest [hostname] [port]
 *   Default: api.anthropic.com 443
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* AmigaOS includes */
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/socket.h>

/* AmiSSL includes */
#include <libraries/amisslmaster.h>
#include <libraries/amissl.h>
#include <proto/amisslmaster.h>
#include <proto/amissl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

/* Amiga socket includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* Library bases */
struct Library *AmiSSLMasterBase = NULL;
struct Library *AmiSSLBase       = NULL;
struct Library *SocketBase       = NULL;

static int step = 0;
static int errors = 0;
static int warnings = 0;

static void ok(const char *msg)
{
    printf("  [%d] OK: %s\n", ++step, msg);
}

static void fail(const char *msg)
{
    printf("  [%d] FAIL: %s\n", ++step, msg);
    errors++;
}

static void warn(const char *msg)
{
    printf("  [%d] WARN: %s\n", ++step, msg);
    warnings++;
}

static void info(const char *msg)
{
    printf("       %s\n", msg);
}

int main(int argc, char **argv)
{
    const char *host = "api.anthropic.com";
    int port = 443;
    struct hostent *he;
    struct sockaddr_in addr;
    int sock = -1;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int rc;

    if (argc > 1) host = argv[1];
    if (argc > 2) port = atoi(argv[2]);

    printf("\nAmigaAI SSL Diagnostic Tool\n");
    printf("===========================\n");
    printf("Target: %s:%d\n\n", host, port);

    /* ---- Step 1: bsdsocket.library ---- */
    printf("[Network Stack]\n");
    SocketBase = OpenLibrary("bsdsocket.library", 4);
    if (SocketBase) {
        char buf[64];
        snprintf(buf, sizeof(buf), "bsdsocket.library v%d",
                 (int)SocketBase->lib_Version);
        ok(buf);
    } else {
        fail("Cannot open bsdsocket.library v4+");
        info("Install Roadshow or a compatible TCP/IP stack.");
        goto summary;
    }

    /* ---- Step 2: AmiSSL Master ---- */
    printf("\n[AmiSSL]\n");
    AmiSSLMasterBase = OpenLibrary("amisslmaster.library",
                                   AMISSLMASTER_MIN_VERSION);
    if (AmiSSLMasterBase) {
        char buf[64];
        snprintf(buf, sizeof(buf), "amisslmaster.library v%d",
                 (int)AmiSSLMasterBase->lib_Version);
        ok(buf);
    } else {
        fail("Cannot open amisslmaster.library");
        info("Install AmiSSL v5 from Aminet:");
        info("  https://aminet.net/package/util/libs/AmiSSL-v5-OS3");
        goto summary;
    }

    /* ---- Step 3: InitAmiSSLMaster ---- */
    if (InitAmiSSLMaster(AMISSL_V3xx, TRUE)) {
        ok("InitAmiSSLMaster(AMISSL_V3xx) succeeded");
    } else {
        fail("InitAmiSSLMaster failed");
        info("AmiSSL installation may be incomplete or corrupted.");
        info("Try reinstalling AmiSSL v5.");
        goto summary;
    }

    /* ---- Step 4: OpenAmiSSL ---- */
    AmiSSLBase = OpenAmiSSL();
    if (AmiSSLBase) {
        ok("OpenAmiSSL succeeded");
    } else {
        fail("OpenAmiSSL failed");
        info("Possible causes:");
        info("  - Not enough memory");
        info("  - AmiSSL libraries incomplete");
        goto summary;
    }

    /* ---- Step 5: InitAmiSSL ---- */
    rc = InitAmiSSL(AmiSSL_SocketBase, (ULONG)SocketBase,
                    AmiSSL_ErrNoPtr, (ULONG)&errno,
                    TAG_DONE);
    if (rc == 0) {
        ok("InitAmiSSL succeeded");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "InitAmiSSL failed (rc=%d)", rc);
        fail(buf);
        goto summary;
    }

    /* ---- Step 6: SSL context ---- */
    printf("\n[SSL Context]\n");
    ctx = SSL_CTX_new(TLS_client_method());
    if (ctx) {
        ok("SSL_CTX_new(TLS_client_method) succeeded");
    } else {
        unsigned long e = ERR_get_error();
        char buf[256];
        ERR_error_string_n(e, buf, sizeof(buf));
        fail("SSL_CTX_new failed");
        info(buf);
        goto summary;
    }

    /* ---- Step 7: CA certificates ---- */
    rc = SSL_CTX_set_default_verify_paths(ctx);
    if (rc == 1) {
        ok("SSL_CTX_set_default_verify_paths succeeded");
    } else {
        warn("SSL_CTX_set_default_verify_paths failed");
        info("CA certificates may not be installed.");
        info("Check: AmiSSL:certs/ directory exists and contains .pem files");
    }

    /* ---- Step 8: DNS resolution ---- */
    printf("\n[Connection to %s:%d]\n", host, port);
    he = gethostbyname((char *)host);
    if (he) {
        char buf[128];
        unsigned char *a = (unsigned char *)he->h_addr;
        snprintf(buf, sizeof(buf), "DNS resolved: %d.%d.%d.%d",
                 a[0], a[1], a[2], a[3]);
        ok(buf);
    } else {
        fail("DNS resolution failed");
        info("Check your DNS settings in ENV:Sys/Resolv.conf");
        info("Or try: nslookup api.anthropic.com");
        goto cleanup;
    }

    /* ---- Step 9: TCP connect ---- */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fail("socket() failed");
        goto cleanup;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        ok("TCP connection established");
    } else {
        fail("TCP connect() failed");
        info("The server may be unreachable. Check your internet connection.");
        goto cleanup;
    }

    /* ---- Step 10: SSL handshake with verify ---- */
    printf("\n[SSL Handshake]\n");
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    ssl = SSL_new(ctx);
    if (!ssl) {
        fail("SSL_new failed");
        goto cleanup;
    }

    SSL_set_fd(ssl, sock);
    SSL_set_tlsext_host_name(ssl, host);

    rc = SSL_connect(ssl);
    if (rc == 1) {
        const char *version = SSL_get_version(ssl);
        const char *cipher  = SSL_get_cipher_name(ssl);
        char buf[128];

        snprintf(buf, sizeof(buf), "SSL handshake OK (%s, %s)",
                 version, cipher);
        ok(buf);

        /* Show certificate info */
        {
            X509 *cert = SSL_get_peer_certificate(ssl);
            if (cert) {
                X509_NAME *subject = X509_get_subject_name(cert);
                X509_NAME *issuer  = X509_get_issuer_name(cert);
                char subj_buf[256], iss_buf[256];

                X509_NAME_oneline(subject, subj_buf, sizeof(subj_buf));
                X509_NAME_oneline(issuer, iss_buf, sizeof(iss_buf));

                printf("       Subject: %s\n", subj_buf);
                printf("       Issuer:  %s\n", iss_buf);

                {
                    long verify_result = SSL_get_verify_result(ssl);
                    if (verify_result == X509_V_OK) {
                        ok("Certificate verification PASSED");
                    } else {
                        char vbuf[128];
                        snprintf(vbuf, sizeof(vbuf),
                                 "Certificate verify error: %s (%ld)",
                                 X509_verify_cert_error_string(verify_result),
                                 verify_result);
                        warn(vbuf);
                    }
                }
                X509_free(cert);
            } else {
                warn("No server certificate received");
            }
        }

        /* Quick HTTP test */
        {
            const char *req = "GET / HTTP/1.0\r\nHost: api.anthropic.com\r\n\r\n";
            char resp[512];
            int n;

            SSL_write(ssl, req, strlen(req));
            n = SSL_read(ssl, resp, sizeof(resp) - 1);
            if (n > 0) {
                resp[n] = '\0';
                /* Show first line of response */
                {
                    char *nl = strchr(resp, '\r');
                    if (!nl) nl = strchr(resp, '\n');
                    if (nl) *nl = '\0';
                }
                printf("       HTTP response: %s\n", resp);
                ok("HTTPS request succeeded");
            } else {
                warn("Could not read HTTP response");
            }
        }
    } else {
        int ssl_err = SSL_get_error(ssl, rc);
        unsigned long ossl_err = ERR_get_error();
        char err_buf[256];

        ERR_error_string_n(ossl_err, err_buf, sizeof(err_buf));
        fail("SSL handshake FAILED (with certificate verification)");
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "ssl_err=%d", ssl_err);
            info(buf);
        }
        info(err_buf);

        /* Print full error stack */
        while ((ossl_err = ERR_get_error()) != 0) {
            ERR_error_string_n(ossl_err, err_buf, sizeof(err_buf));
            info(err_buf);
        }

        /* Step 11: Retry without verification */
        SSL_free(ssl);
        ssl = NULL;
        CloseSocket(sock);
        sock = -1;

        printf("\n[Retry without certificate verification]\n");

        /* Reconnect TCP */
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0 || connect(sock, (struct sockaddr *)&addr,
                                sizeof(addr)) < 0)
        {
            fail("TCP reconnect failed");
            goto cleanup;
        }
        ok("TCP reconnected");

        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
        ssl = SSL_new(ctx);
        if (!ssl) {
            fail("SSL_new failed on retry");
            goto cleanup;
        }

        SSL_set_fd(ssl, sock);
        SSL_set_tlsext_host_name(ssl, host);

        rc = SSL_connect(ssl);
        if (rc == 1) {
            const char *version = SSL_get_version(ssl);
            const char *cipher  = SSL_get_cipher_name(ssl);
            char buf[128];
            snprintf(buf, sizeof(buf),
                     "SSL handshake OK without verify (%s, %s)",
                     version, cipher);
            ok(buf);
            info("=> The problem is certificate verification.");
            info("   Possible fixes:");
            info("   1. Check that AmiSSL:certs/ contains CA certificates");
            info("   2. Reinstall AmiSSL v5 (includes Mozilla CA bundle)");
            info("   3. Check system date/time is correct (date command)");
            info("   4. Use AmigaAI with NOSSL and a local HTTPS proxy");
        } else {
            ssl_err = SSL_get_error(ssl, rc);
            ossl_err = ERR_get_error();
            ERR_error_string_n(ossl_err, err_buf, sizeof(err_buf));
            fail("SSL handshake FAILED even without verification");
            {
                char buf[64];
                snprintf(buf, sizeof(buf), "ssl_err=%d", ssl_err);
                info(buf);
            }
            info(err_buf);
            while ((ossl_err = ERR_get_error()) != 0) {
                ERR_error_string_n(ossl_err, err_buf, sizeof(err_buf));
                info(err_buf);
            }
            info("=> This may be a memory issue or AmiSSL installation problem.");
            info("   Check available memory with 'avail' command.");
        }
    }

cleanup:
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (ctx)  SSL_CTX_free(ctx);
    if (sock >= 0) CloseSocket(sock);

    if (AmiSSLBase) {
        CleanupAmiSSLA(NULL);
        CloseAmiSSL();
    }

summary:
    printf("\n===========================\n");
    printf("Result: %d error(s), %d warning(s)\n", errors, warnings);
    if (errors == 0 && warnings == 0) {
        printf("All tests passed! SSL connection works.\n");
    } else if (errors == 0) {
        printf("Connection works but with warnings.\n");
    } else {
        printf("Connection FAILED. Please post this output\n");
        printf("at https://github.com/murinsel/AmigaAI/issues/4\n");
    }
    printf("\n");

    if (AmiSSLMasterBase) CloseLibrary(AmiSSLMasterBase);
    if (SocketBase) CloseLibrary(SocketBase);

    return errors > 0 ? RETURN_ERROR : RETURN_OK;
}
