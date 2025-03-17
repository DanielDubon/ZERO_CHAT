#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>  // Para std::time_t

class Message {
public:
    // Constructores
    Message();
    Message(const std::string& sender, const std::string& receiver, const std::string& content);

    // Métodos básicos (Getters)
    std::string getSender()   const;
    std::string getReceiver() const;
    std::string getContent()  const;
    std::time_t getTimestamp() const;

    // Métodos para modificar (Setters)
    void setSender(const std::string& sender);
    void setReceiver(const std::string& receiver);
    void setContent(const std::string& content);

private:
    std::string sender_;     // Remitente del mensaje
    std::string receiver_;   // Destinatario del mensaje
    std::string content_;    // Contenido del mensaje
    std::time_t timestamp_;  // Marca de tiempo de creación
};

#endif // MESSAGE_H
