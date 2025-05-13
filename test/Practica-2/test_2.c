
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef __NR_copy_files
#define __NR_copy_files 555
#endif

// Directorios a monitorear
#define DIR_TO_MONITOR1 "/home/ajsivinac/Documentos/folders/dir1"
#define DIR_TO_MONITOR2 "/home/ajsivinac/Documentos/folders/dir2"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <directorio_origen> <directorio_destino>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *src = argv[1];
    const char *dst = argv[2];
    
    long ret = syscall(__NR_copy_files, src, dst);
    if (ret < 0) {
        perror("Error al ejecutar la syscall copy_files");
        return EXIT_FAILURE;
    }
    
    printf("Archivos copiados exitosamente.\n");
    return EXIT_SUCCESS;
}
