#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>
#include "User.h"
#include <libwebsockets.h>
#include "Message.h"
#include <chrono>




class Server {
public:
    // Constructor: se le pasa el puerto en el que escuchará
    explicit Server(int port);
    ~Server();

    // Inicia el servidor (abre el socket, etc.)
    void start();

    // Detiene el servidor y libera recursos
    void stop();

    static int wsCallback(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len);

    bool isUserOnline(const std::string& username);

private:
    int port_;
    bool running_;
    struct lws_context *context_;
    std::vector<Message> history_;
    std::mutex historyMutex_;

    // Mapa para almacenar los usuarios registrados (clave: username)
    std::unordered_map<std::string, std::shared_ptr<User>> users_;
    std::mutex usersMutex_; // Para proteger el acceso concurrente al mapa

    // Agregar al mapa de usuarios
    std::unordered_map<std::string, struct lws *> connections_;

    // Función para manejar la comunicación con cada cliente (cada conexión se maneja en un thread)
    void handleClientData(struct lws *wsi, struct SessionData *session, 
                         const std::vector<uint8_t>& data);

    // Registra un usuario; retorna false si el nombre ya existe
    bool registerUser(const std::string& username, std::shared_ptr<User> user);

    // Elimina (desregistra) un usuario cuando se desconecta
    void unregisterUser(const std::string& username);

    // Envía la lista de usuarios conectados
    void sendUserList(struct lws *wsi);

    // Función auxiliar para enviar mensajes a través de WebSocket
    bool sendMessage(struct lws *wsi, const std::vector<uint8_t>& data);
};

// Forward declaration
struct SessionData{
    Server* server;
    std::string username;
    std::chrono::steady_clock::time_point lastActivity; // Nuevo campo para la última actividad

};

#endif // SERVER_H
