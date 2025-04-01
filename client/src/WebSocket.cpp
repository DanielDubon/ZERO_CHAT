#include "WebSocket.h"
#include "Protocol.h"
#include <cstring>
#include <iostream>
#include <thread>

struct per_session_data {};

struct lws_protocols WebSocket::protocols_[] = {
    {
        "chat-protocol",
        WebSocket::callback,
        sizeof(per_session_data),
        4096,
    },
    { nullptr, nullptr, 0, 0 }
};

WebSocket::WebSocket()
    : context_(nullptr), wsi_(nullptr), connected_(false) {
}

WebSocket::~WebSocket() {
    close();
}

bool WebSocket::connect(const std::string& host, int port, const std::string& path) {
    std::cout << "Intentando conectar a " << host << ":" << port << std::endl;

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols_;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;
    info.user = this;

    context_ = lws_create_context(&info);
    if (!context_) {
        std::cerr << "Error creando contexto WebSocket" << std::endl;
        return false;
    }

    struct lws_client_connect_info conn_info;
    memset(&conn_info, 0, sizeof(conn_info));

    conn_info.context = context_;
    conn_info.address = host.c_str();
    conn_info.port = port;
    conn_info.path = path.c_str();
    conn_info.host = host.c_str();
    conn_info.origin = host.c_str();
    conn_info.protocol = protocols_[0].name;
    conn_info.pwsi = &wsi_;
    conn_info.ssl_connection = 0;

    wsi_ = lws_client_connect_via_info(&conn_info);
    if (!wsi_) {
        std::cerr << "Error creando conexión WebSocket" << std::endl;
        lws_context_destroy(context_);
        context_ = nullptr;
        return false;
    }

    // Esperar a que se establezca la conexión
    int retries = 50;  // Aumentar el número de intentos
    while (!connected_ && retries-- > 0 && context_) {
        lws_service(context_, 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!connected_) {
        std::cerr << "Timeout esperando conexión" << std::endl;
        close();
        return false;
    }

    std::cout << "Conexión WebSocket establecida exitosamente" << std::endl;
    return true;
}

void WebSocket::run() {
    while (context_) {
        if (lws_service(context_, 50) < 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool WebSocket::send(const std::string& payload) {
    if (!connected_ || !wsi_) {
        std::cerr << "Error: No conectado al intentar enviar mensaje" << std::endl;
        return false;
    }

    std::vector<unsigned char> buf(LWS_PRE + payload.size());
    memcpy(buf.data() + LWS_PRE, payload.data(), payload.size());

    int result = lws_write(wsi_, buf.data() + LWS_PRE, payload.size(), LWS_WRITE_TEXT);
    if (result < 0) {
        std::cerr << "Error escribiendo mensaje WebSocket" << std::endl;
        return false;
    }
    return true;
}

void WebSocket::close() {
    if (connected_) {
        // Enviar mensaje de desconexión
        std::vector<std::string> fields = {"LOGOUT"};
        auto msg = Protocol::serializeMessage(99, fields);  // 99: CLIENT_LOGOUT
        send(Protocol::bytesToString(msg));
    }
    if (wsi_) {
        std::cout << "Cerrando conexión WebSocket..." << std::endl;
        lws_callback_on_writable(wsi_);
        wsi_ = nullptr;
    }
    if (context_) {
        lws_context_destroy(context_);
        context_ = nullptr;
    }
    connected_ = false;
}

// Manejadores de eventos
void WebSocket::onMessage(MessageHandler handler) {
    messageHandler_ = std::move(handler);
}

void WebSocket::onConnect(std::function<void()> handler) {
    connectHandler_ = std::move(handler);
}

void WebSocket::onDisconnect(std::function<void()> handler) {
    disconnectHandler_ = std::move(handler);
}

void WebSocket::onError(std::function<void(const std::string&)> handler) {
    errorHandler_ = std::move(handler);
}

// Métodos de cola de mensajes
void WebSocket::queueMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    messageQueue_.push(msg);
    if (connected_) {
        lws_callback_on_writable(wsi_);
    }
}

bool WebSocket::hasQueuedMessages() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return !messageQueue_.empty();
}

std::string WebSocket::getNextQueuedMessage() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (messageQueue_.empty()) return "";
    std::string msg = messageQueue_.front();
    messageQueue_.pop();
    return msg;
}

bool WebSocket::isConnected() const {
    return connected_;
}

// Manejadores internos
void WebSocket::handleConnect() {
    connected_ = true;
    if (connectHandler_) {
        connectHandler_();
    }
}

void WebSocket::handleDisconnect() {
    connected_ = false;
    if (disconnectHandler_) {
        disconnectHandler_();
    }
}

void WebSocket::handleError(const std::string& error) {
    if (errorHandler_) {
        errorHandler_(error);
    }
}

void WebSocket::processReceivedMessage(const std::string& message) {
    if (messageHandler_) {
        messageHandler_(message);
    }
}

int WebSocket::callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len) {
    WebSocket* self = static_cast<WebSocket*>(lws_context_user(lws_get_context(wsi)));
    if (!self) return 0;
    
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            self->connected_ = true;
            if (self->connectHandler_) {
                self->connectHandler_();
            }
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            self->connected_ = false;
            if (self->errorHandler_) {
                std::string error = in ? std::string((char*)in, len) : "unknown error";
                self->errorHandler_(error);
            }
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (in && len && self->messageHandler_) {
                std::string message(static_cast<char*>(in), len);
                self->messageHandler_(message);
            }
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            self->connected_ = false;
            if (self->disconnectHandler_) {
                self->disconnectHandler_();
            }
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (self->hasQueuedMessages()) {
                std::string msg = self->getNextQueuedMessage();
                if (!msg.empty()) {
                    self->send(msg);
                }
            }
            break;

        default:
            break;
    }
    
    return 0;
}


