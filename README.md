# ZERO_CHAT
Chat proyect using web sockets in C++ for OS 


# Dependencias y Configuración

Este proyecto usa C++ para el desarrollo del cliente y servidor de chat, con soporte de WebSockets. A continuación se listan las principales herramientas y librerías requeridas, junto a los pasos de instalación en Ubuntu/Debian (o sistemas similares).

1. Actualizar repositorios e instalar compilador de C++ y Make

Este proyecto está escrito en C++, por lo que se necesita un compilador (g++) y herramientas de construcción (make). En Ubuntu/Debian se pueden instalar con:


```sudo apt-get update```
```sudo apt-get install build-essential```


2. Instalar la librería de WebSockets

El chat utiliza libwebsockets para la comunicación en tiempo real mediante WebSockets. Para instalarla en Ubuntu/Debian:


```sudo apt-get install libwebsockets-dev```

3. (Opcional) Instalar CMake
Si prefieres compilar utilizando CMake (en lugar de los Makefiles incluidos o para configuraciones avanzadas), instala también:

```sudo apt-get install cmake```

4. Verificar instalación

```g++ -o main main.cpp```
```./main```