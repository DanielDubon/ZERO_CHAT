#include "ChatUI.h"
#include "Client.h"
#include <iostream>
#include <memory>
#include <thread>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Uso: " << argv[0] << " <host> <puerto> <nombre_usuario>\n";
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    std::string username = argv[3];

    auto client = std::make_shared<Client>(host, port, username);
    
    if (!client->isConnected()) {
        std::cerr << "No se pudo conectar al servidor\n";
        return 1;
    }

    // Iniciar el cliente en un hilo separado
    std::thread clientThread([client]() {
        client->run();
    });

    // Iniciar la interfaz de usuario
    ChatUI ui(client);
    ui.start();

    // Esperar a que termine el hilo del cliente
    if (clientThread.joinable()) {
        clientThread.join();
    }

    return 0;
}
