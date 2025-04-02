#include "ChatUI.h"
#include "WebUI.h"
#include "Client.h"
#include <iostream>
#include <memory>
#include <thread>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Uso: " << argv[0] << " <host> <puerto> <nombre_usuario> [puerto_web]" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    std::string username = argv[3];
    
    // Puerto para la interfaz web (opcional, por defecto 8080)
    int webPort = 8080;
    if (argc > 4) {
        webPort = std::stoi(argv[4]);
    }

    std::shared_ptr<Client> client;
    try {
        client = std::make_shared<Client>(host, port, username);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error al conectar: " << e.what() << std::endl;
        return 1;
    }
    
    if (!client->isConnected()) {
        std::cerr << "No se pudo conectar al servidor" << std::endl;
        return 1;
    }

    // Iniciar el cliente en un hilo separado
    std::thread clientThread([client]() {
        try {
            client->run();
        } catch (const std::runtime_error& e) {
            std::cerr << "Error durante la ejecución: " << e.what() << std::endl;
        }
    });

    // Iniciar la interfaz web en otro hilo
    auto webUI = std::make_shared<WebUI>(client, webPort);
    std::thread webThread([webUI]() {
        webUI->start();
    });

    // Iniciar la interfaz de consola
    ChatUI ui(client);
    ui.start();

    // Detener la interfaz web
    webUI->stop();

    // Esperar a que terminen los hilos
    if (webThread.joinable()) {
        webThread.join();
    }
    
    if (clientThread.joinable()) {
        clientThread.join();
    }

    return 0;
}
