#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>
#include "system_calls.h"
#include "cJSON.h"
#include "json_utils.h"

#define SYS_network_usage 557

struct net_usage_info {
    unsigned long bytes_sent;
    unsigned long bytes_received;
};


char *get_network_usage_json(){
    struct net_usage_info info;

    long result = syscall(SYS_network_usage, &info);
    cJSON *root;

    if (result < 0) {
        root = create_error_json("Failed to get network usage");
        goto output_json;
    }
    root = cJSON_CreateObject();
   
    // Bytes enviados (en bytes y MB)
    cJSON_AddNumberToObject(root, "bytes_sent", info.bytes_sent);
    cJSON_AddNumberToObject(root, "bytes_sent_mb", info.bytes_sent / (1024.0 * 1024));
    
    // Bytes recibidos (en bytes y MB)
    cJSON_AddNumberToObject(root, "bytes_received", info.bytes_received);
    cJSON_AddNumberToObject(root, "bytes_received_mb", info.bytes_received / (1024.0 * 1024));

output_json:
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);

    return json_str;
}
