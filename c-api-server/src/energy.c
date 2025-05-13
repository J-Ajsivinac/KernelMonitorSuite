#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "system_calls.h"
#include "cJSON.h"
#include "json_utils.h"

#define SYS_get_all_processes_energy 558

struct energy_usage {
    pid_t pid;
    unsigned long cpu_usage;
    unsigned long ram_usage;
    unsigned long io_read;
    unsigned long io_write;
    unsigned long energy_est;
};

char *get_energy_usage_json(){
    const unsigned int MAX_PROCESSES = 1000;
    struct energy_usage *processes = malloc(MAX_PROCESSES * sizeof(struct energy_usage));
    unsigned int count;
    cJSON *root;

    if (!processes) {
        root = create_error_json("Memory allocation failed");
        goto output_json;
    }
    
    if (syscall(SYS_get_all_processes_energy, processes, MAX_PROCESSES, &count) < 0) {
        root = create_error_json("System call failed");
        free(processes);
        goto output_json;
    }

    // Si todo fue bien, creamos el JSON con los datos
    root = cJSON_CreateArray();
    for (unsigned int i = 0; i < count; i++) {
        cJSON *proc = cJSON_CreateObject();
        cJSON_AddNumberToObject(proc, "pid", processes[i].pid);
        cJSON_AddNumberToObject(proc, "cpu_usage", processes[i].cpu_usage);
        cJSON_AddNumberToObject(proc, "ram_usage", processes[i].ram_usage);
        cJSON_AddNumberToObject(proc, "io_read", processes[i].io_read);
        cJSON_AddNumberToObject(proc, "io_write", processes[i].io_write);
        cJSON_AddNumberToObject(proc, "value", processes[i].energy_est);
        cJSON_AddItemToArray(root, proc);
    }
    free(processes);

output_json:
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);

    return json_str;
}
