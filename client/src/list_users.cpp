#include "WebSocket.h"
#include "Protocol.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    WebSocket ws;

    ws.onMessage([&](const std::string &raw) {
        ParsedMessage resp = Protocol::parse(raw);
        if (resp.id == SERVER_LIST) {
            std::cout << "Usuarios conectados:\n";
            for (auto &user : resp.fields)
                std::cout << " - " << user << "\n";
        }
        exit(0);
    });

    // Cambia host/puerto/usuario según necesites
    if (!ws.connect("127.0.0.1", 8080, "miUsuario")) {
        std::cerr << "Error conectando al servidor\n";
        return 1;
    }

    std::thread runner([&](){ ws.run(); });

    // Espera handshake
    while (!ws.isConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Envia solicitud de listado
    ws.queueMessage(Protocol::buildListUsers());

    runner.join();
    return 0;
}
