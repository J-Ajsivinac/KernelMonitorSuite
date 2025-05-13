#define MG_ENABLE_HTTP_MATCH_URI 1
#include "routes.h"
#include <string.h>
#include <time.h>
#include "mongoose.h"
#include "system_calls.h"
#include "cJSON.h"

// Función para obtener los encabezados CORS comunes
const char *get_cors_headers(void) {
    return "Access-Control-Allow-Origin: *\r\n"
           "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
           "Access-Control-Allow-Headers: Content-Type\r\n";
}

// Envía una respuesta HTTP con los encabezados CORS y manejo dinámico del buffer
static void send_http_response(struct mg_connection *c, int status_code, 
                              const char *extra_headers, const char *body_fmt, ...) {
    // Construir los encabezados combinados
    char headers[500] = "Content-Type: application/json\r\n";
    
    // Añadir los encabezados CORS
    strcat(headers, get_cors_headers());
    
    // Añadir headers extra si existen
    if (extra_headers != NULL && strlen(extra_headers) > 0) {
        strcat(headers, extra_headers);
    }

    // Tamaño inicial para el buffer
    size_t buffer_size = 256 * 1024; // 256 KB inicialmente
    char *body_buffer = malloc(buffer_size);
    
    if (body_buffer == NULL) {
        // Error en la asignación de memoria
        mg_http_reply(c, 500, headers, "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}");
        return;
    }
    
    va_list ap;
    size_t required_size; // Cambiado a size_t para evitar problemas de comparación
    
    // Primer intento de formateo para determinar el tamaño necesario
    va_start(ap, body_fmt);
    int result = vsnprintf(body_buffer, buffer_size, body_fmt, ap);
    va_end(ap);
    
    // vsnprintf devuelve un valor negativo en caso de error
    if (result < 0) {
        free(body_buffer);
        mg_http_reply(c, 500, headers, "{\"status\":\"error\",\"message\":\"Formatting error\"}");
        return;
    }
    
    required_size = (size_t)result; // Convertir resultado a size_t después de verificar que no es negativo
    
    // Si el buffer es demasiado pequeño, reajustamos el tamaño
    if (required_size >= buffer_size) {
        free(body_buffer);
        buffer_size = required_size + 1024;
        
        body_buffer = malloc(buffer_size);
        if (body_buffer == NULL) {
            // Error en la segunda asignación de memoria
            mg_http_reply(c, 500, headers, "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}");
            return;
        }
        
        // Re-formatear con el nuevo buffer más grande
        va_start(ap, body_fmt);
        vsnprintf(body_buffer, buffer_size, body_fmt, ap);
        va_end(ap);
    }
    
    // Enviar la respuesta con el cuerpo ya formateado
    mg_http_reply(c, status_code, headers, "%s", body_buffer);
    
    // Liberar la memoria
    free(body_buffer);
}



/// ========
/// HANDLERS
/// ========

static void handle_cors_preflight(struct mg_connection *c) {
    mg_http_reply(c, 204, get_cors_headers(), "");
}

static void handle_not_found(struct mg_connection *c) {
    send_http_response(c, 404, "", "{\"status\":\"error\",\"message\":\"Not found\"}");
}

static void handle_get_time(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;  // No se usa, pero mantenido por convención
    send_http_response(c, 200, "", "{%m:%lu}\n", MG_ESC("time"), time(NULL));
}

static void handle_get_processes(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    char *json = get_process_info_json();
    if (json) {
        send_http_response(c, 200, "", "%s", json);
        free(json);
    } else {
        send_http_response(c, 500, "", "{\"status\":\"error\",\"message\":\"Internal server error\"}");
    }
}

static void handle_kill_process(struct mg_connection *c, struct mg_http_message *hm) {
    char *response_json = kill_request_json(hm->body.buf);
    
    if (response_json) {
        // Verificamos si la respuesta contiene un error
        cJSON *json = cJSON_Parse(response_json);
        if (json && cJSON_HasObjectItem(json, "status") && 
            strcmp(cJSON_GetObjectItem(json, "status")->valuestring, "error") == 0) {
            send_http_response(c, 400, "", "%s", response_json);
        } else {
            send_http_response(c, 200, "", "%s", response_json);
        }
        if (json) cJSON_Delete(json);
        free(response_json);
    } else {
        send_http_response(c, 400, "", "{\"status\":\"error\",\"message\":\"Invalid request payload\"}");
    }
}

static void handle_search_process(struct mg_connection *c, struct mg_http_message *hm) {
    char *response_json = search_process_info_json(hm->body.buf);
    
    if (response_json) {
        cJSON *json = cJSON_Parse(response_json);
        if (json && cJSON_HasObjectItem(json, "status") && 
            strcmp(cJSON_GetObjectItem(json, "status")->valuestring, "error") == 0) {
            send_http_response(c, 404, "", "%s", response_json);
        } else {
            send_http_response(c, 200, "", "%s", response_json);
        }
        if (json) cJSON_Delete(json);
        free(response_json);
    } else {
        send_http_response(c, 400, "", "{\"status\":\"error\",\"message\":\"Invalid or missing PID field\"}");
    }
}

static void handle_get_resources(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    char *json = get_resources_json();
    
    if (json) {
        send_http_response(c, 200, "", "%s", json);
        free(json);
    } else {
        send_http_response(c, 500, "", "{\"status\":\"error\",\"message\":\"Failed to get system resources\"}");
    }
}

static void handle_get_processes_time(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    char *json = get_process_time_json();
    if (json) {
        send_http_response(c, 200, "", "%s", json);
        free(json);
    } else {
        send_http_response(c, 500, "", "{\"status\":\"error\",\"message\":\"Failed to get process times\"}");
    }
}

static void handle_get_energy_usage(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    char *json = get_energy_usage_json();
    if (json) {
        send_http_response(c, 200, "", "%s", json);
        free(json);
    } else {
        send_http_response(c, 500, "", "{\"status\":\"error\",\"message\":\"Failed to get energy usage\"}");
    }
}

static void handle_get_network_usage(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    char *json = get_network_usage_json();
    if (json) {
        send_http_response(c, 200, "", "%s", json);
        free(json);
    } else {
        send_http_response(c, 500, "", "{\"status\":\"error\",\"message\":\"Failed to get network usage\"}");
    }
}
///
/// RUTAS
///

static route_entry routes[] = {
    {"GET","/api/time/get", handle_get_time},
    {"GET","/api/process", handle_get_processes},
    {"GET","/api/process/time", handle_get_processes_time},
    {"GET","/api/resources", handle_get_resources},
    {"GET","/api/network", handle_get_network_usage},
    {"GET","/api/energy", handle_get_energy_usage},
    {"POST","/api/process/search", handle_search_process},
    {"POST", "/api/processes/kill", handle_kill_process},
};

///
/// ROUTER PRINCIPAL
///

void handle_request(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        
        // Handle preflight CORS requests
        if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
            handle_cors_preflight(c);
            return;
        }

        int found = 0;
        for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
            if (mg_strcmp(hm->method, mg_str(routes[i].method)) == 0 &&
                mg_match(hm->uri, mg_str(routes[i].path), NULL)) {
                found = 1;
                routes[i].handler(c, hm);
                return;
            }
        }

        if (!found) {
            handle_not_found(c);
        }
    }
}