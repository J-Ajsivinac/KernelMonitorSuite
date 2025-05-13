#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sysresources.h"

int main() {
    struct sysresources_info info;
    int ret;

    while (1) {
        ret = sysresources(&info);
        if (ret != 0) {
            fprintf(stderr, "Error al llamar a sysresources: %d\n", ret);
            return 1;
        }

        printf("CPU: %lu.%01lu%% | RAM: %lu.%01lu%% (%lu/%lu KB)\n",
               info.cpu_usage / 10, info.cpu_usage % 10,
               info.ram_usage / 10, info.ram_usage % 10,
               info.ram_used, info.ram_total);

        sleep(2);
    }

    return 0;
}