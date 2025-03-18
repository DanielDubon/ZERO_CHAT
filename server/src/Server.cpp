#include "Protocol.h"
#include "Server.h"
#include <vector>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>

void handleClientData(const std::vector<uint8_t>& data) {
    uint8_t code;
    std::vector<std::vector<uint8_t>> fields;
    if (!Protocol::deserializeMessage(data, code, fields)) {
        std::cerr << "Error: Mensaje mal formado." << std::endl;
        return;
    }
    
    // Procesar el mensaje según su código.
    switch(code) {
        case 1:
            // Código 1: Listar usuarios
            std::cout << "Solicitud de listado de usuarios." << std::endl;
            // Lógica para enviar la lista de usuarios...
            break;
        case 4:
            // Código 4: Mandar un mensaje
            if(fields.size() >= 2) {
                std::string destinatario = Protocol::bytesToString(fields[0]);
                std::string mensaje = Protocol::bytesToString(fields[1]);
                std::cout << "Mensaje de " << destinatario << ": " << mensaje << std::endl;
                // Lógica para reenviar el mensaje...
            }
            break;
        // Otros casos según lo definido en el protocolo.
        default:
            std::cerr << "Código desconocido: " << static_cast<int>(code) << std::endl;
            break;
    }
}


Server::Server(int port)
    : port_(port), running_(false) {
}

Server::~Server() {
    stop();
}

void Server::start() {
    running_ = true;
    // Aquí se abriría el socket de escucha en una implementación real.
    std::cout << "Servidor iniciado en el puerto " << port_ << std::endl;
    // Lanzamos un thread para aceptar conexiones
    std::thread acceptThread(&Server::acceptConnections, this);
    acceptThread.detach();
}

void Server::stop() {
    running_ = false;
    // Aquí se cerrarían el socket de escucha y otros recursos.
    std::cout << "Servidor detenido." << std::endl;
}

void Server::acceptConnections() {
    // En una implementación real, aquí se aceptan conexiones desde el socket.
    // En este ejemplo simulamos la aceptación de conexiones cada 5 segundos.
    while (running_) {
        // Creamos un nuevo objeto WebSocket para simular una conexión de cliente.
        std::shared_ptr<WebSocket> clientSocket = std::make_shared<WebSocket>();
        clientSocket->connect("ws://localhost:" + std::to_string(port_));

        // Lanzamos un thread para manejar la comunicación con este cliente.
        std::thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();

        // Esperamos un poco antes de aceptar otra conexión (simulación)
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void Server::handleClient(std::shared_ptr<WebSocket> clientSocket) {
    // Se asume que el primer mensaje recibido es el mensaje de registro.
    // En nuestro protocolo, suponemos que el código 1 indica "registro de usuario"
    std::string rawMessage = clientSocket->receiveMessage();
    if (rawMessage.empty()) {
        std::cerr << "No se recibió mensaje de registro." << std::endl;
        return;
    }

    // Convertimos el mensaje (string) a un vector de bytes
    std::vector<uint8_t> data = Protocol::stringToBytes(rawMessage);

    uint8_t code;
    std::vector<std::vector<uint8_t>> fields;
    if (!Protocol::deserializeMessage(data, code, fields)) {
        std::cerr << "Error: Mensaje mal formado recibido." << std::endl;
        return;
    }

    // Validamos que el mensaje sea de registro y que contenga al menos el username
    if (code != 1 || fields.empty()) {
        std::cerr << "Error: Mensaje de registro inválido." << std::endl;
        return;
    }

    std::string username = Protocol::bytesToString(fields[0]);

    // Para efectos de este ejemplo, usamos una IP simulada.
    std::string ipAddress = "127.0.0.1";

    // Creamos un objeto User (la clase User ya está implementada en User.h/User.cpp)
    auto user = std::make_shared<User>(username, ipAddress);

    // Intentamos registrar el usuario; si ya existe, se rechaza la conexión.
    if (!registerUser(username, user)) {
        std::cout << "El usuario '" << username << "' ya está registrado." << std::endl;
        // Aquí se podría enviar un mensaje de error al cliente.
        return;
    }
    std::cout << "Usuario registrado: " << username << std::endl;

    // Bucle principal para manejar la comunicación con el cliente
    while (running_ && clientSocket->isConnected()) {
        std::string incoming = clientSocket->receiveMessage();
        if (incoming.empty()) {
            // Suponemos que un mensaje vacío indica desconexión
            break;
        }
        // Convertimos el mensaje entrante a vector de bytes para deserializar
        std::vector<uint8_t> msgData = Protocol::stringToBytes(incoming);
        uint8_t msgCode;
        std::vector<std::vector<uint8_t>> msgFields;
        if (!Protocol::deserializeMessage(msgData, msgCode, msgFields)) {
            std::cerr << "Error al deserializar mensaje de " << username << std::endl;
            continue;
        }
        // Aquí se pueden procesar los diferentes códigos de mensaje (por ejemplo, chat, cambio de status, etc.)
        std::cout << "Mensaje recibido de " << username << ", código: " << static_cast<int>(msgCode) << std::endl;
        // Actualizar la última actividad del usuario (si corresponde)
        user->updateLastActivity();
    }

    // Una vez que el cliente se desconecta, se elimina el usuario registrado.
    unregisterUser(username);
    std::cout << "Usuario desconectado: " << username << std::endl;
    clientSocket->disconnect();
}

bool Server::registerUser(const std::string& username, std::shared_ptr<User> user) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    // Si el usuario ya existe, se rechaza el registro.
    if (users_.find(username) != users_.end()) {
        return false;
    }
    users_[username] = user;
    return true;
}

void Server::unregisterUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    users_.erase(username);
}
