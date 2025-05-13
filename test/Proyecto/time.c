
// test_get_all_process_times.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

#define SYS_get_all_process_times 561  // Asegúrate que coincida con tu syscall

#define MAX_PROCESSES 1024

struct process_times {
    pid_t pid;
    unsigned long start_time;
    unsigned long end_time;
};

struct process_time_list {
    int max_processes;
    int num_processes;
    struct process_times *times;
};

int main() {
    struct process_times *times_buffer;
    struct process_time_list plist;

    times_buffer = malloc(sizeof(struct process_times) * MAX_PROCESSES);
    if (!times_buffer) {
        perror("malloc");
        return 1;
    }

    plist.max_processes = MAX_PROCESSES;
    plist.times = times_buffer;

    int result = syscall(SYS_get_all_process_times, &plist);
    if (result < 0) {
        perror("Error en syscall");
        free(times_buffer);
        return 1;
    }

    printf("Total de procesos reportados: %d\n\n", plist.num_processes);

    for (int i = 0; i < plist.num_processes; i++) {
        printf("PID: %d\n", plist.times[i].pid);
        printf("Inicio: %lu segundos desde boot\n", plist.times[i].start_time);
        if (plist.times[i].end_time != 0)
            printf("Final:  %lu segundos desde boot\n", plist.times[i].end_time);
        else
            printf("Final:  Aún activo\n");
        printf("-----------------------------\n");
    }

    free(times_buffer);
    return 0;
}
