#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>

namespace Protocol {

// Convierte una cadena de texto a un vector de bytes.
std::vector<uint8_t> stringToBytes(const std::string &str);

// Convierte un vector de bytes a una cadena de texto.
std::string bytesToString(const std::vector<uint8_t>& bytes);

// Serializa un mensaje dado un código y una lista de campos en forma de vector de bytes.
// Cada campo se serializa primero con su longitud (1 byte) y luego sus datos.
std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::vector<uint8_t>>& fields);

// Versión auxiliar: Serializa un mensaje tomando cada campo como una cadena de texto.
std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::string>& fields);

// Deserializa un mensaje binario.
// Parámetros de salida: 'code' contendrá el código del mensaje y 'fields' contendrá los campos extraídos.
// Devuelve true si la deserialización es exitosa, false en caso de error.
bool deserializeMessage(const std::vector<uint8_t>& data, uint8_t &code, std::vector<std::vector<uint8_t>> &fields);

} // namespace Protocol

#endif // PROTOCOL_H