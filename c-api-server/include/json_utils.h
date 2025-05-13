#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <cJSON.h>

cJSON* create_error_json(const char *error_msg);

#endif // JSON_UTILS_H