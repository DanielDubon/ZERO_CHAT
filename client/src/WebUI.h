#ifndef WEB_UI_H
#define WEB_UI_H

#include "Client.h"
#include <string>
#include <memory>
#include <functional>
#include "../../external/cpp-httplib/httplib.h"
#include "../../external/json/single_include/nlohmann/json.hpp"

class WebUI {
public:
    WebUI(std::shared_ptr<Client> client, int port = 8080);
    void start();
    void stop();
    bool isRunning() const;

private:
    std::shared_ptr<Client> client_;
    httplib::Server server_;
    bool running_;
    int port_;

    void setupRoutes();
    void broadcastToClients(const std::string& message);
    void createWebFiles();  // Nuevo método para crear archivos web
    
    // Handlers para las diferentes rutas
    void handleGetIndex(const httplib::Request& req, httplib::Response& res);
    void handleGetUsers(const httplib::Request& req, httplib::Response& res);
    void handlePostMessage(const httplib::Request& req, httplib::Response& res);
    void handlePostStatus(const httplib::Request& req, httplib::Response& res);
    void handleWebSocket(const httplib::Request& req, httplib::Response& res);
    void handleGetMessages(const httplib::Request& req, httplib::Response& res);  // Nuevo handler
};

#endif // WEB_UI_H 