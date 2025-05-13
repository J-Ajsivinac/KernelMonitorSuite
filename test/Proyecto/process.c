#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <pwd.h>
#include <sys/sysinfo.h>
#include <time.h>

// Definir el número de syscall (el mismo que usamos en syscall_64.tbl)
#define SYS_detailed_process_list 556

// Definiciones para coincidencia con el kernel
#define TASK_COMM_LEN 16
#define MAX_USERNAME_LEN 32

// Estructuras idénticas a las definidas en el kernel
struct process_info {
    pid_t pid;                        // ID del proceso
    char name[TASK_COMM_LEN];         // Nombre del proceso
    unsigned int cpu_percent;         // Porcentaje de CPU * 100 (calculado en el kernel)
    unsigned int ram_percent;         // Porcentaje de RAM * 100 (calculado en el kernel)
    int priority;                     // Prioridad del proceso
    char state_str[20];               // Estado del proceso como string
    long state;                // Estado del proceso
    uid_t uid;                        // ID del usuario que ejecuta el proceso
    //char username[MAX_USERNAME_LEN];  // Nombre del usuario
    int num_threads;                  // Número de hilos
    unsigned long start_time;         // Tiempo de inicio
};

struct process_list {
    int max_processes;         // Número máximo de procesos que puede contener el buffer
    int num_processes;         // Número de procesos actualmente en el buffer
    struct process_info *processes; // Buffer de información de procesos
};

const char* get_username(uid_t uid){
    struct passwd *pwd = getpwuid(uid);
    if(pwd){
        return pwd->pw_name;
    }
    static char uid_str[20];
    sprintf(uid_str, "%d", uid);
    return uid_str;
}

// Convertir estado del proceso a string legible
const char* get_state_str(long state) {
    switch (state) {
        case 0: return "Running";
        case 1: return "Sleeping";
        case 2: return "Disk Sleep";
        case 4: return "Zombie";
        case 8: return "Stopped";
        case 16: return "Tracing";
        case 32: return "Paging";
        case 64: return "Dead";
        case 128: return "Wakekill";
        default: return "Unknown";
    }
}

void print_start_time(unsigned long start_time_ns) {
    struct sysinfo s_info;
    time_t boot_time, proc_time;
    struct tm *timeinfo;
    char time_str[64];

    if (sysinfo(&s_info) == 0) {
        time(&boot_time);
        boot_time -= s_info.uptime;

        // Si usas jiffies en kernel, conviértelo a segundos:
        // proc_time = boot_time + (start_time_ns / HZ);
        proc_time = boot_time + (start_time_ns / 1000000000UL); // nanosegundos a segundos

        timeinfo = localtime(&proc_time);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
        printf("%s", time_str);
    } else {
        printf("N/A");
    }
}


int main() {
    struct process_list list;
    int i, result;
    
    // Asignar memoria para hasta 1024 procesos
    list.max_processes = 1000;
    list.processes = malloc(list.max_processes * sizeof(struct process_info));
    
    if (!list.processes) {
        perror("Failed to allocate memory");
        return EXIT_FAILURE;
    }
    
    // Llamar a nuestra syscall
    result = syscall(SYS_detailed_process_list, &list);
    
    if (result < 0) {
        perror("Syscall failed");
        free(list.processes);
        return EXIT_FAILURE;
    }
    
    // Imprimir encabezado
    printf("%-6s %-16s %-8s %-8s %-8s %-12s %-12s %-8s %-10s\n", 
        "PID", "NAME", "%CPU", "%RAM", "PRIO", "STATE", "USER", "THREADS", "START");
    
    // Imprimir información de cada proceso
    // Toda la información viene directamente del kernel
    for (i = 0; i < list.num_processes; i++) {
        struct process_info *proc = &list.processes[i];
        
        printf("%-6d %-16s %6.2f %6.2f %8d %-12s %-12s %8d ",
               proc->pid,
               proc->name,
               proc->cpu_percent / 10000.0,  // Convertir de entero a float para mostrar
               proc->ram_percent / 100.0,  // Convertir de entero a float para mostrar
               proc->priority,
               get_state_str(proc->state),            // Ahora usamos el string de estado del kernel
               get_username(proc->uid),
               proc->num_threads);
        print_start_time(proc->start_time);
        printf("\n");
    }
    
    // Liberar memoria
    free(list.processes);
    
    return EXIT_SUCCESS;
}