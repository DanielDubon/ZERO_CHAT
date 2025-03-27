# ZERO_CHAT
Chat project using web sockets in C++ for OS 

## Dependencias y Configuración

Este proyecto usa C++ para el desarrollo del cliente y servidor de chat, con soporte de WebSockets. A continuación se listan las principales herramientas y librerías requeridas, junto a los pasos de instalación en Ubuntu/Debian (o sistemas similares).

### 1. Actualizar repositorios e instalar compilador de C++ y Make

Este proyecto está escrito en C++, por lo que se necesita un compilador (g++) y herramientas de construcción (make). En Ubuntu/Debian se pueden instalar con:

```bash
sudo apt-get update
sudo apt-get install build-essential
```

### 2. Instalar la librería de WebSockets

El chat utiliza libwebsockets para la comunicación en tiempo real mediante WebSockets. Para instalarla en Ubuntu/Debian:

```bash
sudo apt-get install libwebsockets-dev
```

### 3. (Opcional) Instalar CMake
Si prefieres compilar utilizando CMake (en lugar de los Makefiles incluidos o para configuraciones avanzadas), instala también:

```bash
sudo apt-get install cmake
```

### 4. Verificar instalación

```bash
g++ -o main main.cpp
./main
```

## Dependencias externas

Este proyecto utiliza las siguientes bibliotecas externas como submódulos de Git:
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - Biblioteca HTTP para C++
- [nlohmann/json](https://github.com/nlohmann/json) - Biblioteca JSON para C++

## Clonar el repositorio

### Opción 1: Clonar con submódulos (recomendado)

Para clonar este repositorio incluyendo todos los submódulos, usa:

```bash
git clone --recurse-submodules https://github.com/tu-usuario/ZERO_CHAT.git
cd ZERO_CHAT
```

### Opción 2: Clonar y luego inicializar submódulos

Si ya has clonado el repositorio sin los submódulos, puedes obtenerlos con:

```bash
git submodule init
git submodule update
```

## Configuración de submódulos (solo para administradores)

Si no se incluyeron las librerias de external/json/ y external/cpp-httlib/ , sigue estos pasos:

```bash

# 1. Eliminar las carpetas del índice de Git (si existen)
git rm -r --cached external/cpp-httplib
git rm -r --cached external/json

# 2. Eliminar las carpetas físicas (opcional)
rm -rf external/cpp-httplib
rm -rf external/json

# 3.  Desde la carpeta raíz, añadir los submódulos
git submodule add https://github.com/yhirose/cpp-httplib.git external/cpp-httplib
git submodule add https://github.com/nlohmann/json.git external/json

# 4. Inicializar y actualizar los submódulos
git submodule init
git submodule update

# 5. Verificar que los archivos estén disponibles
ls -la external/cpp-httplib/httplib.h
ls -la external/json/single_include/nlohmann/json.hpp


```

## Ejecución

Para ejecutar el cliente:
```bash
./client <host> <port> <username> <web_port>
```

Ejemplo:
```bash
./client localhost 8080 Usuario 8081
```

Luego, abre tu navegador y visita:
```
http://localhost:8081
```

