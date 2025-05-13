#include "system_calls.h"
#include "cJSON.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "json_utils.h"

#define SYS_detailed_process_list 556
#define SYS_get_process_by_pid  560
#define SYS_get_all_process_times 561 

#define MAX_PROCESSES 1024
#define TASK_COMM_LEN 16

struct process_info {
    pid_t pid;
    char name[TASK_COMM_LEN];
    unsigned int cpu_percent;
    unsigned int ram_percent;
    int priority;
    char state_str[20];
    long state;
    uid_t uid;
    int num_threads;
    unsigned long start_time;
};

struct process_list {
    int max_processes;
    int num_processes;
    struct process_info *processes;
};

struct process_times {
    pid_t pid;
    char name[TASK_COMM_LEN];
    unsigned long start_time;
    unsigned long end_time;
};

struct process_time_list {
    int max_processes;
    int num_processes;
    struct process_times *times;
};

const char* get_username(uid_t uid){
    struct passwd *pwd = getpwuid(uid);
    if (pwd) return pwd->pw_name;
    static char uid_str[20];
    sprintf(uid_str, "%d", uid);
    return uid_str;
}

const char* get_state_str(long state) {
    switch (state) {
        case 0: return "Running"; case 1: return "Sleeping"; case 2: return "Disk Sleep";
        case 4: return "Zombie"; case 8: return "Stopped"; case 16: return "Tracing";
        case 32: return "Paging"; case 64: return "Dead"; case 128: return "Wakekill";
        default: return "Unknown";
    }
}

char *format_start_time(unsigned long start_time_ns) {
    struct sysinfo s_info;
    time_t boot_time, proc_time;
    struct tm *timeinfo;
    static char time_str[64];

    if (sysinfo(&s_info) == 0) {
        time(&boot_time);
        boot_time -= s_info.uptime;
        proc_time = boot_time + (start_time_ns / 1000000000UL);
        timeinfo = localtime(&proc_time);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
        return time_str;
    } else {
        return "N/A";
    }
}

char *get_process_info_json() {
    struct process_list list;
    list.max_processes = 1020;
    list.processes = malloc(list.max_processes * sizeof(struct process_info));
    cJSON *root = NULL;
    char *json_str = NULL;

    if (!list.processes) {
        root = create_error_json("Memory allocation failed");
        goto cleanup;
    }

    int result = syscall(SYS_detailed_process_list, &list);
    if (result < 0) {
        root = create_error_json("Failed to get process list");
        goto cleanup;
    }

    root = cJSON_CreateArray();
    for (int i = 0; i < list.num_processes; i++) {
        struct process_info *p = &list.processes[i];
        cJSON *proc = cJSON_CreateObject();
        
        cJSON_AddNumberToObject(proc, "pid", p->pid);
        cJSON_AddStringToObject(proc, "name", p->name);
        cJSON_AddNumberToObject(proc, "cpu", p->cpu_percent / 100.0);
        cJSON_AddNumberToObject(proc, "ram", p->ram_percent / 100.0);
        cJSON_AddNumberToObject(proc, "priority", p->priority);
        cJSON_AddStringToObject(proc, "state", get_state_str(p->state));
        cJSON_AddStringToObject(proc, "user", get_username(p->uid));
        cJSON_AddNumberToObject(proc, "threads", p->num_threads);
        cJSON_AddStringToObject(proc, "start_time", format_start_time(p->start_time));
        
        cJSON_AddItemToArray(root, proc);
    }

cleanup:
    if (list.processes) free(list.processes);
    if (root) {
        json_str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
    } else {
        json_str = cJSON_PrintUnformatted(create_error_json("Unknown error occurred"));
    }
    
    return json_str;
}

// Search PID

char *search_process_info_json(const char *json_str) {
    cJSON *body = NULL;
    cJSON *response = NULL;
    char *json_reply = NULL;
    struct process_info info_pid = {0};

    body = cJSON_Parse(json_str);
    if (!body) {
        response = create_error_json("Invalid JSON input");
        goto cleanup;
    }

    cJSON *pid_item = cJSON_GetObjectItem(body, "pid");
    if (!cJSON_IsNumber(pid_item)) {
        response = create_error_json("Missing or invalid pid field");
        goto cleanup;
    }

    pid_t pid = pid_item->valueint;
    int result = syscall(SYS_get_process_by_pid, pid, &info_pid);
    
    if (result < 0) {
        response = create_error_json("Process not found or access denied");
        goto cleanup;
    }

    response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "pid", info_pid.pid);
    cJSON_AddStringToObject(response, "name", info_pid.name);
    cJSON_AddNumberToObject(response, "cpu", info_pid.cpu_percent / 100.0);
    cJSON_AddNumberToObject(response, "ram", info_pid.ram_percent / 100.0);
    cJSON_AddNumberToObject(response, "priority", info_pid.priority);
    cJSON_AddStringToObject(response, "state", get_state_str(info_pid.state));
    cJSON_AddStringToObject(response, "user", get_username(info_pid.uid));
    cJSON_AddNumberToObject(response, "threads", info_pid.num_threads);
    cJSON_AddStringToObject(response, "start_time", format_start_time(info_pid.start_time));

cleanup:
    if (body) cJSON_Delete(body);
    if (response) {
        json_reply = cJSON_PrintUnformatted(response);
        cJSON_Delete(response);
    } else {
        json_reply = cJSON_PrintUnformatted(create_error_json("Unknown error occurred"));
    }
    
    return json_reply;
}

// Get time

char *get_process_time_json() {
    struct process_time_list plist = {0};
    cJSON *root = NULL;
    char *json_str = NULL;

    plist.times = malloc(sizeof(struct process_times) * MAX_PROCESSES);
    if (!plist.times) {
        root = create_error_json("Memory allocation failed");
        goto cleanup;
    }

    plist.max_processes = MAX_PROCESSES;
    int result = syscall(SYS_get_all_process_times, &plist);
    
    if (result < 0) {
        root = create_error_json("Failed to get process times");
        goto cleanup;
    }

    root = cJSON_CreateArray();
    for (int i = 0; i < plist.num_processes; i++) {
        struct process_times *p = &plist.times[i];
        cJSON *proc = cJSON_CreateObject();
        
        cJSON_AddNumberToObject(proc, "id", p->pid);
        cJSON_AddStringToObject(proc, "name", p->name);
        cJSON_AddStringToObject(proc, "startTime", format_start_time(p->start_time*1000));
        cJSON_AddItemToObject(proc, "endTime", 
            (p->end_time != 0) ? cJSON_CreateString(format_start_time(p->end_time)): cJSON_CreateNull());
        
        cJSON_AddItemToArray(root, proc);
    }

cleanup:
    if (plist.times) free(plist.times);
    if (root) {
        json_str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
    } else {
        json_str = cJSON_PrintUnformatted(create_error_json("Unknown error occurred"));
    }
    
    return json_str;
}