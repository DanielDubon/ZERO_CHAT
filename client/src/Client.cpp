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
        handleIncomingMessage(msg);
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
        connected = ws_.connect(host, port, "/?name=" + username);
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
    // Usaremos siempre el código 4 para "Mandar un mensaje"
    uint8_t messageType = 4;
    std::vector<std::string> fields;
    
    if (recipient == "all" || recipient == "~") {
        // Para chat general, se transforma "all" en "~"
        fields = {"~", message};
        std::cout << "Mensaje broadcast enviado: " << message << std::endl;
        Message msg(username_, "~", message);
        addMessage(msg);
    } else {
        // Mensaje privado
        fields = {recipient, message};
        std::cout << "Mensaje privado enviado a " << recipient << ": " << message << std::endl;
        Message msg(username_, recipient, message);
        addMessage(msg);
    }

    // Serializar y enviar el mensaje
    std::string serialized = Protocol::bytesToString(Protocol::serializeMessage(messageType, fields));
    ws_.send(serialized);
}

// Establecer el estado del usuario
void Client::setStatus(const std::string& newStatus) {
    status_ = newStatus;
    std::cout << "Estado actualizado a: " << status_ << std::endl;

    std::vector<std::string> fields = {username_, newStatus};
    auto msg = Protocol::serializeMessage(3, fields);
    ws_.send(Protocol::bytesToString(msg));
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
        case 50: { // SERVER_ERROR
            if (!fields.empty()) {
                std::cout << "\nError del servidor: " 
                          << Protocol::bytesToString(fields[0]) << std::endl;
            }
            break;
        }
        case 51: { // Respuesta a: Listar usuarios registrados
            std::cout << "\nUsuarios conectados:\n";
            {
                std::lock_guard<std::mutex> lock(usersMutex_);
                connectedUsers_.clear();
                // Se esperan pares: [username, status]
                for (size_t i = 0; i + 1 < fields.size(); i += 2) {
                    std::string uname = Protocol::bytesToString(fields[i]);
                    std::string status = Protocol::bytesToString(fields[i + 1]);
                    connectedUsers_.push_back({uname, status});
                    std::cout << "- " << uname << " estado: " << status;
                    if (uname == username_) {
                        std::cout << " (tú)";
                        this->status_ = status;
                    }
                    std::cout << std::endl;
                }
            }
            break;
        }
        case 52: { // Respuesta a: Obtener un usuario por su nombre
            if (fields.size() >= 2) {
                std::string uname = Protocol::bytesToString(fields[0]);
                std::string status = Protocol::bytesToString(fields[1]);
                std::cout << "\nInformación de usuario: " << uname 
                          << " estado: " << status << std::endl;
            }
            break;
        }
        case 53: { // Usuario se acaba de registrar (broadcast)
            if (fields.size() >= 2) {
                std::string uname = Protocol::bytesToString(fields[0]);
                std::string status = Protocol::bytesToString(fields[1]);
                std::cout << "\nNuevo usuario registrado: " << uname 
                          << " (" << status << ")" << std::endl;
                {
                    std::lock_guard<std::mutex> lock(usersMutex_);
                    // Agregar a la lista local si no existe
                    bool found = false;
                    for (const auto &u : connectedUsers_) {
                        if (u.first == uname) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        connectedUsers_.push_back({uname, status});
                    }
                }
            }
            break;
        }
        case 54: { // Usuario cambió estatus (broadcast)
            if (fields.size() >= 2) {
                std::string uname = Protocol::bytesToString(fields[0]);
                std::string newStatus = Protocol::bytesToString(fields[1]);
                std::cout << "\nEl usuario " << uname 
                          << " cambió su estatus a: " << newStatus << std::endl;
                {
                    std::lock_guard<std::mutex> lock(usersMutex_);
                    for (auto &u : connectedUsers_) {
                        if (u.first == uname) {
                            u.second = newStatus;
                            break;
                        }
                    }
                }
            }
            break;
        }
        case 55: { // Recibió mensaje (broadcast o privado)
            if (fields.size() >= 2) {
                std::string origen = Protocol::bytesToString(fields[0]);
                std::string contenido = Protocol::bytesToString(fields[1]);
                std::cout << "\n[" << origen << "]: " << contenido << std::endl;
                
                // Para este ejemplo, asumimos que los mensajes recibidos con código 55 son broadcast.
                Message msg(origen, "~", contenido);
                addMessage(msg);
            }
            break;
        }
        case 56: { // Respuesta a: Obtener historial de mensajes
            if (!fields.empty()) {
                // Se espera que el primer campo sea el número de mensajes (en formato numérico)
                int numMensajes = std::stoi(Protocol::bytesToString(fields[0]));
                std::cout << "\nHistorial de mensajes (" << numMensajes << " mensajes):\n";
                for (size_t i = 1; i + 1 < fields.size(); i += 2) {
                    std::string uname = Protocol::bytesToString(fields[i]);
                    std::string contenido = Protocol::bytesToString(fields[i + 1]);
                    std::cout << "[" << uname << "]: " << contenido << std::endl;
                }
            }
            break;
        }
        default:
            std::cout << "\nMensaje desconocido recibido (código " << (int)code << ")\n";
            break;
    }
    displayPrompt();
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
