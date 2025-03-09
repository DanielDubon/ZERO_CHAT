#!/bin/bash

# start_server.sh
# Uso: ./start_server.sh [puerto]
# Si no se pasa ningún puerto, usará 8080 por defecto

PORT=${1:-8080}

echo "Compilando el servidor..."
cd "$(dirname "$0")/../server" || exit 1
make

echo "Iniciando el servidor en el puerto $PORT..."
./src/server "$PORT"
