#!/bin/bash

# Directorios
KERNEL_DIR="/home/ajsivinac/Descargas/linux-6.12.17"
ENTREGA_DIR="/home/ajsivinac/Documentos/USAC/SO2/SO2-beta"
ARCHIVO_A_OMITIR="stvhu6Ck"

# Obtener archivos modificados y nuevos
cd "$KERNEL_DIR"
git diff --name-only --diff-filter=AM | grep -v "$ARCHIVO_A_OMITIR" > modified_files.txt
git ls-files --others --exclude-standard | grep -v "$ARCHIVO_A_OMITIR" > new_files.txt
cat modified_files.txt new_files.txt > all_files.txt

# Crear carpeta de entrega si no existe
mkdir -p "$ENTREGA_DIR"

# Copiar archivos modificados y nuevos
rsync -av --files-from=all_files.txt "$KERNEL_DIR" "$ENTREGA_DIR"

# Subir cambios al repo de entrega
cd "$ENTREGA_DIR"
git add .
git commit -m "Entrega de archivos modificados y nuevos"
git push

echo "Proceso completado. Archivos entregados en: $ENTREGA_DIR (omitiendo $ARCHIVO_A_OMITIR)"