#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <stdlib.h>
#include "cJSON.h"
#include "system_calls.h"
#include "json_utils.h"

#define SYS_resources 562

static double kb_to_gb(unsigned long kb) {
    return (double)kb / (1024.0 * 1024.0);
}

struct sysresources_info {
    unsigned long cpu_usage;     /* Uso de CPU en décimas de porcentaje */
    unsigned long ram_total;     /* RAM total en KB */
    unsigned long ram_used;      /* RAM usada en KB */
    unsigned long ram_usage;     /* Uso de RAM en décimas de porcentaje */
};

char *get_resources_json() {
    struct sysresources_info info;
    cJSON *response = NULL;
    char *json_string = NULL;

    // Llamada al sistema
    int ret = syscall(SYS_resources, &info);
    if (ret != 0) {
        response = create_error_json("Failed to get system resources");
        goto cleanup;
    }

    // Crear respuesta exitosa
    response = cJSON_CreateObject();

    // Datos de CPU
    cJSON *cpu = cJSON_CreateObject();
    cJSON_AddNumberToObject(cpu, "percentage", info.cpu_usage / 10.0); 
    cJSON_AddItemToObject(response, "CPU", cpu);

    // Datos de RAM
    cJSON *ram = cJSON_CreateObject();
    cJSON_AddNumberToObject(ram, "percentage", info.ram_usage / 10.0);
    cJSON_AddNumberToObject(ram, "used_gb", kb_to_gb(info.ram_used));
    cJSON_AddNumberToObject(ram, "total_gb", kb_to_gb(info.ram_total));
    cJSON_AddItemToObject(response, "RAM", ram);

cleanup:
    // Convertir a string JSON
    json_string = cJSON_Print(response);
    
    // Liberar memoria
    if (response) {
        cJSON_Delete(response);
    }
    
    return json_string;
}