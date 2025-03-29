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
            std::cout << "Nueva conexión WebSocket establecida" << std::endl;
            session->server = server;
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
                server->unregisterUser(session->username);
                std::cout << "Usuario " << session->username << " desconectado" << std::endl;
            }
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
        case 1: { // Registro
            if (fields.empty()) return;
            std::string username = Protocol::bytesToString(fields[0]);
            session->username = username;
            auto user = std::make_shared<User>(username, "127.0.0.1");
            if (registerUser(username, user)) {
                std::cout << "Usuario registrado: " << username << std::endl;
                connections_[username] = wsi;  // Guardar la conexión
                sendUserList(wsi);  // Enviar lista de usuarios al nuevo cliente
            }
            break;
        }

        case 2: { // Solicitud de lista de usuarios
            sendUserList(wsi);
            break;
        }

        case 4: { // Mensaje de chat
            if (fields.size() < 2) return;
            std::string dest = Protocol::bytesToString(fields[0]);
            std::string content = Protocol::bytesToString(fields[1]);
            std::cout << session->username << " -> " << dest << ": " << content << std::endl;

            // Preparar mensaje para reenviar
            std::vector<std::string> messageFields = {session->username, content};

            if (dest == "all") {
                // Reenviar con code=4
                auto response = Protocol::serializeMessage(4, messageFields); 
                // Mandar a todos menos el emisor
                for (const auto& conn : connections_) {
                    if (conn.first != session->username) {
                        sendMessage(conn.second, response);
                    }
                }
            }
            
            break;
        }

        case 54: { // Mensaje PRIVADO
            // Esperamos 3 campos: [sender, destinatario, contenido]
            if (fields.size() < 3) return;
        
            std::string sender   = Protocol::bytesToString(fields[0]);
            std::string dest     = Protocol::bytesToString(fields[1]);
            std::string content  = Protocol::bytesToString(fields[2]);
        
            std::cout << "[LOG] Mensaje privado de " << sender << " a " << dest << ": " << content << std::endl;
        
            // Prepara la respuesta con code=55 para que el cliente lo muestre como privado
            std::vector<std::string> messageFields = {sender, content};
            auto response = Protocol::serializeMessage(55, messageFields);
        
            // Enviar al destinatario
            auto it = connections_.find(dest);
            if (it != connections_.end()) {
                sendMessage(it->second, response);
            }
        
            // Opcional: si quieres que el emisor también vea su propio mensaje privado, reenvíale una copia
            auto me = connections_.find(sender);
            if (me != connections_.end()) {
                sendMessage(me->second, response);
            }
        
            break;
        }        

        case 99: { // LOGOUT
            if (session && !session->username.empty()) {
                connections_.erase(session->username);
                unregisterUser(session->username);
                std::cout << "Usuario " << session->username << " desconectado" << std::endl;
            }
            break;
        }
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
    for (const auto& user : users_) {
        userList.push_back(user.first);
    }
    
    // Serializar y enviar la lista
    auto response = Protocol::serializeMessage(51, userList);  // 51: SERVER_LIST
    sendMessage(wsi, response);
}
