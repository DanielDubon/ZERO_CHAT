#include "ChatUI.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

ChatUI::ChatUI(std::shared_ptr<Client> client)
    : client_(client), running_(false) {
        client_->setUpdateUserListCallback([this]() {
            this->refreshUserList();
        });
}

void ChatUI::start() {
    running_ = true;
    showHelp();

    while (running_ && client_->isConnected()) {
        displayPrompt();
        std::string input = getInput();
        processCommand(input);
    }
}

void ChatUI::stop() {
    running_ = false;
}

bool ChatUI::isRunning() const {
    return running_;
}

void ChatUI::showHelp() const {
    std::cout << "\n=== Comandos Disponibles ===\n"
              << "/msg <usuario> <mensaje> - Enviar mensaje privado\n"
              << "/broadcast <mensaje>     - Enviar mensaje a todos\n"
              << "/status <estado>         - Cambiar estado (ACTIVO/OCUPADO/INACTIVO)\n"
              << "/list                    - Listar usuarios conectados\n"
              << "/help                    - Mostrar esta ayuda\n"
              << "/quit                    - Salir\n"
              << "========================\n\n";
}

void ChatUI::refreshUserList() {
    auto users = client_->getConnectedUsers();
    std::cout << "\n*** Lista Actualizada de Usuarios Conectados ***\n";
    for (const auto& user : users) {
        std::cout << "- " << user.first << " (" << user.second << ")\n";
    }
    std::cout << "**************************************************\n";
}

void ChatUI::processCommand(const std::string& input) {
    if (input.empty()) return;

    std::istringstream iss(input);
    std::string command;
    iss >> command;

    if (command[0] != '/') {
        // Si no es un comando, tratar como mensaje broadcast
        handleMessage("all " + input);
        return;
    }

    std::string args;
    std::getline(iss, args);
    if (!args.empty() && args[0] == ' ') {
        args = args.substr(1);  // Eliminar espacio inicial
    }

    if (command == "/msg") {
        handleMessage(args);
    } else if (command == "/broadcast") {
        handleMessage("all " + args);
    } else if (command == "/status") {
        handleStatus(args);
    } else if (command == "/list") {
        handleList();
    } else if (command == "/help") {
        handleHelp();
    } else if (command == "/quit") {
        handleQuit();
    } else {
        std::cout << "Comando desconocido. Use /help para ver los comandos disponibles.\n";
    }
}

std::string ChatUI::getInput() const {
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void ChatUI::displayPrompt() const {
    std::cout << "> ";
}

void ChatUI::handleMessage(const std::string& args) {
    std::istringstream iss(args);
    std::string recipient;
    iss >> recipient;

    std::string message;
    std::getline(iss, message);
    if (!message.empty() && message[0] == ' ') {
        message = message.substr(1);
    }

    if (recipient.empty() || message.empty()) {
        std::cout << "Uso: /msg <usuario> <mensaje> o /broadcast <mensaje>\n";
        return;
    }

    client_->sendMessage(recipient, message);
}

void ChatUI::handleStatus(const std::string& status) {
    if (status.empty()) {
        std::cout << "Uso: /status <ACTIVO|OCUPADO|INACTIVO>\n";
        return;
    }

    if (status != "ACTIVO" && status != "OCUPADO" && status != "INACTIVO") {
        std::cout << "Estado no válido. Use: ACTIVO, OCUPADO o INACTIVO\n";
        return;
    }

    client_->setStatus(status);
}

void ChatUI::handleList() {
    client_->listConnectedUsers();
}

void ChatUI::handleHelp() {
    showHelp();
}

void ChatUI::handleQuit() {
    std::cout << "Cerrando sesión...\n";
    running_ = false;
}
