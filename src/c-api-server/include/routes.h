#ifndef ROUTES_H
#define ROUTES_H

#include "mongoose.h"

// Tipo de función para los handlers
typedef void (*route_handler_fn)(struct mg_connection *c, struct mg_http_message *hm);

// Struct para mapear rutas a handlers
typedef struct {
    const char *method;  // "GET", "POST", etc.
    const char *path;    // "/api/..."
    void (*handler)(struct mg_connection *, struct mg_http_message *);
} route_entry;

// El manejador de eventos principal que usa el router
void handle_request(struct mg_connection *c, int ev, void *ev_data);

#endif // ROUTES_H
