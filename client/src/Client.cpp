#include "Client.h"
#include "Protocol.h"
#include "WebSocket.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>

// Constructor por defecto
Client::Client() : ws_(), username_(""), status_("ACTIVO") {}

// Constructor
Client::Client(const std::string& host, int port, const std::string& username)
    : ws_(), username_(username), status_("ACTIVO"), host_(host), port_(port) {
    
    // Configurar los handlers antes de conectar
    ws_.onMessage([this](const std::string& msg) {
        handleIncomingMessage(msg);  // Nueva función para procesar mensajes
    });

    ws_.onConnect([this]() {
        std::cout << "Conectado al servidor como " << username_ << std::endl;
        // Enviar mensaje de registro
        std::vector<std::string> regFields = {username_};
        auto regMsg = Protocol::serializeMessage(1, regFields);
        ws_.send(Protocol::bytesToString(regMsg));
    });

    ws_.onDisconnect([]() {
        std::cout << "Desconectado del servidor" << std::endl;
    });

    ws_.onError([](const std::string& error) {
        std::cerr << "Error: " << error << std::endl;
    });

    // Intentar conectar con reintentos
    int retries = 3;
    bool connected = false;
    while (retries-- > 0 && !connected) {
        connected = ws_.connect(host, port);
        if (!connected) {
            std::cout << "Reintentando conexión..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 1 segundo
        }
    }
}

// Destructor
Client::~Client() {
    ws_.close();
}

// Enviar un mensaje al servidor
void Client::sendMessage(const std::string& recipient, const std::string& message) {
    uint8_t messageType;
    std::vector<std::string> fields;

    if (recipient == "all") {
        messageType = 4;  // BROADCAST_MESSAGE
        fields = {"all", message};
        std::cout << "Mensaje enviado a todos: " << message << std::endl;
        
        // Añadir el mensaje al almacén local para mostrarlo en la UI
        Message msg(username_, "all", message);
        addMessage(msg);
    } else {
        messageType = 54;  // PRIVATE_MESSAGE
        fields = {username_, recipient, message};
        std::cout << "Mensaje privado enviado a " << recipient << ": " << message << std::endl;
        
        // Añadir el mensaje al almacén local para mostrarlo en la UI
        Message msg(username_, recipient, message);
        addMessage(msg);
    }

    ws_.send(Protocol::bytesToString(Protocol::serializeMessage(messageType, fields)));
}

// Establecer el estado del usuario
void Client::setStatus(const std::string& status) {
    status_ = status;
    std::cout << "Estado actualizado a: " << status_ << std::endl;
}

// Listar usuarios conectados
void Client::listConnectedUsers() {
    // Enviar solicitud al servidor para obtener la lista de usuarios
    ws_.send(Protocol::bytesToString(Protocol::serializeMessage(static_cast<uint8_t>(2), std::vector<std::string>{})));
}

// Verificar si está conectado
bool Client::isConnected() const {
    return ws_.isConnected();
}

// Ejecutar el cliente
void Client::run() {
    ws_.run();
}

// Añadir esta nueva función para procesar mensajes entrantes
void Client::handleIncomingMessage(const std::string& rawMsg) {
    std::vector<uint8_t> data = Protocol::stringToBytes(rawMsg);
    uint8_t code;
    std::vector<std::vector<uint8_t>> fields;

    if (!Protocol::deserializeMessage(data, code, fields)) {
        std::cerr << "Error: Mensaje mal formado\n";
        return;
    }

    switch (code) {
        case 3: // SERVER_WELCOME
            if (!fields.empty()) {
                std::cout << "\n¡Bienvenido al chat! Tu nombre de usuario es: " 
                         << Protocol::bytesToString(fields[0]) << std::endl;
            }
            break;

        case 4: // SERVER_MESSAGE (Broadcast)
            if (fields.size() >= 2) {
                std::string sender = Protocol::bytesToString(fields[0]);
                std::string content = Protocol::bytesToString(fields[1]);
                std::cout << "\n[" << sender << "]: " << content << std::endl;
                
                // Añadir el mensaje al almacén como broadcast
                Message msg(sender, "all", content);
                // Verificar que el tipo sea correcto
                std::cout << "Tipo de mensaje recibido: " << msg.getType() << std::endl;
                addMessage(msg);
                
                displayPrompt();  // Volver a mostrar el prompt después del mensaje
            }
            break;

        case 51: // SERVER_LIST
            {
                std::cout << "\nUsuarios conectados:\n";
                
                // Actualizar la lista de usuarios conectados
                std::lock_guard<std::mutex> lock(usersMutex_);
                connectedUsers_.clear();
                
                for (const auto& field : fields) {
                    std::string username = Protocol::bytesToString(field);
                    std::cout << "- " << username;
                    if (username == username_) {
                        std::cout << " (tú)";
                        // Añadir el usuario actual con su estado
                        connectedUsers_.push_back({username, status_});
                    } else {
                        // Añadir otros usuarios con estado desconocido
                        connectedUsers_.push_back({username, "ACTIVO"});
                    }
                    std::cout << std::endl;
                }
                displayPrompt();
            }
            break;

        case 5: // SERVER_LIST_RESPONSE
            {
                std::cout << "\nUsuarios conectados:\n";
                
                // Actualizar la lista de usuarios conectados
                std::lock_guard<std::mutex> lock(usersMutex_);
                connectedUsers_.clear();
                
                for (const auto& field : fields) {
                    std::string username = Protocol::bytesToString(field);
                    std::cout << "- " << username << std::endl;
                    
                    // Añadir a la lista de usuarios conectados
                    if (username == username_) {
                        connectedUsers_.push_back({username, status_});
                    } else {
                        connectedUsers_.push_back({username, "ACTIVO"});
                    }
                }
                displayPrompt();
            }
            break;

        case 6: // SERVER_ERROR
            if (!fields.empty()) {
                std::cout << "\nError del servidor: " 
                         << Protocol::bytesToString(fields[0]) << std::endl;
            }
            displayPrompt();
            break;

        case 55: // PRIVATE_MESSAGE
            if (fields.size() >= 2) {
                std::string sender = Protocol::bytesToString(fields[0]);
                std::string content = Protocol::bytesToString(fields[1]);
                std::cout << "\n[Mensaje Privado de " << sender << "]: " << content << std::endl;
                
                // Añadir el mensaje al almacén como privado
                Message msg(sender, username_, content);
                addMessage(msg);
                
                displayPrompt();
            }
            break;

        default:
            std::cout << "\nMensaje desconocido recibido (código " << (int)code << ")\n";
            displayPrompt();
            break;
    }
}

// Añadir esta función auxiliar
void Client::displayPrompt() const {
    std::cout << "> " << std::flush;
}

std::vector<Message> Client::getMessages() {
    std::lock_guard<std::mutex> lock(messagesMutex_);
    return messages_;
}

void Client::addMessage(const Message& message) {
    std::lock_guard<std::mutex> lock(messagesMutex_);
    messages_.push_back(message);
    
    // Limitar el número de mensajes almacenados para evitar problemas de memoria
    const size_t MAX_MESSAGES = 100;
    if (messages_.size() > MAX_MESSAGES) {
        messages_.erase(messages_.begin(), messages_.begin() + (messages_.size() - MAX_MESSAGES));
    }
    
    // Depuración
    std::cout << "Mensaje añadido al almacén. Total de mensajes: " << messages_.size() << std::endl;
}
