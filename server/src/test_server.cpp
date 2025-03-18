#include "Server.h"
#include "Protocol.h"
#include "WebSocket.h"
#include <iostream>
#include <thread>
#include <chrono>

// Función auxiliar para simular el envío de un mensaje de registro.
std::string createRegistrationMessage(const std::string& username) {
    uint8_t code = 1; // Suponiendo que 1 es el código de registro
    std::vector<std::string> fields = {username};
    // Convertimos el mensaje serializado a string (para el test, usamos la conversión inversa)
    std::vector<uint8_t> bytes = Protocol::serializeMessage(code, fields);
    return Protocol::bytesToString(bytes);
}

int main() {
    int port = 8080;
    Server server(port);
    server.start();

    // Simular un cliente que se conecta y se registra
    std::shared_ptr<WebSocket> clientSocket = std::make_shared<WebSocket>();
    clientSocket->connect("ws://localhost:" + std::to_string(port));

    // Enviar mensaje de registro (ejemplo: username "testUser")
    std::string registrationMessage = createRegistrationMessage("testUser");
    // Usamos sendMessage en vez de sendMessage, ya que en nuestra simulación WebSocket usa sendMessage()
    clientSocket->sendMessage(registrationMessage);

    // Esperar un poco para que el servidor procese la conexión
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Simular desconexión: enviar un mensaje vacío
    clientSocket->sendMessage("");
    
    // Esperar un poco para que se maneje la desconexión
    std::this_thread::sleep_for(std::chrono::seconds(2));

    server.stop();
    std::cout << "Test finalizado." << std::endl;
    return 0;
}
