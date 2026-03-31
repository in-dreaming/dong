#ifndef DONG_HTTP_CLIENT_H
#define DONG_HTTP_CLIENT_H

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Dong HTTP Client (Platform-injected)
// =============================================================================
// Provides synchronous HTTP GET for resource loading (stylesheets, fetch API).
// Applications/backends inject via dong_platform_set_http_client().
// On Windows a default WinHTTP-based client is provided.

typedef enum DongHttpResult {
    DONG_HTTP_OK = 0,
    DONG_HTTP_ERR_NETWORK = 1,
    DONG_HTTP_ERR_TIMEOUT = 2,
    DONG_HTTP_ERR_UNSUPPORTED = 3,
    DONG_HTTP_ERR_INVALID_URL = 4,
} DongHttpResult;

typedef struct DongHttpResponse {
    int status_code;
    void* body;
    size_t body_size;
} DongHttpResponse;

typedef struct DongHttpClient DongHttpClient;

typedef struct DongHttpClientVTable {
    DongHttpResult (*get)(DongHttpClient* client, const char* url, DongHttpResponse* out_response);
    void (*free_response)(DongHttpClient* client, DongHttpResponse* response);
} DongHttpClientVTable;

struct DongHttpClient {
    const DongHttpClientVTable* vtable;
    void* user_data;
};

static inline DongHttpResult dong_http_get(DongHttpClient* client, const char* url,
                                           DongHttpResponse* out) {
    if (!out) return DONG_HTTP_ERR_INVALID_URL;
    out->body = NULL;
    out->body_size = 0;
    out->status_code = 0;
    return (client && client->vtable && client->vtable->get)
               ? client->vtable->get(client, url, out)
               : DONG_HTTP_ERR_UNSUPPORTED;
}

static inline void dong_http_free_response(DongHttpClient* client, DongHttpResponse* response) {
    if (!response || !response->body) return;
    if (client && client->vtable && client->vtable->free_response) {
        client->vtable->free_response(client, response);
        return;
    }
    free(response->body);
    response->body = NULL;
    response->body_size = 0;
}

#ifdef __cplusplus
}
#endif

#endif // DONG_HTTP_CLIENT_H
