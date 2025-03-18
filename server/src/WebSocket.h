#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <vector>
#include <functional>  // Para std::function

class WebSocket {
public:
    // Constructor
    WebSocket();

    // Métodos para manejar conexiones
    void connect(const std::string& url);  // Conectar a un servidor WebSocket
    void disconnect();                     // Desconectar del servidor
    bool isConnected() const;              // Verificar si está conectado

    // Métodos para enviar y recibir mensajes
    void sendMessage(const std::string& message);  // Enviar un mensaje
    std::string receiveMessage();                  // Recibir un mensaje

    // Callback para manejar mensajes entrantes
    void setMessageHandler(std::function<void(const std::string&)> handler);

private:
    bool connected_;  // Estado de la conexión
    std::function<void(const std::string&)> messageHandler_;  // Callback para mensajes
};

#endif // WEBSOCKET_H