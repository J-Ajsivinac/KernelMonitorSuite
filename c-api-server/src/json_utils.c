#include "json_utils.h"
#include <cJSON.h>

cJSON* create_error_json(const char *error_msg) {
    cJSON *error_json = cJSON_CreateObject();
    if (!error_json) return NULL;
    
    cJSON_AddStringToObject(error_json, "status", "error");
    cJSON_AddStringToObject(error_json, "message", error_msg);
    return error_json;
}