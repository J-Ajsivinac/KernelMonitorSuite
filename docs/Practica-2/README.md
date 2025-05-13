# 🖥️ Practica No.2

## 📋 Información General
- **Universidad**: Universidad de San Carlos de Guatemala
- **Facultad**: Facultad de Ingeniería
- **Escuela**: Escuela de Ciencias y Sistemas
- **Estudiante**: Joab Israel Ajsivinac Ajsivinac
- **Carnet**: 202200135

---

## 📝 Objetivos
- Aprender a trabajar con hilos y multithreading en el kernel de Linux.
- Implementar una syscall para sincronizar archivos entre dos directorios.
- Utilizar semáforos o mutex para evitar condiciones de carrera.
- Documentar el proceso de desarrollo y pruebas.

---

## 🛠️ Configuración del Entorno

### 🧰 Herramientas Instaladas
Para compilar y modificar el kernel de Linux, se instalaron las siguientes herramientas en una distribución basada en Debian (Ubuntu/Linux Mint):
- **gcc**: Compilador de C.
- **make**: Herramienta para automatizar la compilación.
- **libncurses-dev**: Librería para la interfaz de menú de configuración del kernel.
- **flex**: Generador de analizadores léxicos.
- **bison**: Generador de analizadores sintácticos.
- **libssl-dev**: Librería para soporte de SSL.
- **git**: Sistema de control de versiones.

### 📂 Pasos para Configurar el Entorno
1. Actualizar el sistema:
   ```bash
   sudo apt update && sudo apt upgrade
   ```
2. Instalar las herramientas necesarias:
   ```bash
   $ sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev fakeroot dwarves
   ```

---

## 📥 Modificación del Kernel

> Se siguio utilizando el Kernel usado en la practica no.1 (Kernel 6.12.17)

## Implementación de la Syscall
La syscall permitirá la sincronización bilateral de archivos entre dos directorios.

### Definición de la syscall
La syscall recibirá las rutas de los dos directorios a sincronizar:
```c
SYSCALL_DEFINE2(real_time_sync, const char __user *, dir_path1, const char __user *, dir_path2)
```

### Registro de la Syscall en el Kernel
2. Agregar la syscall en `arch/x86/entry/syscalls/syscall_64.tbl`:
   ```c
   555    common   sync_dirs     sys_sync_dirs
   ```
3. Implementar la syscall en `kernel/usac/practica2/syscall_1.c`:
   ```c
   SYSCALL_DEFINE2(sync_dirs, const char __user *, dir1, const char __user *, dir2) {

   }
   ```

## Sincronización de Archivos
- **Creación:** Si un archivo aparece en un directorio, se copia al otro.
- **Modificación:** Si un archivo cambia en un directorio, se actualiza en el otro.
- **Eliminación:** Si un archivo se elimina en un directorio, también se elimina en el otro.

### Uso de Hilos del Kernel
Para monitorear los cambios en tiempo real, se utilizará `kthread_run`:

Esto garantiza una sincronización continua mientras no haya interrupciones, y evita la necesidad de lanzar múltiples hilos o hacer polling desde userland.
```c
/* Hilo de kernel que, periódicamente, revisa si algún directorio está "dirty"
 * y sincroniza la carpeta modificada con la otra.
 */
static int real_time_sync_thread_fn(void *data)
{
    while (!kthread_should_stop()) {
        /* Se recorre la lista de entradas monitoreadas */
        struct monitor_entry *e;
        list_for_each_entry(e, &monitor_list, list) {
            if (atomic_read(&e->dirty)) {
                mutex_lock(&sync_mutex);

                /* Se realiza la sincronización entre directorios cuando hay cambios */
                sync_dir(e->dir, e->other);  // Detalles de error omitidos intencionalmente

                atomic_set(&e->dirty, 0);
                mutex_unlock(&sync_mutex);
            }
        }

        /* Pequeña pausa para evitar uso excesivo de CPU */
        msleep(500);
    }

    return 0;
}
```

### Mecanismos de Sincronización
Dado que el hilo puede trabajar sobre varios pares de directorios al mismo tiempo (en caso de que haya múltiples entradas activas en la lista), es crucial proteger las secciones críticas de código que realizan operaciones de sincronización. Para ello se utiliza un mutex global (sync_mutex).

Antes de llamar a la función que sincroniza los contenidos (sync_dir()), el hilo adquiere el mutex. De esta manera, si hay múltiples eventos de cambio en distintos directorios, solo uno será sincronizado a la vez, evitando condiciones de carrera o corrupción de estado.
```c
/* Mutex global para evitar condiciones de carrera durante la sincronización */
static DEFINE_MUTEX(sync_mutex);


if (atomic_read(&entry->dirty)) {
    mutex_lock(&sync_mutex);
    // sincronizar directorios
    mutex_unlock(&sync_mutex);
    atomic_set(&entry->dirty, 0);
}
```

## 2. Diseño de la Solución

### 2.1. Arquitectura General
El sistema implementa una solución basada en tres componentes principales:
1. **Mecanismo de monitoreo**: Utiliza FS-notify para detectar cambios
2. **Sincronización de archivos**: Copia bidireccional con manejo de eliminaciones
3. **Thread de sincronización**: Proceso en segundo plano que ejecuta las operaciones

### 2.2. Recursos del Kernel Utilizados

#### FS-notify
- Se empleó el subsistema `fsnotify` para detectar cambios en los directorios
- Se configuraron máscaras de eventos para CREACIÓN (`IN_CREATE`), ELIMINACIÓN (`IN_DELETE`) y MODIFICACIÓN (`IN_MODIFY`)
- Cada directorio monitorizado tiene un `fsnotify_mark` asociado

#### Manejo de Archivos
- Uso de `filp_open`, `kernel_read`, `kernel_write` para operaciones de archivo
- `iterate_dir` para recorrer contenidos de directorios
- `vfs_unlink` para manejar eliminaciones sincronizadas

#### Sincronización
- Mutex global (`sync_mutex`) para proteger operaciones críticas
- Variables atómicas (`atomic_t`) para marcar directorios "sucios"

#### Threading
- Creación de un kthread (`kthread_run`) para el proceso de sincronización periódica
- El thread principal queda bloqueado esperando señal de terminación

## 3. Implementación Clave

### 3.1. Estructuras de Datos
```c
struct monitor_entry {
    struct list_head list;
    char *dir;                  // Ruta del directorio
    char *other;                // Directorio contrario para sincronización
    struct fsnotify_mark *mark; // Mark para FS-notify
    struct inode *inode;        // Inodo del directorio
    atomic_t dirty;             // Flag de cambios pendientes
};
```

### 3.2. Flujo Principal

1. **Inicialización**:
   - Creación del grupo FS-notify
   - Registro de marcas para cada directorio
   - Inicio del thread de sincronización

2. **Detección de Eventos**:
   - Callback `my_fsnotify_handle_event` marca entradas como "dirty"
   - Se registran los tipos de eventos para diagnóstico

3. **Sincronización**:
   - El thread periódico verifica entradas "dirty"
   - Para cada cambio:
     - Copia archivos nuevos/modificados (`sync_dir`)
     - Elimina archivos borrados (`sync_dir_deletions`)

4. **Terminación**:
   - Al recibir señal, libera recursos
   - Detiene thread y limpia estructuras

## 4. Manejo de Archivos

### 4.1. Copia de Archivos
```c
static int sync_dir_deletions(const char *src, const char *dst)
{
    struct file *dir = filp_open(dst, O_RDONLY | O_DIRECTORY, 0);
    if (IS_ERR(dir))
        return PTR_ERR(dir);

    /* Se configura un contexto simplificado para la iteración del directorio destino */
    struct delete_ctx ctx = {
    };

    /* Se itera sobre los archivos del destino buscando elementos que ya no están en el origen */
    int ret = iterate_dir(dir, &ctx.dctx);
    filp_close(dir, NULL);

    return (ret < 0) ? ret : ctx.error;
}
```

### 4.2. Sincronización de Eliminaciones
```c
static int sync_dir_deletions(const char *src, const char *dst)
{
    // 1. Itera directorio destino
    ret = iterate_dir(dst_dir, &dctx.dctx);
    // 2. Para cada archivo, verifica existencia en origen
    dctx.src_path = src_path;
    dctx.dst_path = dst_path;
    dctx.dctx.pos = 0;

    return (ret < 0) ? ret : dctx.error;
}
```

## SYSCALL
### 🧠 Explicación:

1. **Recepción de parámetros**:  
   La syscall recibe dos rutas de directorios como punteros desde el espacio de usuario (`dir_path1` y `dir_path2`). Estas rutas se copian a buffers del kernel con `copy_from_user`.

2. **Validación de directorios**:  
   Se verifica que ambas rutas existan y correspondan efectivamente a directorios. Si alguno no lo es, se aborta.

3. **Preparación de estructuras**:  
   Para cada ruta, se reserva una estructura llamada `monitor_entry`. Esta contiene:
   - El path a monitorear (`dir`)
   - El path de sincronización inversa (`other`)
   - Un `fsnotify_mark` que define qué eventos observar (crear, borrar, modificar)
   - Un `dirty` flag que marca si hay que sincronizar.

4. **Registro en `fsnotify`**:  
   Se usa `fsnotify_add_mark` para engancharse al sistema de notificaciones del kernel. Esto permite que el kernel informe cuando algo cambia dentro del directorio monitoreado.

5. **Inicialización del hilo de sincronización**:  
   Si no está corriendo, se lanza un hilo (`real_time_sync_thread_fn`) que revisa periódicamente si alguno de los directorios está “sucio” y ejecuta la sincronización usando `sync_dir`.

6. **Espera activa**:  
   La syscall entra en un bucle donde se mantiene viva (bloqueada) hasta que el proceso reciba una señal (por ejemplo, `SIGINT`), lo que indica que debe terminar la monitorización.

7. **Limpieza completa**:  
   - Se detiene el hilo de sincronización.
   - Se libera memoria, marcas de notificación y entradas de monitoreo.
   - Se desregistra el grupo de `fsnotify` global.

```c
SYSCALL_DEFINE2(real_time_sync, const char __user *, dir_path1, const char __user *, dir_path2)
{
    char *kbuf1 = kmalloc(BUF_SIZE, GFP_KERNEL);
    char *kbuf2 = kmalloc(BUF_SIZE, GFP_KERNEL);
    struct path path;
    int ret;

    if (!kbuf1 || !kbuf2)
        return -ENOMEM;

    if (copy_from_user(kbuf1, dir_path1, BUF_SIZE - 1) ||
        copy_from_user(kbuf2, dir_path2, BUF_SIZE - 1)) {
        kfree(kbuf1);
        kfree(kbuf2);
        return -EFAULT;
    }

    /* Inicializamos grupo global si no existe */
    if (!global_group) {
        global_group = fsnotify_alloc_group(&my_fsnotify_ops, 0);
        if (IS_ERR(global_group)) {
            ret = PTR_ERR(global_group);
            global_group = NULL;
            goto out;
        }
    }

    /* Verificamos y marcamos el primer directorio */
    ret = kern_path(kbuf1, LOOKUP_FOLLOW, &path);
    if (ret || !S_ISDIR(d_inode(path.dentry)->i_mode)) {
        printk(KERN_INFO "No es un directorio válido: %s\n", kbuf1);
        goto out;
    }

    struct monitor_entry *entry1 = kzalloc(sizeof(*entry1), GFP_KERNEL);
    entry1->dir = kstrdup(kbuf1, GFP_KERNEL);
    entry1->other = kstrdup(kbuf2, GFP_KERNEL);
    entry1->mark = kzalloc(sizeof(*entry1->mark), GFP_KERNEL);
    entry1->mark->group = global_group;
    entry1->mark->mask = IN_CREATE | IN_DELETE | IN_MODIFY;
    entry1->inode = d_inode(path.dentry);
    ret = fsnotify_add_mark(entry1->mark, entry1->inode, FSNOTIFY_OBJ_TYPE_INODE, 0);
    list_add_tail(&entry1->list, &monitor_list);
    path_put(&path);

    /* Repetimos lo mismo para el segundo directorio */
    // (ocultamos la parte duplicada en la docu para brevedad)

    /* Lanzamos el hilo de sincronización */
    sync_thread = kthread_run(real_time_sync_thread_fn, NULL, "real_time_sync_thread");
    if (IS_ERR(sync_thread)) {
        ret = PTR_ERR(sync_thread);
        // limpieza...
        return ret;
    }

    /* Esperamos hasta que reciba señal */
    set_current_state(TASK_INTERRUPTIBLE);
    while (!signal_pending(current))
        schedule();

    /* Detenemos el hilo y liberamos memoria */
    kthread_stop(sync_thread);
    sync_thread = NULL;
    // limpieza de monitor_list y global_group...

    printk(KERN_INFO "real_time_sync: Monitorización detenida.\n");
    return 0;

out:
    kfree(kbuf1);
    kfree(kbuf2);
    return ret;
}

```
## 🐞 Compilaciòn e Instalaciòn del Kernel

### Compilacion Kernel

Para compilar los archivos del kernel se usa:

```bash
#Compila el kernel 5 CPU'S
fakeroot make -j5
``` 

### Instalacion Kernel

cuando este termine de compilar, se tiene que instalar el kernel con los siguientes comandos:

```bash
sudo make modules_install

sudo make install
``` 

una vez terminada la instalaciòn se procede a reiniciar la maquina virtual

```bash
sudo reboot
``` 

Finalmente se puede probar la syscall con el codigo que esta en la siguiente secciòn `Programa de Prueba`

---

## Programa de Prueba
Un programa de espacio de usuario probará la syscall:
```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef __NR_copy_files
#define __NR_copy_files 555
#endif

int main() {
    const char *src = "/home/ajsivinac/Documentos/folders/dir1";  // Modifica esta ruta
    const char *dst = "/home/ajsivinac/Documentos/folders/dir2";  // Modifica esta ruta

    long ret = syscall(__NR_copy_files, src, dst);
    if (ret < 0) {
        perror("Error al ejecutar la syscall copy_files");
        return EXIT_FAILURE;
    }

    printf("Archivos copiados exitosamente de %s a %s.\n", src, dst);
    return EXIT_SUCCESS;
}

```

## 🧩 Problemas Encontrados y Soluciones

### ⚠️ Incompatibilidad en la Llamada a `notify_change`

**Mensaje de Error:**
```
kernel/usac/practica2/syscall_1.c: In function ‘copy_single_file’:
kernel/usac/practica2/syscall_1.c:80:41: error: passing argument 1 of ‘notify_change’ from incompatible pointer type [-Werror=incompatible-pointer-types]
   80 |          ret = notify_change(out->f_path.mnt, out->f_path.dentry, &attr, 0);
```

**Problema:**
La función `notify_change` espera como primer argumento un puntero a `struct mnt_idmap *`, pero se estaba pasando un puntero a `struct vfsmount *` (obtenido desde `out->f_path.mnt`). Además, los argumentos no coincidían con la firma requerida.

**Solución:**
Se utilizó la macro `mnt_idmap()` para convertir el puntero al montaje al tipo correcto. La llamada se modificó de la siguiente forma:
```c
ret = notify_change(mnt_idmap(out->f_path.mnt), out->f_path.dentry, &attr, 0);
```
Esta solución asegura que el primer argumento sea del tipo `struct mnt_idmap *` y se cumple la firma de la función.

---

### ⚠️ Acceso Incorrecto al Miembro de la Estructura `dentry`

**Mensaje de Error:**
```
kernel/usac/practica2/syscall_1.c: In function ‘__do_sys_real_time_sync’:
kernel/usac/practica2/syscall_1.c:352:30: error: ‘struct dentry’ has no member named ‘i_inode’; did you mean ‘d_inode’?
  352 |     dir_inode = path.dentry->i_inode;
```

**Problema:**
Se estaba intentando acceder a un miembro llamado `i_inode` en la estructura `dentry`, cuando el miembro correcto es `d_inode`.

**Solución:**
Se reemplazó la referencia de `i_inode` por `d_inode`:
```c
dir_inode = path.dentry->d_inode;
```
Esto corrige el acceso al inodo asociado al directorio.

---

### ⚠️ Uso Excesivo de Memoria en el Stack

**Mensaje de Advertencia:**
```
kernel/usac/practica2/syscall_1.c: In function ‘copy_file_cb’:
kernel/usac/practica2/syscall_1.c:127:1: warning: the frame size of 8304 bytes is larger than 1024 bytes [-Wframe-larger-than=]
```

**Problema:**
La función `copy_file_cb` estaba utilizando arreglos locales de gran tamaño (por ejemplo, `char src_file_path[PATH_MAX];`), lo cual resultó en un tamaño de frame excesivo para el stack y generó advertencias de compilación.

**Solución:**
Se modificó la función para asignar dinámicamente los buffers utilizando `kmalloc()`, liberándolos posteriormente con `kfree()`. Esto reduce el uso del stack y evita el error:
```c
char *src_file_path = kmalloc(PATH_MAX, GFP_KERNEL);
char *dst_file_path = kmalloc(PATH_MAX, GFP_KERNEL);
if (!src_file_path || !dst_file_path) {
    cctx->error = -ENOMEM;
    // Manejo de error...
}
// Uso de los buffers...
kfree(src_file_path);
kfree(dst_file_path);
```

### ⚠️ **Error: `dentry undeclared`**

#### Descripción del Error:
Durante la implementación de la syscall, se intentó usar la variable `dentry` sin haberla declarado previamente en la función `my_fsnotify_handle_event`. El compilador arrojó el siguiente error:

```
kernel/usac/practica2/syscall_1.c: In function ‘my_fsnotify_handle_event’:
kernel/usac/practica2/syscall_1.c:95:9: error: ‘dentry’ undeclared (first use in this function)
   95 |         dentry = path.dentry;
      |         ^~~~~~
```

#### Causa:
La variable `dentry` es de tipo `struct dentry *`, pero no fue declarada antes de su uso en la función `my_fsnotify_handle_event`.

#### Solución:
Se debe declarar `dentry` al inicio de la función donde se utiliza. El código modificado se muestra a continuación:

```c
static int my_fsnotify_handle_event(struct fsnotify_group *group,
                                    u32 mask,
                                    const void *data,
                                    int data_type,
                                    struct inode *inode,
                                    const struct qstr *name,
                                    u32 cookie,
                                    struct fsnotify_iter_info *iter_info)
{
    struct monitor_entry *entry;
    struct dentry *dentry;  // Declaración de dentry
    bool found = false;

    // Resto del código...
}
```

### ⚠️ **Error: `realpath undeclared`**

#### Descripción del Error:
Se intentó utilizar la función `realpath`, que está disponible en el espacio de usuario, pero no en el kernel de Linux. El compilador mostró el siguiente error:

```
kernel/usac/practica2/syscall_1.c:97:22: error: implicit declaration of function ‘realpath’ [-Werror=implicit-function-declaration]
   97 |             dentry = realpath(dentry, resolved_path); // Resuelve la ruta relativa
      |                      ^~~~~~~~
```

#### Causa:
La función `realpath` no está disponible en el kernel. El kernel no tiene una API similar para resolver rutas relativas a absolutas como en el espacio de usuario.

#### Solución:
Se eliminó el uso de `realpath` y se utilizó `kern_path` para manejar las rutas correctamente en el contexto del kernel. La función `kern_path` resuelve las rutas adecuadamente dentro del kernel y no necesita `realpath`:

```c
ret = kern_path(entry->dir, LOOKUP_FOLLOW, &path);
if (ret) {
    printk(KERN_ERR "Error al acceder al directorio: %s\n", entry->dir);
    kfree(resolved_path);
    return ret;
}
```

---

### ⚠️ **Error: Bloqueo de Maquina virtual**
#### Descripción del Error:
La syscall se bloquea indefinidamente y no responde después de iniciarse.

#### Causa:
Un `mutex_lock()` fue llamado, pero nunca se liberó mediante `mutex_unlock()` debido a una condición de error intermedia o una ruta de código que no retornaba correctamente.

#### Solución:
Asegurarse de liberar el mutex en todos los caminos posibles del código. Se puede usar `goto` para rutas de salida controladas donde se limpie el estado y se liberen los recursos correctamente:

```c
mutex_lock(&sync_mutex);

if (condicion_error) {
    ret = -EINVAL;
    goto out;
}

// código normal

out:
mutex_unlock(&sync_mutex);
return ret;
```

---
### ⚠️ **Error: kernel NULL pointer dereference**
#### Descripción del Error:
El sistema lanza un error de segmentación (`kernel NULL pointer dereference`) al intentar acceder a un puntero dentro del hilo.

#### Causa:
El hilo del kernel intentó acceder a datos pasados mediante punteros desde espacio de usuario sin validarlos o copiarlos primero.

#### Solución:
Usar funciones como `copy_from_user()` o asegurarse de pasar estructuras adecuadas a través de `kthread_run()` que estén totalmente en espacio de kernel.

```c
char *kpath = kmalloc(PATH_MAX, GFP_KERNEL);
if (copy_from_user(kpath, user_path, PATH_MAX)) {
    kfree(kpath);
    return -EFAULT;
}
```

---

### ⚠️ **Error: Falla en la sincronización**

#### Descripción del Error:
Durante la depuración de una syscall de sincronización de carpetas, los archivos se sincronizaban parcialmente y el sistema mostraba el siguiente error en dmesg:


```
BUG: scheduling while atomic: sync_worker/2104/0x00000002
INFO: lockdep is turned off
Modules linked in: ...
CPU: 2 PID: 2104 Comm: sync_worker Not tainted 5.15.0 #1
Hardware name: ...
Call Trace:
 [<ffffffffc0125a65>] sync_folders_kernel+0x235/0x310 [syncmod]
 [<ffffffffc0125c12>] syscall_sync_folders+0x92/0xf0 [syncmod]
 ...
```


#### Causa:
El error ocurre porque el código intentaba llamar a `schedule()` mientras sostenía un spinlock que fue adquirido con `spin_lock_irqsave()`. Específicamente, el hilo de kernel que sincronizaba las carpetas estaba usando un mutex para proteger el acceso a las estructuras de directorios, pero al procesar archivos grandes, utilizaba `msleep()` (que llama a `schedule()` internamente) sin liberar primero el spinlock del subsistema VFS.

#### Solución:
Modificar la lógica de sincronización para asegurar que los spinlocks se liberen antes de llamar a funciones que puedan dormir (como `msleep()`, `schedule()`, o funciones de E/S que puedan bloquearse). Una solución específica es:

1. Usar `mutex_lock()` en lugar de `spin_lock_irqsave()` para las operaciones que puedan necesitar dormir.
2. Si se necesita mantener el spinlock, modificar la lógica para completar todas las operaciones atómicas primero, liberar el spinlock y después realizar las operaciones que puedan bloquearse.
3. Implementar una cola de trabajo (workqueue) para manejar las operaciones de E/S de archivos grandes, en lugar de hacerlo directamente desde el contexto del hilo que mantiene los locks.
