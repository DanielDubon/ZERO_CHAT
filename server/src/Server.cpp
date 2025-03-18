#include "Protocol.h"
#include <vector>
#include <cstdint>
#include <iostream>

void handleClientData(const std::vector<uint8_t>& data) {
    uint8_t code;
    std::vector<std::vector<uint8_t>> fields;
    if (!Protocol::deserializeMessage(data, code, fields)) {
        std::cerr << "Error: Mensaje mal formado." << std::endl;
        return;
    }
    
    // Procesar el mensaje según su código.
    switch(code) {
        case 1:
            // Código 1: Listar usuarios
            std::cout << "Solicitud de listado de usuarios." << std::endl;
            // Lógica para enviar la lista de usuarios...
            break;
        case 4:
            // Código 4: Mandar un mensaje
            if(fields.size() >= 2) {
                std::string destinatario = Protocol::bytesToString(fields[0]);
                std::string mensaje = Protocol::bytesToString(fields[1]);
                std::cout << "Mensaje de " << destinatario << ": " << mensaje << std::endl;
                // Lógica para reenviar el mensaje...
            }
            break;
        // Otros casos según lo definido en el protocolo.
        default:
            std::cerr << "Código desconocido: " << static_cast<int>(code) << std::endl;
            break;
    }
}
