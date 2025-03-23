#include "WebSocket.h"
#include "Protocol.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

namespace {
    std::atomic<bool> running{true};

    void signalHandler(int) {
        running = false;
    }
}

int main() {
    signal(SIGINT, signalHandler);

    WebSocket ws;
    bool connected = false;

    // Configurar handlers con más información
    ws.onMessage([](const std::string& msg) {
        std::vector<uint8_t> data(msg.begin(), msg.end());
        uint8_t code;
        std::vector<std::vector<uint8_t>> fields;
        
        if (Protocol::deserializeMessage(data, code, fields)) {
            switch (code) {
                case 51: // SERVER_LIST
                    std::cout << "Lista de usuarios conectados:" << std::endl;
                    for (const auto& field : fields) {
                        std::cout << " - " << Protocol::bytesToString(field) << std::endl;
                    }
                    break;
                case 55: // SERVER_MESSAGE
                    if (fields.size() >= 2) {
                        std::string sender = Protocol::bytesToString(fields[0]);
                        std::string content = Protocol::bytesToString(fields[1]);
                        std::cout << sender << ": " << content << std::endl;
                    }
                    break;
                default:
                    std::cout << "Mensaje recibido (código " << (int)code << ")" << std::endl;
            }
        }
    });

    ws.onConnect([&connected]() {
        connected = true;
        std::cout << "¡Conectado al servidor!" << std::endl;
    });

    ws.onDisconnect([]() {
        std::cout << "Desconectado del servidor" << std::endl;
    });

    ws.onError([](const std::string& error) {
        std::cerr << "Error: " << error << std::endl;
    });

    if (!ws.connect("localhost", 8080, "TestUser")) {
        std::cerr << "No se pudo conectar al servidor" << std::endl;
        return 1;
    }

    std::thread wsThread([&ws]() {
        ws.run();
    });

    while (!connected && running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (connected) {
        // Registro
        std::vector<std::string> regFields = {"TestUser"};
        auto regMsg = Protocol::serializeMessage(1, regFields);
        ws.send(Protocol::bytesToString(regMsg));

        // Solicitar lista de usuarios
        auto listMsg = Protocol::serializeMessage(2, std::vector<std::string>());
        ws.send(Protocol::bytesToString(listMsg));

        // Bucle de mensajes
        int msgCount = 0;
        while (running && ws.isConnected()) {
            // Alternar entre mensaje privado y broadcast
            std::vector<std::string> fields;
            if (msgCount % 2 == 0) {
                fields = {"all", "Broadcast mensaje " + std::to_string(msgCount)};
            } else {
                fields = {"TestUser", "Mensaje privado " + std::to_string(msgCount)};
            }
            
            auto chatMsg = Protocol::serializeMessage(4, fields);
            ws.send(Protocol::bytesToString(chatMsg));
            
            std::cout << "Mensaje enviado: " << msgCount << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            msgCount++;
        }
    }

    ws.close();  // Esto ahora enviará el mensaje de LOGOUT
    if (wsThread.joinable()) {
        wsThread.join();
    }

    return 0;
} 