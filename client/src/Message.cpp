#include "Message.h"
#include <ctime>
#include <iomanip>
#include <sstream>

Message::Message()
    : sender_(""), receiver_(""), content_(""), timestamp_(std::time(nullptr))
{
}

Message::Message(const std::string& sender, const std::string& receiver, const std::string& content)
    : sender_(sender), receiver_(receiver), content_(content), timestamp_(std::time(nullptr))
{
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
    return (receiver_ == "all" || receiver_.empty()) ? "broadcast" : "private";
} 