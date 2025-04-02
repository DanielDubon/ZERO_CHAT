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
    std::string statusNumStr;
    if(newStatus == "ACTIVO")
        statusNumStr = "1";
    else if(newStatus == "OCUPADO")
        statusNumStr = "2";
    else if(newStatus == "INACTIVO")
        statusNumStr = "3";
    else if(newStatus == "DISCONNECTED")
        statusNumStr = "0";
    else {
        std::cerr << "Estado inválido: " << newStatus << std::endl;
        return;
    }

    status_ = newStatus; // Guarda el estado en formato texto para la UI
    std::cout << "Estado actualizado a: " << status_ << std::endl;

    std::vector<std::string> fields = {username_, statusNumStr };
    auto msg = Protocol::serializeMessage(3, fields);
    ws_.send(Protocol::bytesToString(msg));
}


// Listar usuarios conectados
void Client::listConnectedUsers() {
    // Cambiar el código 2 por 1 para solicitar la lista de usuarios
    std::vector<std::string> fields;  // No necesitamos campos
    auto msg = Protocol::serializeMessage(1, fields);
    if (!ws_.send(Protocol::bytesToString(msg))) {
        std::cerr << "Error al solicitar lista de usuarios\n";
    } else {
        std::cout << "Solicitud de lista de usuarios enviada\n";
    }
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
        case 51: { // SERVER_LIST (Lista de usuarios)
            std::cout << "\nRecibida lista actualizada de usuarios\n";
            {
                std::lock_guard<std::mutex> lock(usersMutex_);
                connectedUsers_.clear();
                
                // Procesar todos los campos recibidos
                for (size_t i = 1; i < fields.size(); i += 2) {
                    std::string username = Protocol::bytesToString(fields[i]);
                    std::string status = Protocol::bytesToString(fields[i + 1]);
                    connectedUsers_.push_back({username, status});
                    std::cout << "Usuario en lista: " << username << " (" << status << ")\n";
                }
            }
            
            // Llamar al callback para actualizar la UI inmediatamente
            if (updateUserListCallback_) {
                std::cout << "Llamando al callback de actualización de usuarios\n";
                updateUserListCallback_();
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
        case 53: { // Nuevo usuario conectado
            if (fields.size() >= 2) {
                std::string newUser = Protocol::bytesToString(fields[0]);
                std::string newStatus = Protocol::bytesToString(fields[1]);
                std::cout << "\nNuevo usuario conectado: " << newUser << "\n";
                
                // Solicitar lista actualizada inmediatamente
                listConnectedUsers();
                
                if (updateUserListCallback_) {
                    updateUserListCallback_();
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
                if (updateUserListCallback_)
                {
                    updateUserListCallback_();
                }
                
            }
            break;
        }
        case 55: { // Recibió mensaje (broadcast o privado)
            if (fields.size() >= 2) {
                if (fields.size() >= 3) {
                    // Mensaje privado: [origen, destinatario, contenido]
                    std::string origen = Protocol::bytesToString(fields[0]);
                    std::string destino = Protocol::bytesToString(fields[1]);
                    std::string contenido = Protocol::bytesToString(fields[2]);
                    std::cout << "\n[Mensaje Privado] " << origen << " -> " << destino << ": " << contenido << std::endl;
                    
                    // Crear y almacenar el mensaje
                    Message msg(origen, destino, contenido);
                    addMessage(msg);
                    
                    // Forzar actualización de la UI
                    if (updateUserListCallback_) {
                        updateUserListCallback_();
                    }
                } else {
                    // Mensaje broadcast: [origen, contenido]
                    std::string origen = Protocol::bytesToString(fields[0]);
                    std::string contenido = Protocol::bytesToString(fields[1]);
                    std::cout << "\n[Mensaje Broadcast] " << origen << ": " << contenido << std::endl;
                    
                    // Crear y almacenar el mensaje
                    Message msg(origen, "~", contenido);
                    addMessage(msg);
                    
                    // Forzar actualización de la UI
                    if (updateUserListCallback_) {
                        updateUserListCallback_();
                    }
                }
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
