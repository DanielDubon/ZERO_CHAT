#ifndef CLIENT_H
#define CLIENT_H

#include "WebSocket.h"
#include "Message.h"
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <functional>

class Client {
public:
    Client();  // Constructor por defecto
    Client(const std::string& host, int port, const std::string& username);
    ~Client();

    void connect(const std::string& uri);  // Conectar usando una URI
    void sendMessage(const std::string& recipient, const std::string& message);
    void setStatus(const std::string& status);  // Establecer el estado del usuario
    void listConnectedUsers();  // Listar usuarios conectados
    void requestHistoryPublic(); // Declaración
    bool isConnected() const;  // Verificar si está conectado
    void run();  // Añadir esta declaración

    void setUpdateUserListCallback(const std::function<void()>& callback){
        updateUserListCallback_ = callback;
    }

    // Nuevo método para obtener mensajes
    std::vector<Message> getMessages();
    
    // Nuevo método para añadir un mensaje
    void addMessage(const Message& message);
    
    // Getter para el nombre de usuario
    std::string getUsername() const { return username_; }
    
    // Getter para el estado
    std::string getStatus() const { return status_; }

    // Método para obtener la lista de usuarios conectados
    std::vector<std::pair<std::string, std::string>> getConnectedUsers() {
        std::lock_guard<std::mutex> lock(usersMutex_);
        return connectedUsers_;
    }

private:
    void handleIncomingMessage(const std::string& rawMsg);
    void displayPrompt() const;
    
    WebSocket ws_;
    std::function<void()> updateUserListCallback_;
    std::string username_;
    std::string status_;  // Estado del usuario
    std::string host_;    // Añadir estos
    int port_;           // miembros
    bool connected_;     // Variable para el estado de conexión
    
    std::vector<Message> messages_;  // Almacén de mensajes
    std::mutex messagesMutex_;       // Mutex para proteger el acceso a los mensajes
    
    std::vector<std::pair<std::string, std::string>> connectedUsers_;  // Lista de usuarios conectados (nombre, estado)
    std::mutex usersMutex_;  // Mutex para proteger el acceso a la lista de usuarios
};

#endif // CLIENT_H
