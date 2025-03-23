#ifndef CHAT_UI_H
#define CHAT_UI_H

#include "Client.h"
#include <string>
#include <memory>
#include <functional>

class ChatUI {
public:
    ChatUI(std::shared_ptr<Client> client);
    void start();
    void stop();
    bool isRunning() const;

private:
    std::shared_ptr<Client> client_;
    bool running_;

    void showHelp() const;
    void processCommand(const std::string& command);
    std::string getInput() const;
    void displayPrompt() const;

    // Comandos disponibles
    void handleMessage(const std::string& args);
    void handleStatus(const std::string& args);
    void handleList();
    void handleHelp();
    void handleQuit();
};

#endif // CHAT_UI_H
