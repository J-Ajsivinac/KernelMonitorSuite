#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#define SYS_get_cpu_time 551
#define SYS_get_cpu_temp 552
#define SYS_listen_key 553

void test_get_cpu_time() {
    pid_t pid;
    long time_used;
    
    printf("Ingrese el ID del proceso (PID): ");
    if (scanf("%d", &pid) != 1) {
        fprintf(stderr, "Error: Entrada inválida. Debe ingresar un número entero.\n");
        return;
    }

    time_used = syscall(SYS_get_cpu_time, pid);
    if (time_used < 0) {
        perror("Error en la syscall");
        return;
    }

    printf("Tiempo de CPU usado por el proceso %d: %ld ms\n", pid, time_used);
}

void test_get_cpu_temp() {
    long temp = syscall(SYS_get_cpu_temp);
    if (temp < 0) {
        perror("Error obteniendo la temperatura");
        return;
    }
    printf("Temperatura actual de la CPU: %ld°C\n", temp);
}

void test_listen_key() {
    printf("Esperando que se presione la tecla 'a'...\n");
    syscall(SYS_listen_key, 64353);  // tecla 'a'
    printf("Tecla 'a' presionada!\n");
}

int main() {
    int opcion;
    while (1) {
        printf("\nSeleccione una syscall a probar:\n");
        printf("1. Obtener tiempo de CPU usado por un proceso\n");
        printf("2. Obtener temperatura de la CPU\n");
        printf("3. Escuchar la tecla 'a'\n");
        printf("4. Salir\n");
        printf("Opción: ");
        
        if (scanf("%d", &opcion) != 1) {
            fprintf(stderr, "Entrada inválida.\n");
            return 1;
        }
        
        switch (opcion) {
            case 1:
                test_get_cpu_time();
                break;
            case 2:
                test_get_cpu_temp();
                break;
            case 3:
                test_listen_key();
                break;
            case 4:
                printf("Saliendo...\n");
                return 0;
            default:
                printf("Opción no válida. Intente nuevamente.\n");
        }
    }
}
