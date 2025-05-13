#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>

#define __NR_kill_control 559

int main() {
    pid_t pid = 4648; 
    int signal = SIGKILL;

    long ret = syscall(__NR_kill_control, pid, signal);
    if (ret == 0) {
        printf("Señal enviada correctamente.\n");
    } else {
        perror("Error al enviar señal");
    }
    return 0;
}
