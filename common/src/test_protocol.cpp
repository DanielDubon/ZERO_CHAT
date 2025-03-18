#include "Protocol.h"
#include <iostream>
#include <vector>
#include <string>

using namespace Protocol;
using namespace std;

int main() {
    // Ejemplo: Crear un mensaje con código 4 ("Mandar un mensaje")
    // con dos campos: destinatario y contenido del mensaje.
    uint8_t code = 4;
    vector<string> fields = {"JP", "Hola, ¿cómo estás?"};

    // Serializar el mensaje.
    vector<uint8_t> serialized = serializeMessage(code, fields);
    
    // Imprimir los bytes serializados.
    cout << "Mensaje serializado (bytes):" << endl;
    for (auto byte : serialized) {
        cout << static_cast<int>(byte) << " ";
    }
    cout << "\n" << endl;
    
    // Deserializar el mensaje.
    uint8_t deserializedCode;
    vector<vector<uint8_t>> deserializedFields;
    bool success = deserializeMessage(serialized, deserializedCode, deserializedFields);
    
    if(success) {
        cout << "Código deserializado: " << static_cast<int>(deserializedCode) << endl;
        cout << "Campos deserializados:" << endl;
        for (auto &field : deserializedFields) {
            cout << " - " << bytesToString(field) << endl;
        }
    } else {
        cout << "Error en la deserialización del mensaje." << endl;
    }
    
    return 0;
}
