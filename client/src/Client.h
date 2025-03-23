#ifndef CLIENT_H
#define CLIENT_H

#include "WebSocket.h"
#include <string>
#include <iostream>

class Client {
public:
    Client();  // Constructor por defecto
    Client(const std::string& host, int port, const std::string& username);
    ~Client();

    void connect(const std::string& uri);  // Conectar usando una URI
    void sendMessage(const std::string& recipient, const std::string& message);
    void setStatus(const std::string& status);  // Establecer el estado del usuario
    void listConnectedUsers();  // Listar usuarios conectados
    bool isConnected() const;  // Verificar si está conectado
    void run();  // Añadir esta declaración

private:
    void handleIncomingMessage(const std::string& rawMsg);
    void displayPrompt() const;
    
    WebSocket ws_;
    std::string username_;
    std::string status_;  // Estado del usuario
    std::string host_;    // Añadir estos
    int port_;           // miembros
};

#endif // CLIENT_H
