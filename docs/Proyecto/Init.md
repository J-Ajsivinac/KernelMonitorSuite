# 📚 Documentación del Proyecto

Este documento explica cómo configurar, compilar y ejecutar todas las partes del proyecto, incluyendo el kernel personalizado, el servidor de API en C y la aplicación frontend.

## 📋 Índice

1. [🐧 Compilación del Kernel](#compilación-del-kernel)
2. [🖥️ Servidor API en C](#servidor-api-en-c)
3. [🌐 Aplicación Frontend](#aplicación-frontend)
4. [🚀 Ejecución del Sistema Completo](#ejecución-del-sistema-completo)
5. [🔧 Solución de Problemas](#solución-de-problemas)

## 🐧 Compilación del Kernel

Este proyecto requiere un kernel Linux personalizado con syscalls específicas para su funcionamiento.

### ⚙️ Requisitos Previos

```bash
# Instalar dependencias para compilar el kernel
sudo apt-get update
sudo apt-get install build-essential libncurses-dev bison flex libssl-dev libelf-dev git
```

### 📝 Pasos para compilar el kernel

1. **📥 Obtener el código fuente del kernel**

```bash
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.12.17.tar.xz
tar xf linux-6.12.17.tar.xz
cd linux-6.12.17
```

2. **📁 Copiar los archivos de las syscalls personalizadas**

```bash
# Asegúrate de que estás en el directorio linux-6.12.17
# Copiar la definición de las syscalls
cp -r ruta/al/proyecto/kernel/usac/ kernel/
cp ruta/al/proyecto/arch/x86/entry/syscalls/syscall_64.tbl arch/x86/entry/syscalls/
cp ruta/al/proyecto/include/linux/syscalls_usac.h include/linux/
cp ruta/al/proyecto/include/linux/uts.h include/linux/
```

3. **⚙️ Configurar el kernel**

```bash
# Obtener la configuración actual (opcional)
cp /boot/config-$(uname -r) .config

# Configurar el kernel
make menuconfig
```

En el menú de configuración, asegúrate de:
- Habilitar el soporte para tus syscalls personalizadas (busca opciones relacionadas con "USAC" si hay alguna)
- Guardar la configuración y salir

4. **🔨 Compilar el kernel**

```bash
# Usar múltiples cores para acelerar la compilación 
# (reemplaza N con el número de cores disponibles)
make -j$(nproc)

# Instalar módulos
sudo make modules_install

# Instalar el kernel
sudo make install

# Actualizar GRUB
sudo update-grub
```

5. **🔄 Reiniciar el sistema con el nuevo kernel**

```bash
sudo reboot
```

6. **✅ Verificar que estás usando el kernel correcto**

```bash
uname -r
# Debería mostrar 6.12.17 o similar
```

## 🖥️ Servidor API en C

El servidor API en C maneja las comunicaciones entre el kernel y el frontend.

### 🔨 Compilación del servidor

```bash
cd c-api-server
make
```

La compilación generará un ejecutable llamado `server` en el directorio principal.

### ▶️ Ejecución del servidor

```bash
cd c-api-server
./server
```

Por defecto, el servidor se ejecuta en el puerto 8000. Para cambiar el puerto:

```bash
./server --port=8080
```

## 🌐 Aplicación Frontend

La aplicación frontend está construida con React y Vite.

### ⚙️ Requisitos Previos

- Node.js (v16 o superior)
- PNPM

```bash
# Instalar Node.js (si no está instalado)
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs

# Instalar PNPM
npm install -g pnpm
```

### 📦 Instalación de dependencias

```bash
cd frontend
pnpm install
```

### 🔧 Configuración

Antes de ejecutar la aplicación, asegúrate de configurar correctamente la conexión con el servidor API. Edita el archivo `src/configConnect.js`:

```javascript
// Ejemplo de configuración
export const API_URL = 'http://localhost:8000';
```

### 🚀 Ejecución en modo desarrollo

```bash
cd frontend
pnpm dev
```

La aplicación estará disponible en `http://localhost:5173` por defecto.

### 🏗️ Compilación para producción

```bash
cd frontend
pnpm build
```

Los archivos de producción se generarán en el directorio `dist`.

Para servir la aplicación compilada:

```bash
pnpm preview
```

## 🚀 Ejecución del Sistema Completo

Para que el sistema funcione correctamente, todos los componentes deben estar en ejecución:

1. **🐧 Asegúrate de estar usando el kernel personalizado**
   ```bash
   uname -r
   # Verifica que sea la versión 6.12.17
   ```

2. **🖥️ Inicia el servidor API**
   ```bash
   cd c-api-server
   ./server
   ```

3. **🌐 Inicia la aplicación frontend en otra terminal**
   ```bash
   cd frontend
   pnpm dev
   ```

4. **🌍 Accede a la aplicación**
   
   Abre tu navegador y navega a `http://localhost:5173`