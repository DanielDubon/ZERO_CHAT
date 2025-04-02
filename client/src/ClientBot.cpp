// ClientBot.cpp
#include "Client.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

int main() {
    // Configuración de conexión (modifica host/puerto según necesites)
    std::string host = "localhost"; // o la IP de tu servidor
    int port = 8080;                // Puerto del servidor WebSocket
    std::string username = "BotTest";

    // Crear instancia del cliente (bot)
    Client bot(host, port, username);

    // Ejecutar el cliente en un hilo separado
    std::thread clientThread([&bot]() {
        bot.run();
    });

    // Espera para asegurar que la conexión se establezca
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Vector con los posibles estados
    std::vector<std::string> statuses = {"ACTIVO", "OCUPADO", "INACTIVO"};
    int counter = 0;

    // Bucle infinito para depuración
    while (true) {
        // 1. Enviar mensaje broadcast
        std::string broadcastMsg = "Broadcast mensaje " + std::to_string(counter);
        bot.sendMessage("all", broadcastMsg);
        std::cout << "[DEBUG] Enviado broadcast: " << broadcastMsg << std::endl;
        
        // 2. Cambiar estado cíclicamente
        std::string newStatus = statuses[counter % statuses.size()];
        bot.setStatus(newStatus);
        std::cout << "[DEBUG] Estado cambiado a: " << newStatus << std::endl;

        // 3. Enviar mensaje privado a sí mismo
        std::string privateMsg = "Mensaje privado " + std::to_string(counter);
        bot.sendMessage("Bianca", privateMsg);
        std::cout << "[DEBUG] Enviado mensaje privado: " << privateMsg << std::endl;

        counter++;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Nota: en un caso real se debería terminar la ejecución limpiamente
    clientThread.join();
    return 0;
}
