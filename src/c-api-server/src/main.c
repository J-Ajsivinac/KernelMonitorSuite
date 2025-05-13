#include "mongoose.h"
#include "routes.h"

int main(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    const char *port = "http://0.0.0.0:8000";
    struct mg_connection *conn = mg_http_listen(&mgr, port, handle_request, NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error: no se pudo iniciar el servidor en %s\n", port);
        return 1;
    }

    printf("✅ API corriendo en %s\n", port);

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}
