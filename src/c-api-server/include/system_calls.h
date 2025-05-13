#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H
#include <sys/types.h>

char *get_process_info_json();
char *search_process_info_json(const char *json_str);
char *kill_request_json(const char *json_str);
char *get_process_time_json();
char *get_resources_json();

char *get_energy_usage_json();
char *get_network_usage_json();

#endif // SYSTEM_CALLS_H