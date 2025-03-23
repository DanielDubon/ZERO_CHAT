#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>

class Message {
public:
    Message();
    Message(const std::string& sender, const std::string& receiver, const std::string& content);
    
    std::string getSender() const;
    std::string getReceiver() const;
    std::string getContent() const;
    std::string getTimestamp() const;
    std::string getType() const;  // "broadcast" o "private"
    
private:
    std::string sender_;
    std::string receiver_;
    std::string content_;
    std::time_t timestamp_;
};

#endif // MESSAGE_H 