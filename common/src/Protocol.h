#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>

namespace Protocol {

// Convierte una cadena de texto a un vector de bytes
std::vector<uint8_t> stringToBytes(const std::string &str);

// Convierte un vector de bytes a una cadena de texto
std::string bytesToString(const std::vector<uint8_t>& bytes);

// Serializa un mensaje con campos como strings
std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::string>& fields);

// Serializa un mensaje con campos como bytes
std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::vector<uint8_t>>& fields);

// Deserializa un mensaje
bool deserializeMessage(const std::vector<uint8_t>& data, uint8_t &code, std::vector<std::vector<uint8_t>> &fields);

} // namespace Protocol

#endif // PROTOCOL_H