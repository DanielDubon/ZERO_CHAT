#include "WebSocket.h"
#include <iostream>

// Constructor
WebSocket::WebSocket()
    : connected_(false)
{
}

// Conectar a un servidor WebSocket
void WebSocket::connect(const std::string& url) {
    // Simulación de conexión a un servidor WebSocket
    std::cout << "Conectando a " << url << "...\n";
    connected_ = true;
    std::cout << "Conexión establecida.\n";
}

// Desconectar del servidor
void WebSocket::disconnect() {
    if (connected_) {
        std::cout << "Desconectando...\n";
        connected_ = false;
        std::cout << "Desconectado.\n";
    }
}

// Verificar si está conectado
bool WebSocket::isConnected() const {
    return connected_;
}

// Enviar un mensaje
void WebSocket::sendMessage(const std::string& message) {
    if (connected_) {
        std::cout << "Enviando mensaje: " << message << "\n";
    } else {
        std::cout << "Error: No conectado al servidor.\n";
    }
}

// Recibir un mensaje
std::string WebSocket::receiveMessage() {
    if (connected_) {
        std::string message;
        std::cout << "Esperando mensaje...\n";
        // Simulación de recepción de un mensaje
        std::getline(std::cin, message);
        return message;
    } else {
        std::cout << "Error: No conectado al servidor.\n";
        return "";
    }
}

// Callback para manejar mensajes entrantes
void WebSocket::setMessageHandler(std::function<void(const std::string&)> handler) {
    messageHandler_ = handler;
}