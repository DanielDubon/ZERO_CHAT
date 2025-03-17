#ifndef USER_H
#define USER_H

#include <string>
#include <ctime>  // Para std::time_t

class User {
public:
    // Constructor por defecto
    User();

    // Constructor parametrizado
    User(const std::string& username, const std::string& ipAddress);

    // Getters
    std::string getUsername() const;
    std::string getIpAddress() const;
    std::time_t getLastActivity() const;
    std::string getStatus() const;

    // Setters
    void setUsername(const std::string& username);
    void setIpAddress(const std::string& ipAddress);
    void updateLastActivity();  // Actualiza la última actividad al tiempo actual
    void setStatus(const std::string& status);  // Cambia el estado del usuario

private:
    std::string username_;      // Nombre de usuario
    std::string ipAddress_;     // Dirección IP del usuario
    std::time_t lastActivity_;  // Última vez que el usuario estuvo activo
    std::string status_;        // Estado del usuario (ACTIVO, OCUPADO, INACTIVO)
};

#endif // USER_H