
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

#define __NR_get_system_network_usage 557

struct net_usage_info {
    unsigned long bytes_sent;
    unsigned long bytes_received;
};

int main() {
    struct net_usage_info info;

    long result = syscall(__NR_get_system_network_usage, &info);
    if (result < 0) {
        fprintf(stderr, "Error al invocar la syscall: %s\n", strerror(errno));
        return 1;
    }

    printf("Tráfico de red total del sistema:\n");
    printf("  Bytes enviados   : %lu (%.2f MB)\n", info.bytes_sent, info.bytes_sent / (1024.0 * 1024));
    printf("  Bytes recibidos  : %lu (%.2f MB)\n", info.bytes_received, info.bytes_received / (1024.0 * 1024));

    return 0;
}
