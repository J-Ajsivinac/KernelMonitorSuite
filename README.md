<h1 align="center">Sistemas Operativos 2</h1>

<p align="center"></p>
<div align="center"> 🏛 Universidad San Carlos de Guatemala</div>
<div align="center"> 📆 Primer Semestre 2025</div>
<div align="center"> 🧑‍💼 Joab Israel Ajsivinac Ajsivinac | 202200135 </div>
<br/> 

<br/>

> [!NOTE]  
> **Practica 1**
>
> <div align="center" style="display:flex;justify-content:center;gap:20px"><img src="https://go-skill-icons.vercel.app/api/icons?i=linux,c" /></div>
> 
> * Linux
> * c
> 
> [👁 Ver Syscalls](https://github.com/J-Ajsivinac/KernelMonitorSuite/tree/main/linux-6.12.17/kernel/usac/practica1)

<br>

---


> [!NOTE]  
> **Practica 2**
>
> <div align="center" style="display:flex;justify-content:center;gap:20px"><img src="https://go-skill-icons.vercel.app/api/icons?i=linux,c" /></div>
> 
> * Linux
> * c
> 
> [👁 Ver Syscalls](https://github.com/J-Ajsivinac/KernelMonitorSuite/tree/main/linux-6.12.17/kernel/usac/practica2)

<br>

---


> [!NOTE]  
> **Proyecto**
>
> <div align="center" style="display:flex;justify-content:center;gap:20px"><img src="https://go-skill-icons.vercel.app/api/icons?i=linux,c,react,tailwindcss,mongoose," /></div>
> 
> * Linux
> * C
> * React
> * Tailwindcss
> * Mongoose
>
> [👁 Ver Syscalls](https://github.com/J-Ajsivinac/KernelMonitorSuite/tree/main/linux-6.12.17/kernel/usac/Proyecto)
>
> [🗄️ Ver Server](https://github.com/J-Ajsivinac/KernelMonitorSuite/tree/main/src/c-api-server)
>
> [🖥️ Ver Frontend](https://github.com/J-Ajsivinac/KernelMonitorSuite/tree/main/src/frontend)

---

# Extra

## 📄Script de Entrega `update.sh`

### 🧾 Descripción General

Este script automatiza el proceso de recolección, copia y entrega de archivos **modificados o nuevos** dentro del árbol de código fuente del kernel Linux. Excluye un archivo específico, y luego sube estos archivos a un repositorio de entrega.

---

### 📁 Estructura de Directorios

* `KERNEL_DIR`: Ruta del directorio raíz del código fuente del kernel.
* `ENTREGA_DIR`: Ruta del directorio donde se copiarán los archivos para entrega.
* `ARCHIVO_A_OMITIR`: Nombre del archivo que debe ser excluido del proceso.

---

### ⚙️ Pasos que Realiza el Script

1. **Cambiar al directorio del kernel**:

   ```bash
   cd "$KERNEL_DIR"
   ```

2. **Listar archivos modificados y nuevos (sin incluir el archivo a omitir)**:

   * `git diff --name-only --diff-filter=AM`: Lista archivos modificados (`M`) o añadidos (`A`).
   * `git ls-files --others --exclude-standard`: Lista archivos nuevos no versionados.
   * Ambos comandos filtran los archivos que coincidan con el nombre `ARCHIVO_A_OMITIR`.

3. **Unir ambas listas en un solo archivo `all_files.txt`**:

   ```bash
   cat modified_files.txt new_files.txt > all_files.txt
   ```

4. **Crear el directorio de entrega si no existe**:

   ```bash
   mkdir -p "$ENTREGA_DIR"
   ```

5. **Copiar los archivos listados a la carpeta de entrega usando `rsync`**:

   ```bash
   rsync -av --files-from=all_files.txt "$KERNEL_DIR" "$ENTREGA_DIR"
   ```

6. **Hacer commit y push al repositorio de entrega**:

   ```bash
   cd "$ENTREGA_DIR"
   git add .
   git commit -m "Entrega de archivos modificados y nuevos"
   git push
   ```

7. **Mensaje final de confirmación**:

   ```bash
   echo "Proceso completado. Archivos entregados en: $ENTREGA_DIR (omitiendo $ARCHIVO_A_OMITIR)"
   ```

---

### 📌 Requisitos Previos

* Tener `git` instalado y configurado en ambos directorios (`KERNEL_DIR` y `ENTREGA_DIR`).
* El directorio `KERNEL_DIR` debe estar inicializado como un repositorio Git.
* El directorio `ENTREGA_DIR` debe ser un clon válido de un repositorio Git accesible para `push`.
* Tener permisos de lectura en `KERNEL_DIR` y escritura en `ENTREGA_DIR`.
* Tener configurado el acceso remoto al repositorio de entrega (`git remote`).

---

### 📝 Ejemplo de Uso

```bash
./update.sh
```

> Se recomienda revisar que `ARCHIVO_A_OMITIR` sea un nombre exacto de archivo (sin ruta) y que efectivamente se quiera excluir.
