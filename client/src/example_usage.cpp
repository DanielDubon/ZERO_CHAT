WebSocket ws;

// Configurar handlers
ws.onMessage([](const std::string& msg) {
    std::cout << "Mensaje recibido: " << msg << std::endl;
});

ws.onConnect([]() {
    std::cout << "Conectado al servidor!" << std::endl;
});

ws.onDisconnect([]() {
    std::cout << "Desconectado del servidor" << std::endl;
});

ws.onError([](const std::string& error) {
    std::cerr << "Error: " << error << std::endl;
});

// Conectar
if (ws.connect("localhost", 8080, "usuario1")) {
    // Enviar un mensaje
    ws.queueMessage("Hola servidor!");
    
    // Iniciar el bucle de eventos
    ws.run();
} 