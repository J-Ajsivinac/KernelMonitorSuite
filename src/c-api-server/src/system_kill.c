#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <stdlib.h>
#include "cJSON.h"
#include "system_calls.h"
#include "json_utils.h"

#define SYS_kill_control 559

char *kill_request_json(const char *json_str) {
    cJSON *body = NULL;
    cJSON *response = NULL;
    char *json_reply = NULL;

    // Parsear el JSON de entrada
    body = cJSON_Parse(json_str);
    if (!body) {
        response = create_error_json("Invalid JSON input");
        goto cleanup;
    }

    // Validar campos requeridos
    cJSON *pid_item = cJSON_GetObjectItem(body, "pid");
    cJSON *sig_item = cJSON_GetObjectItem(body, "signal");
    
    if (!cJSON_IsNumber(pid_item) || !cJSON_IsNumber(sig_item)) {
        response = create_error_json("Missing or invalid pid/signal fields");
        goto cleanup;
    }

    // Extraer valores
    pid_t pid = pid_item->valueint;
    int signal = sig_item->valueint;

    // Llamada al sistema
    int result = syscall(SYS_kill_control, pid, signal);
    
    // Construir respuesta
    if (result == 0) {
        response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddStringToObject(response, "message", "Signal sent successfully");
        cJSON_AddNumberToObject(response, "pid", pid);
        cJSON_AddNumberToObject(response, "signal", signal);
    } else {
        response = create_error_json("Failed to send signal (check process existence or permissions)");
    }

cleanup:
    if (body) {
        cJSON_Delete(body);
    }
    
    if (response) {
        json_reply = cJSON_PrintUnformatted(response);
        cJSON_Delete(response);
    } else {
        json_reply = cJSON_PrintUnformatted(create_error_json("Unknown error occurred"));
    }
    
    return json_reply;
}