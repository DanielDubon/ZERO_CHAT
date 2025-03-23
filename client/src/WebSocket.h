#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <functional>
#include <queue>
#include <mutex>
#include <libwebsockets.h>
#include "Protocol.h"

class WebSocket {
public:
    using MessageHandler = std::function<void(const std::string&)>; 

    WebSocket();
    ~WebSocket();

    // Métodos principales
    bool connect(const std::string& host, int port, const std::string& path = "/");
    bool send(const std::string& payload);
    bool isConnected() const;
    void close();
    void run();

    // Manejadores de eventos
    void onMessage(MessageHandler handler);
    void onConnect(std::function<void()> handler);
    void onDisconnect(std::function<void()> handler);
    void onError(std::function<void(const std::string&)> handler);

    // Cola de mensajes
    void queueMessage(const std::string& msg);
    bool hasQueuedMessages() const;
    std::string getNextQueuedMessage();

private:
    struct lws_context *context_;
    struct lws *wsi_;
    MessageHandler messageHandler_;
    std::function<void()> connectHandler_;
    std::function<void()> disconnectHandler_;
    std::function<void(const std::string&)> errorHandler_;
    
    bool connected_;
    std::string pendingMessage_;
    
    std::queue<std::string> messageQueue_;
    mutable std::mutex queueMutex_;

    static int callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len);
    static struct lws_protocols protocols_[];

    void handleConnect();
    void handleDisconnect();
    void handleError(const std::string& error);
    void processReceivedMessage(const std::string& message);
};

#endif // WEBSOCKET_H
