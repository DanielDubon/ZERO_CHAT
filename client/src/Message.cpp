#include "Message.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

Message::Message() : timestamp_(std::time(nullptr)), type_("broadcast") {}

Message::Message(const std::string& sender, const std::string& receiver, const std::string& content)
    : sender_(sender), receiver_(receiver), content_(content), timestamp_(std::time(nullptr))
{
    // Si el receptor es "~" o "all", se trata como broadcast; de lo contrario, es privado.
    if (receiver == "~" || receiver == "all") {
        type_ = "broadcast";
    } else {
        type_ = "private";
    }
    std::cout << "Creando mensaje: " << sender << " -> " << receiver 
              << " (" << type_ << "): " << content << std::endl;
}

std::string Message::getSender() const {
    return sender_;
}

std::string Message::getReceiver() const {
    return receiver_;
}

std::string Message::getContent() const {
    return content_;
}

std::string Message::getTimestamp() const {
    std::tm* timeinfo = std::localtime(&timestamp_);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%H:%M");
    return oss.str();
}

std::string Message::getType() const {
    return type_;
} 