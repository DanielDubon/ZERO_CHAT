#include "Message.h"
#include <ctime> // Para std::time()

// Constructor por defecto: inicializa cadenas vacías y marca el tiempo actual
Message::Message()
    : sender_(""), receiver_(""), content_(""), timestamp_(std::time(nullptr))
{
}

// Constructor parametrizado: inicializa con datos y asigna la hora actual
Message::Message(const std::string& sender, const std::string& receiver, const std::string& content)
    : sender_(sender), receiver_(receiver), content_(content), timestamp_(std::time(nullptr))
{
}

// Getters
std::string Message::getSender() const {
    return sender_;
}

std::string Message::getReceiver() const {
    return receiver_;
}

std::string Message::getContent() const {
    return content_;
}

std::time_t Message::getTimestamp() const {
    return timestamp_;
}

// Setters
void Message::setSender(const std::string& sender) {
    sender_ = sender;
}

void Message::setReceiver(const std::string& receiver) {
    receiver_ = receiver;
}

void Message::setContent(const std::string& content) {
    content_ = content;
}
