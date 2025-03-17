#include "User.h"
#include <ctime>  // Para std::time()

// Constructor por defecto
User::User()
    : username_(""), ipAddress_(""), lastActivity_(std::time(nullptr)), status_("ACTIVO")
{
}

// Constructor parametrizado
User::User(const std::string& username, const std::string& ipAddress)
    : username_(username), ipAddress_(ipAddress), lastActivity_(std::time(nullptr)), status_("ACTIVO")
{
}

// Getters
std::string User::getUsername() const {
    return username_;
}

std::string User::getIpAddress() const {
    return ipAddress_;
}

std::time_t User::getLastActivity() const {
    return lastActivity_;
}

std::string User::getStatus() const {
    return status_;
}

// Setters
void User::setUsername(const std::string& username) {
    username_ = username;
}

void User::setIpAddress(const std::string& ipAddress) {
    ipAddress_ = ipAddress;
}

void User::updateLastActivity() {
    lastActivity_ = std::time(nullptr);  // Actualiza la última actividad al tiempo actual
}

void User::setStatus(const std::string& status) {
    status_ = status;
}