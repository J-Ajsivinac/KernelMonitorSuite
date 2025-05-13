#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_get_all_processes_energy 558 // Reemplazar con el número asignado

struct energy_usage {
    pid_t pid;
    unsigned long cpu_usage;
    unsigned long ram_usage;
    unsigned long io_read;
    unsigned long io_write;
    unsigned long energy_est;
};

int main() {
    const unsigned int MAX_PROCESSES = 1000;
    struct energy_usage *processes = malloc(MAX_PROCESSES * sizeof(struct energy_usage));
    unsigned int count;
    
    if (!processes) {
        perror("malloc");
        return 1;
    }
    
    if (syscall(SYS_get_all_processes_energy, processes, MAX_PROCESSES, &count) < 0) {
        perror("syscall");
        free(processes);
        return 1;
    }
    
    printf("Procesos obtenidos: %u\n", count);
    printf("PID\tCPU\tRAM(KB)\tI/O(R)\tI/O(W)\tEnergía\n");
    
    for (unsigned int i = 0; i < count; i++) {
        printf("%d\t%lu\t%lu\t%lu\t%lu\t%lu\n",
               processes[i].pid,
               processes[i].cpu_usage,
               processes[i].ram_usage,
               processes[i].io_read,
               processes[i].io_write,
               processes[i].energy_est);
    }
    
    free(processes);
    return 0;
}