# Protocolo de Comunicación para el Chat

El protocolo se basa en el envío de mensajes en formato binario con la siguiente estructura:

1. **Código del mensaje (1 byte):**  
   Identifica la acción o evento (por ejemplo, 1: Listar usuarios, 4: Mandar un mensaje, etc.).

2. **Campos del mensaje:**  
   Cada campo se envía en el siguiente formato:  
   - **Longitud (1 byte):** Tamaño en bytes del campo (máximo 255).  
   - **Datos (N bytes):** El contenido del campo.

### Ejemplo de un mensaje

Para un mensaje con código 4 ("Mandar un mensaje") que envía dos campos:
- Campo 1: Destinatario ("JP")
- Campo 2: Mensaje ("Hola, ¿cómo estás?")

La serialización se realiza de la siguiente forma:
1. Se coloca el código (4).
2. Se coloca la longitud del primer campo (2, para "JP") y luego el campo ("JP").
3. Se coloca la longitud del segundo campo (20, por ejemplo) y luego el campo ("Hola, ¿cómo estás?").

### Funciones Implementadas

- `std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::vector<uint8_t>>& fields);`  
  Serializa el mensaje en un vector de bytes.

- `std::vector<uint8_t> serializeMessage(uint8_t code, const std::vector<std::string>& fields);`  
  Versión auxiliar que trabaja con cadenas de texto.

- `bool deserializeMessage(const std::vector<uint8_t>& data, uint8_t &code, std::vector<std::vector<uint8_t>> &fields);`  
  Deserializa el vector de bytes, extrayendo el código y los campos.

- Funciones auxiliares para convertir entre `std::string` y `std::vector<uint8_t>`:
  - `stringToBytes()`
  - `bytesToString()`

Este protocolo garantiza una comunicación estructurada y eficiente, limitando cada campo a 255 bytes para facilitar la gestión de buffers en C/C++.
