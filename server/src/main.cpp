#include "Server.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Si se pasa un puerto como argumento, lo usamos; de lo contrario, usamos el puerto 8080.
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    // Instancia del servidor con el puerto seleccionado.
    Server server(port);
    
    try {
        server.start();
        std::cout << "Servidor corriendo en el puerto " << port << ". Presiona Enter para detenerlo..." << std::endl;
        std::cin.get(); // Espera a que se presione Enter.
        server.stop();
    } catch (const std::exception &e) {
        std::cerr << "Error en el servidor: " << e.what() << std::endl;
    }
    
    return 0;
}
