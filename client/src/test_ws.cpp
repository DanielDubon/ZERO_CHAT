// client/src/test_ws.cpp

#include "WebSocket.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    WebSocket ws;
    ws.onMessage([](const std::string &msg) {
        std::cout << "Servidor dice: " << msg << std::endl;
        exit(0);
    });

    const std::string host = "127.0.0.1";
    const int port = 8080;
    const std::string username = "miUsuario";

    if (!ws.connect(host, port, username)) {
        std::cerr << "Error conectando y registrando usuario" << std::endl;
        return 1;
    }

    // Inicia el bucle de eventos en un hilo separado
    std::thread runner([&ws]() {
        ws.run();
    });

    // Espera hasta que el handshake se complete
    while (!ws.isConnected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Registro exitoso como '" << username << "'" << std::endl;

    ws.close();
    runner.join();
    return 0;
}
