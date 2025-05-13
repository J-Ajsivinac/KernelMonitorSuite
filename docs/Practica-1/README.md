# 🖥️ Manual de Práctica #1: Introducción al Kernel de Linux

## 📋 Información General
- **Universidad**: Universidad de San Carlos de Guatemala
- **Facultad**: Facultad de Ingeniería
- **Escuela**: Escuela de Ciencias y Sistemas
- **Estudiante**: Joab Israel Ajsivinac Ajsivinac
- **Carnet**: 202200135

---

## 🎯 Objetivos
- Aprender a compilar el kernel de Linux.
- Aprender a realizar modificaciones al kernel de Linux.
- Comprender cómo funcionan las llamadas al sistema en Linux.

---

## 📝 Descripción de la Práctica
En esta práctica, se realizaron modificaciones al kernel de Linux para la distribución **USAC Linux**, una versión ligera de Linux desarrollada por estudiantes de la Universidad de San Carlos de Guatemala. Las tareas principales incluyeron:
1. Configuración del entorno de desarrollo.
2. Descarga y modificación del kernel de Linux.
3. Implementación de llamadas al sistema personalizadas.
4. Documentación de los pasos seguidos y problemas encontrados.

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

## 📥 Descarga y Modificación del Kernel

### 📥 Descarga del Kernel
1. Se descargó la última versión **longterm** del kernel de Linux desde [kernel.org](https://www.kernel.org/).
   ```bash
   wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-6.12.17.tar.xz
   ```
2. Se extrajo el archivo descargado:
   ```bash
   tar -xvf linux-6.12.17.tar.xz
   ```

### ✏️ Modificaciones al Kernel
1. **Mensaje personalizado en el registro de inicio**:
   Se modificó el archivo `init/main.c` para mostrar un mensaje de bienvenida personalizado al iniciar el kernel.
   ```c
   // Modificación en init/main.c
    // mensaje custom
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣤⡀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣾⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⣹⣿⣿⣆⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿⣿⣶⠆⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣿⣿⣿⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣿⣿⡇⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣁⣤⣤⣴⣶⣶⣤⣤⣄⣀⠀⠀⠀⣸⣿⣿⣿⡇⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣦⣴⣿⣿⣿⣿⠁⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⡿⣿⣿⣿⣿⣿⣿⣿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⠟⠁⠀⠹⣿⣿⡟⣰⠋⠀⠀⠈⣿⣿⣿⣿⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⢼⣿⠀⠀⠀⢀⣿⣿⣇⣇⠀⠀⠀⠀⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣦⣀⣠⣾⣿⣿⣿⣿⣤⣀⣠⣾⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⠀⠀⢀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⣧⣀⣤⣴⣿⣿⠇⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣴⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⣀⣴⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⠀⠀⣠⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣤⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "⠀⢠⣾⣿⠟⠉⣸⣿⣿⣿⠟⠉⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣦⣄⡀⠀⠀⠀\n");
	printk(KERN_INFO "⢰⣿⡿⠁⠀⠀⣿⣿⡿⠁⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠀\n");
	printk(KERN_INFO "⣾⣿⠁⠀⠀⠀⣿⣿⣷⣿⣿⠿⠛⠉⣿⣿⣿⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⠛⠛⠻⠈⠙⠀\n");
	printk(KERN_INFO "⢻⣿⡀⠀⢠⣾⣿⣿⡟⠉⠀⠀⠀⠀⠹⣿⣿⣇⠈⠻⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣦⣤⣤⡾\n");
	printk(KERN_INFO "⠘⢿⡇⢠⡿⠋⢹⣿⡇⠀⠀⠀⠀⠀⠀⠹⣿⣿⡆⠀⠀⠉⠻⣿⣿⡿⠿⠿⣿⣿⣷⡉⠙⢿⣿⣟⠛⠉⠁⠀\n");
	printk(KERN_INFO "⠀⢘⡿⠀⠀⠀⠀⣿⡇⠀⠀⠀⠀⠀⠀⠀⠘⢿⣿⡄⠀⠀⠀⢹⣿⠀⠀⠀⠀⠈⠹⣷⠀⠀⠙⢿⣆⣀⣀⠀\n");
	printk(KERN_INFO "⠀⠈⠁⠀⠀⠀⠀⣿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡇⠀⠀⠀⠘⠿⣦⠀⠀⠀⠀⠀⠛⠀⠀⠀⠀⠉⠉⠁⠀\n");
	printk(KERN_INFO "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
	printk(KERN_INFO "\n");
	printk(KERN_INFO "   ___     _     _      _              \n");
	printk(KERN_INFO "  / _ |   (_)__ (_)  __(_)__  ___ _____\n");
	printk(KERN_INFO " / __ |  / (_-</ / |/ / / _ \\/ _ `/ __/\n");
	printk(KERN_INFO "/_/ |_|_/ /___/_/|___/_/_//_/\\_,_/\\__/ \n");
	printk(KERN_INFO "     |___/                              \n");
   ```

2. **Modificación del valor de UTS_SYSNAME**:
   Se modificó el archivo `include/linux/utsname.h` para cambiar el nombre del kernel a "USAC Linux".
   ```c
   // Modificación en include/linux/utsname.h
   #define UTS_SYSNAME "USAC Linux"
   ```

---

## 🛠️ Implementación de Llamadas al Sistema Personalizadas

### 📊 Llamada al Sistema 1: Tiempo de CPU Usado por un Proceso
1. **Descripción**: Esta llamada al sistema devuelve el tiempo de CPU usado por un proceso en milisegundos, dado su PID.
2. **Implementación**:
   - Se creó un nuevo archivo `syscalls_usac.h` en el directorio `include/linux/`.
   - Se modificó la tabla de llamadas al sistema en `arch/x86/entry/syscalls/syscall_64.tbl`.
   - Se implementó la función en `kernel/sys.c`.
   ```c
   // Código de la llamada al sistema en syscalls_usac.h
   asmlinkage long sys_get_cpu_time(pid_t pid);
   ```

### ⌨️ Llamada al Sistema 2: Escuchar una Tecla Específica
1. **Descripción**: Esta llamada al sistema espera hasta que se presione una tecla específica y realiza una acción.
2. **Implementación**:
   - Se modificó el archivo `syscalls_usac.h` para agregar la nueva llamada al sistema.
   - Se implementó la función en `kernel/sys.c`.
   ```c
   // Código de la llamada al sistema en syscalls_usac.h
   asmlinkage long sys_listen_key(char key);
   ```

### 🌡️ Llamada al Sistema 3: Temperatura de la CPU
1. **Descripción**: Esta llamada al sistema devuelve la temperatura de la CPU en grados Celsius.
2. **Implementación**:
   - Se modificó el archivo `syscalls_usac.h` para agregar la nueva llamada al sistema.
   - Se implementó la función en `kernel/sys.c`.
   ```c
   // Código de la llamada al sistema en syscalls_usac.h
   asmlinkage long sys_get_cpu_temp(void);
   ```

---

## 🚨 Problemas Encontrados y Soluciones

### 🛑 Problema 1: Error con la 3ra syscall
- **Descripción**: Al intentar crear una acción al momento de presionar la tecla indicada de la syscall 2, no se abria la terminal
- **Solución**: 

    - *Antes*
```c
static void open_terminal(void)
{
    char *argv[] = { "/usr/bin/x-terminal-emulator", "-e", "firefox", NULL };
    static char *envp[] = { 
        "HOME=/root", 
        "PATH=/sbin:/usr/sbin:/bin:/usr/bin", 
        "DISPLAY=:0", 
        "XAUTHORITY=/home/tu_usuario/.Xauthority", 
        NULL 
    };

    int action = call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
    if (action == 0) {
        printk(KERN_INFO "Programa lanzado correctamente: %d\n", action);
    } else {
        printk(KERN_ERR "Error al lanzar programa. Código de error: %d\n", action);
    }
}
```
Se vio la necesidad de agregar mas parametros    
    - *Solucuión aplicada*

```c
static void open_terminal(void)
{
    char *argv[] = { "/usr/bin/x-terminal-emulator", NULL };
    static char *envp[] = {
        "SHELL=/bin/bash",
        "HOME=/home/ajsivinac",
        "USER=ajsivinac",
        "PATH=/home/ajsivinac/bin:/home/ajsivinac/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/ajsivinac",
        "DISPLAY=:0",
        "PWD=/home/ajsivinac", 
        NULL};

    call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
}
```


### **2. Definición de `argv` (Argumentos para el programa)**  
```c
char *argv[] = { "/usr/bin/x-terminal-emulator", NULL };
```
- `argv` es un arreglo de cadenas que representa los argumentos del programa que queremos ejecutar.  
- `"/usr/bin/x-terminal-emulator"`: Es el ejecutable que queremos abrir (una terminal gráfica).  
- `NULL`: Indica el final del arreglo (obligatorio en `call_usermodehelper`).  

---

### **3. Definición de `envp` (Variables de entorno)**  
```c
static char *envp[] = {
    "SHELL=/bin/bash",
    "HOME=/home/ajsivinac",
    "USER=ajsivinac",
    "PATH=/home/ajsivinac/bin:/home/ajsivinac/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/ajsivinac",
    "DISPLAY=:0",
    "PWD=/home/ajsivinac", 
    NULL
};
```
#### **Explicación de cada variable:**  
- `"SHELL=/bin/bash"` → Especifica que la shell predeterminada para la terminal será `bash`.  
- `"HOME=/home/ajsivinac"` → Define la carpeta de inicio del usuario `ajsivinac`.  
- `"USER=ajsivinac"` → Define el usuario que ejecutará la terminal.  
- `"PATH=..."` → Lista de rutas donde buscar ejecutables (comandos).  
- `"DISPLAY=:0"` → Especifica el entorno gráfico X11 donde se ejecutará la terminal.  
- `"PWD=/home/ajsivinac"` → Define el directorio de trabajo inicial de la terminal.  
- `NULL` → Indica el final del arreglo (necesario en `call_usermodehelper`).  

---

### **4. Llamada a `call_usermodehelper()`**
```c
call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
```
- **`argv[0]` → `"/usr/bin/x-terminal-emulator"`**  
  → Es el programa que queremos ejecutar.  
- **`argv`**  
  → Es la lista de argumentos del programa (en este caso, solo el ejecutable).  
- **`envp`**  
  → Es la lista de variables de entorno que se usarán para ejecutar la terminal.  
- **`UMH_NO_WAIT`**  
  → Indica que el kernel no debe esperar a que el proceso termine; se ejecuta en segundo plano.  

### 🛑 Problema 2: Llamada al sistema no registrada
- **Descripción**: Al intentar probar una llamada al sistema, esta no estaba registrada en la tabla de syscalls.
- **Solución**: Se verificó la tabla de syscalls en `arch/x86/entry/syscalls/syscall_64.tbl` y se corrigió el número de la llamada al sistema.

### 🛑 Problema 3: Problema con la syscall por usar maquina virutal
- **Descripción**: El archivo donde se guarda la información de la temepratura, al usar maquina virtual no existe.
- **Solución**: Se modifico la logica para que cuando se este corriendo en maquina virtual se usen los ultimos 4 digitos del carnet como temepratura.

---

## 🏁 Conclusiones
En esta práctica se logró compilar y modificar el kernel de Linux, además de implementar llamadas al sistema personalizadas. Se documentaron los pasos seguidos y los problemas encontrados, lo que permitió comprender mejor el funcionamiento interno del kernel de Linux.
