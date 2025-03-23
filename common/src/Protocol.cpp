#include "Protocol.h"
#include <stdexcept>

namespace Protocol {

// Convierte una cadena a vector de bytes.
std::vector<uint8_t> stringToBytes(const std::string &str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

// Convierte un vector de bytes a cadena.
std::string bytesToString(const std::vector<uint8_t>& bytes) {
    return std::string(bytes.begin(), bytes.end());
}

// Serializa un mensaje: código + (longitud de campo, campo) por cada campo.
std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::string>& fields) {
    std::vector<std::vector<uint8_t>> byteFields;
    for (const auto &str : fields) {
        byteFields.push_back(stringToBytes(str));
    }
    return serializeMessage(code, byteFields);
}

std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::vector<uint8_t>>& fields) {
    std::vector<uint8_t> serialized;
    serialized.push_back(code);
    
    for (const auto &field : fields) {
        if (field.size() > 255) {
            throw std::runtime_error("Field size exceeds 255 bytes");
        }
        serialized.push_back(static_cast<uint8_t>(field.size()));
        serialized.insert(serialized.end(), field.begin(), field.end());
    }
    return serialized;
}

// Deserializa un mensaje recibido.
// Se asume que el primer byte es el código y luego vienen pares: [longitud, datos].
bool deserializeMessage(const std::vector<uint8_t>& data, uint8_t &code, std::vector<std::vector<uint8_t>> &fields) {
    if (data.empty()) {
        return false;
    }
    
    size_t index = 0;
    code = data[index++];
    
    while (index < data.size()) {
        if (index + 1 > data.size()) {
            return false;
        }
        
        uint8_t len = data[index++];
        if (index + len > data.size()) {
            return false;
        }
        
        std::vector<uint8_t> field(data.begin() + index, data.begin() + index + len);
        fields.push_back(field);
        index += len;
    }
    
    return true;
}

} // namespace Protocol
