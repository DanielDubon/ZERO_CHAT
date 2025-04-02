#include "Protocol.h"
#include "Server.h"
#include <vector>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <libwebsockets.h>

static struct lws_protocols protocols[] = {
    {
        "chat-protocol",
        Server::wsCallback,
        sizeof(SessionData),
        4096,
    },
    { nullptr, nullptr, 0, 0 }
};

Server::Server(int port)
    : port_(port), running_(false), context_(nullptr) {
}

Server::~Server() {
    stop();
}

void Server::start() {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof info);

    info.port = port_;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;  // Quitar SSL por ahora
    info.user = this;

    context_ = lws_create_context(&info);
    if (!context_) {
        throw std::runtime_error("Error creando contexto del servidor");
    }

    running_ = true;
    std::cout << "Servidor WebSocket iniciado en puerto " << port_ << std::endl;

    while (running_) {
        lws_service(context_, 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Server::stop() {
    running_ = false;
    if (context_) {
        lws_context_destroy(context_);
        context_ = nullptr;
    }
    std::cout << "Servidor detenido." << std::endl;
}

int Server::wsCallback(struct lws *wsi, enum lws_callback_reasons reason,
                      void *user, void *in, size_t len) {
    SessionData *session = static_cast<SessionData*>(user);
    Server *server = static_cast<Server*>(lws_context_user(lws_get_context(wsi)));

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            std::cout << "Nueva conexión establecida" << std::endl;
            session->server = server;
        
            if (!session->username.empty()) {
                // Crea el objeto User y márcalo como ACTIVO
                auto userObj = std::make_shared<User>(session->username, "127.0.0.1");
                userObj->setStatus("ACTIVO");
                server->users_[session->username] = userObj;
                server->connections_[session->username] = wsi;
        
                std::cout << "[LOG] Usuario " << session->username << " conectado." << std::endl;
            }
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            if (!in || len == 0) return 0;

            std::vector<uint8_t> data(static_cast<uint8_t*>(in),
                                    static_cast<uint8_t*>(in) + len);
            server->handleClientData(wsi, session, data);
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            if (session && !session->username.empty()) {
                std::string disconnectedUser = session->username;
                server->unregisterUser(disconnectedUser);
                std::cout << "Usuario " << disconnectedUser << " desconectado" << std::endl;
                // Remover también la conexión del mapa de conexiones
                server->connections_.erase(disconnectedUser);
                session -> username.clear();
                // Notificar a todos los clientes con la lista actualizada
                std::lock_guard<std::mutex> lock(server->usersMutex_);
                for (const auto& conn : server->connections_) {
                    server->sendUserList(conn.second);
                }
            }
            break;
        }

        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
            // Buffer temporal para leer el valor de la query 'name'
            char nameBuffer[128];
            memset(nameBuffer, 0, sizeof(nameBuffer));

            // Intenta obtener el valor de la query 'name='
            // Retorna < 0 si no existe ese parámetro en la URL
            if (lws_get_urlarg_by_name(wsi, "name", nameBuffer, sizeof(nameBuffer)) == NULL) {
                std::cerr << "[ERROR] Falta parámetro 'name' en la query." << std::endl;
                // Rechazamos la conexión
                return 1; 
            }

            // nameBuffer contiene el valor después de 'name='
            std::string candidateName(nameBuffer);

            // Si candidateName empieza con "name=", removerla
            const std::string prefix = "name=";
            if (candidateName.compare(0, prefix.size(), prefix) == 0) {
                candidateName = candidateName.substr(prefix.size());
            }
            
            // Validaciones del protocolo
            if (candidateName.empty() || candidateName == "~") {
                std::cerr << "[ERROR] Nombre vacío o '~' no permitido." << std::endl;
                return 1; // Rechaza la conexión
            }

            // Chequear si ya hay un usuario con ese nombre en línea
            if (server->isUserOnline(candidateName)) {
                std::cerr << "[ERROR] Usuario " << candidateName << " ya está en línea." << std::endl;
                return 1; // Rechaza la conexión
            }

            // Si todo ok, guardamos el nombre en la SessionData para usarlo luego
            session->username = candidateName;

            // Podemos aceptar la conexión devolviendo 0
            return 0;
            break;
        }

        

        default:
            break;
    }

    return 0;
}

void Server::handleClientData(struct lws *wsi, SessionData *session, 
    const std::vector<uint8_t>& data) {
    uint8_t code;
    std::vector<std::vector<uint8_t>> fields;

    if (!Protocol::deserializeMessage(data, code, fields)) {
        std::cerr << "Error: Mensaje mal formado" << std::endl;
        return;
    }

    switch (code) {
        case 1: { // Listar usuarios conectados
            // No se esperan campos.
            sendUserList(wsi);
            break;
        }

        case 2: { // Obtener un usuario por su nombre
            if (fields.size() < 1) return;
                std::string username = Protocol::bytesToString(fields[0]);
                auto it = users_.find(username);
                if (it != users_.end()) {
                    // Respuesta con código 52: [Username, Status]
                    std::vector<std::string> responseFields = { username, it->second->getStatus() };
                    auto response = Protocol::serializeMessage(52, responseFields);
                    sendMessage(wsi, response);
                } else {
                    // Error: usuario no existe (código 50 con error 1)
                    std::vector<uint8_t> errorMsg = { 50, 1 };
                    sendMessage(wsi, errorMsg);
                }
                    break;
        }

        case 3: { // Cambiar estatus de un usuario
        // Se esperan 2 campos: [username, newStatus]
            if (fields.size() < 2) return;
                std::string username = Protocol::bytesToString(fields[0]);
                std::string newStatusStr = Protocol::bytesToString(fields[1]);

                int statusNum;

                try
                {
                    statusNum = std::stoi(newStatusStr);
                }
                catch(const std::exception& e)
                {
                   std::vector<uint8_t> errorMsg = {50, 2};
                   sendMessage(wsi, errorMsg);
                   break;
                }

                if (statusNum < 0 || statusNum > 3) {
                    std::vector<uint8_t> errorMsg = { 50, 2 }; // Error 2: estatus inválido
                    sendMessage(wsi, errorMsg);
                    break;
                }

                std::string statusText;
                switch (statusNum) {
                case 0: statusText = "DISCONNECTED"; break;
                case 1: statusText = "ACTIVO"; break;
                case 2: statusText = "OCUPADO"; break;
                case 3: statusText = "INACTIVO"; break;
                 }
                
                auto it = users_.find(username);
            if (it != users_.end()) {
                it->second->setStatus(statusText);
                std::cout << "[LOG] El usuario " << username << " ahora es: " << statusText << std::endl;
                // Notificar a todos (código 54: Usuario cambió estatus)
                std::vector<std::string> responseFields = { username, statusText };
                auto response = Protocol::serializeMessage(54, responseFields);
                for (auto &conn : connections_) {
                sendMessage(conn.second, response);
                }
                } else {
                    // Error: usuario no existe
                    std::vector<uint8_t> errorMsg = { 50, 1 };
                    sendMessage(wsi, errorMsg);
                 }
                break;
        }

        case 4: { // Mandar un mensaje
            // Se esperan 2 campos: [destino, mensaje]
            if (fields.size() < 2) return;
            std::string dest = Protocol::bytesToString(fields[0]);
            std::string content = Protocol::bytesToString(fields[1]);
            std::cout << session->username << " -> " << dest << ": " << content << std::endl;
        
            if (dest == "~") { // Chat general (broadcast)
                // Enviar con 2 campos: [origen, mensaje]
                std::vector<std::string> responseFields = { session->username, content };
                auto response = Protocol::serializeMessage(55, responseFields);
                for (auto &conn : connections_) {
                    sendMessage(conn.second, response);
                }
            } else { // Mensaje privado
                auto it = connections_.find(dest);
                if (it != connections_.end()) {
                    // Enviar con 3 campos: [origen, destinatario, mensaje]
                    std::vector<std::string> responseFields = { session->username, dest, content };
                    auto response = Protocol::serializeMessage(55, responseFields);
                    sendMessage(it->second, response);
                    // Opcional: reenviar copia al emisor
                    sendMessage(wsi, response);
                } else {
                    // Error: destinatario desconectado (código 50 con error 4)
                    std::vector<uint8_t> errorMsg = { 50, 4 };
                    sendMessage(wsi, errorMsg);
                }
                std::lock_guard<std::mutex> lock(historyMutex_);
                Message histMsg(session->username, dest, content);
                history_.push_back(histMsg);
            }
            break;
        }

        case 5: { // Obtener Mensajes (historial)
            // Se espera 1 campo: [chat]
            if (fields.size() < 1) return;
            std::string chatPartner = Protocol::bytesToString(fields[0]);
            
            std::vector<Message> filtered;
            {
                std::lock_guard<std::mutex> lock(historyMutex_);
                if (chatPartner == "~") {
                    // Para chat general: filtrar mensajes broadcast (donde el receptor es "~")
                    for (const auto &msg : history_) {
                        if (msg.getReceiver() == "~") {
                            filtered.push_back(msg);
                        }
                    }
                } else {
                    // Para chat privado: filtrar mensajes entre el solicitante (session->username)
                    // y el chatPartner
                    for (const auto &msg : history_) {
                        if ((msg.getSender() == chatPartner && msg.getReceiver() == session->username) ||
                            (msg.getSender() == session->username && msg.getReceiver() == chatPartner)) {
                            filtered.push_back(msg);
                        }
                    }
                }
            }
            // Limitar a 255 mensajes, ya que el campo del número es 1 byte.
            if (filtered.size() > 255) {
                filtered.resize(255);
            }
            
            // Construir la respuesta para el historial (código 56)
            // Primer campo: número de mensajes (convertido a 1 byte)
            // Luego, para cada mensaje: [remitente, contenido]
            std::vector<std::string> responseFields;
            responseFields.push_back(std::string(1, static_cast<char>(filtered.size())));
            for (const auto &msg : filtered) {
                responseFields.push_back(msg.getSender());
                responseFields.push_back(msg.getContent());
            }
            auto response = Protocol::serializeMessage(56, responseFields);
            sendMessage(wsi, response);
            break;
        }

                default:
                std::cerr << "[ERROR] Código de mensaje no reconocido: " << static_cast<int>(code) << std::endl;
                break;
                }
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

bool Server::sendMessage(struct lws *wsi, const std::vector<uint8_t>& data) {
    // Reservar espacio para LWS_PRE
    std::vector<uint8_t> buf(LWS_PRE + data.size());
    memcpy(buf.data() + LWS_PRE, data.data(), data.size());

    int result = lws_write(wsi, buf.data() + LWS_PRE, data.size(), LWS_WRITE_TEXT);
    return result > 0;
}

void Server::sendUserList(struct lws *wsi) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    std::vector<std::string> userList;

    userList.push_back(std::string(1, static_cast<char>(users_.size())));
    for (const auto& kv : users_) {
        userList.push_back(kv.first);                   
        userList.push_back(kv.second->getStatus());     
    }
    
    auto response = Protocol::serializeMessage(51, userList);
    sendMessage(wsi, response);
}

bool Server::isUserOnline(const std::string& username) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    return users_.find(username) != users_.end();
}
