#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>
#include "User.h"
#include "WebSocket.h"

class Server {
public:
    // Constructor: se le pasa el puerto en el que escuchará
    explicit Server(int port);
    ~Server();

    // Inicia el servidor (abre el socket, etc.)
    void start();

    // Detiene el servidor y libera recursos
    void stop();

private:
    int port_;
    bool running_;

    // Mapa para almacenar los usuarios registrados (clave: username)
    std::unordered_map<std::string, std::shared_ptr<User>> users_;
    std::mutex usersMutex_; // Para proteger el acceso concurrente al mapa

    // Función que se encarga de aceptar conexiones (ejecutada en un thread separado)
    void acceptConnections();

    // Función para manejar la comunicación con cada cliente (cada conexión se maneja en un thread)
    void handleClient(std::shared_ptr<WebSocket> clientSocket);

    // Registra un usuario; retorna false si el nombre ya existe
    bool registerUser(const std::string& username, std::shared_ptr<User> user);

    // Elimina (desregistra) un usuario cuando se desconecta
    void unregisterUser(const std::string& username);
};

#endif // SERVER_H
