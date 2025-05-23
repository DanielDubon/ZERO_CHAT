#include "WebUI.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "../../external/json/single_include/nlohmann/json.hpp"
#include <thread>

using json = nlohmann::json;
namespace fs = std::filesystem;

WebUI::WebUI(std::shared_ptr<Client> client, int port)
    : client_(client), running_(false), port_(port) {
}

void WebUI::start() {
    setupRoutes();
    running_ = true;
    
    // Crear un hilo para el servidor web
    std::thread([this]() {
        std::cout << "Iniciando servidor web en http://localhost:" << port_ << std::endl;
        
        // Configurar opciones del servidor
        server_.new_task_queue = [](){ return new httplib::ThreadPool(8); };
        
        // Iniciar el servidor
        if (!server_.listen("0.0.0.0", port_)) {  // Usar 0.0.0.0 en lugar de localhost
            std::cerr << "Error al iniciar el servidor web" << std::endl;
        }
    }).detach();
}

void WebUI::stop() {
    running_ = false;
    server_.stop();
}

bool WebUI::isRunning() const {
    return running_;
}

void WebUI::setupRoutes() {
    // Servir archivos estáticos
    server_.set_mount_point("/static", "./web/static");

    // Ruta principal
    server_.Get("/", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetIndex(req, res);
    });

    // Ruta para index.html
    server_.Get("/index.html", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetIndex(req, res);
    });

    // Servir CSS directamente
    server_.Get("/static/css/styles.css", [this](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Solicitud recibida para styles.css" << std::endl;
        res.set_content(R"(
* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

body {
    font-family: 'Roboto', sans-serif;
    background-color: #f5f5f5;
    color: #333;
    line-height: 1.6;
}

.chat-container {
    max-width: 1100px;
    margin: 30px auto;
    background: #fff;
    border-radius: 10px;
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
    overflow: hidden;
    display: flex;
    flex-direction: column;
    height: 80vh;
}

.chat-header {
    background: #4a69bd;
    color: #fff;
    padding: 15px;
    display: flex;
    align-items: center;
    justify-content: space-between;
}

.chat-header h1 {
    font-size: 1.5rem;
    font-weight: 500;
}

.user-status {
    display: flex;
    align-items: center;
    gap: 10px;
}

#username {
    font-weight: 500;
}

#status-selector {
    padding: 5px;
    border-radius: 5px;
    border: none;
    background-color: #fff;
    color: #333;
}

.chat-main {
    display: flex;
    flex: 1;
}

.chat-sidebar {
    background: #f8f9fa;
    color: #333;
    padding: 20px;
    width: 250px;
    border-right: 1px solid #e9ecef;
    overflow-y: auto;
}

.chat-sidebar h3 {
    margin-bottom: 15px;
    font-size: 1.2rem;
    color: #4a69bd;
    font-weight: 500;
}

.chat-sidebar ul {
    list-style: none;
}

.chat-sidebar li {
    padding: 10px;
    margin-bottom: 5px;
    border-radius: 5px;
    background-color: #fff;
    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
    display: flex;
    align-items: center;
    justify-content: space-between;
}

.chat-sidebar .status-indicator {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    margin-right: 10px;
}

.status-ACTIVO {
    background-color: #2ecc71;
}

.status-OCUPADO {
    background-color: #f39c12;
}

.status-INACTIVO {
    background-color: #e74c3c;
}

.chat-messages {
    flex: 1;
    padding: 20px;
    overflow-y: auto;
    max-height: calc(80vh - 130px);
    display: flex;
    flex-direction: column;
}

.message {
    padding: 10px 15px;
    margin-bottom: 15px;
    border-radius: 10px;
    position: relative;
    max-width: 70%;
}

.message.broadcast {
    background-color: #e9ecef;
    color: #333;
    align-self: flex-start;
}

.message.private {
    background-color: #4a69bd;
    color: #fff;
    align-self: flex-start;
    border-left: 4px solid #e74c3c;
}

.message.private::before {
    content: "Privado";
    position: absolute;
    top: -10px;
    left: 10px;
    background-color: #e74c3c;
    color: white;
    font-size: 0.7rem;
    padding: 2px 5px;
    border-radius: 3px;
}

.message.outgoing {
    background-color: #78e08f;
    color: #333;
    align-self: flex-end;
}

.message.outgoing.private {
    background-color: #78e08f;
    color: #333;
    align-self: flex-end;
    border-right: 4px solid #e74c3c;
}

.message.outgoing.private::before {
    content: "Privado";
    position: absolute;
    top: -10px;
    right: 10px;
    background-color: #e74c3c;
    color: white;
    font-size: 0.7rem;
    padding: 2px 5px;
    border-radius: 3px;
}

.message .meta {
    font-size: 0.8rem;
    margin-bottom: 5px;
    display: flex;
    justify-content: space-between;
}

.message .meta .sender {
    font-weight: 500;
}

.message .meta .time {
    color: #777;
}

.chat-form-container {
    padding: 15px;
    background-color: #f8f9fa;
    border-top: 1px solid #e9ecef;
}

#chat-form {
    display: flex;
    gap: 10px;
}

#msg {
    flex: 1;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 5px;
    outline: none;
}

#recipient-selector {
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 5px;
    outline: none;
}

.btn {
    padding: 10px 15px;
    background: #4a69bd;
    color: #fff;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    transition: background 0.3s;
}

.btn:hover {
    background: #3c5aa6;
}

@media (max-width: 700px) {
    .chat-main {
        flex-direction: column;
    }
    
    .chat-sidebar {
        width: 100%;
        height: 150px;
    }
    
    .message {
        max-width: 90%;
    }
}
        )", "text/css");
        
        std::cout << "Respuesta enviada para styles.css" << std::endl;
    });

    // Servir JavaScript directamente
    server_.Get("/static/js/main.js", [this](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Solicitud recibida para main.js" << std::endl;
        
        // Obtener el nombre de usuario real del cliente usando el getter
        std::string realUsername = client_->getUsername();
        
        res.set_content(R"(
// Mantener un registro de los mensajes ya mostrados
const messageIds = new Set();

// Añadir esta variable global al inicio del script
let selectedRecipient = 'all';

function loadMessages() {
    fetch('/api/messages')
        .then(response => response.json())
        .then(data => {
            console.log('Cargando mensajes, cantidad recibida:', data.messages.length);
            
            if (data.messages.length > 0) {
                // Guardar la posición actual del scroll
                const scrollHeightBefore = chatMessages.scrollHeight;
                const currentScrollTop = chatMessages.scrollTop;
                
                // Limpiar todos los mensajes existentes
                chatMessages.innerHTML = '';
                messageIds.clear();
                
                // Añadir todos los mensajes
                data.messages.forEach(message => {
                    const messageId = message.timestamp + '-' + message.sender + '-' + message.content;
                    if (!messageIds.has(messageId)) {
                        messageIds.add(messageId);
                        const isOutgoing = message.sender === username;
                        
                        if (message.receiver === "~") {
                            message.type = 'broadcast';
                        } else {
                            message.type = 'private';
                        }
                        
                        outputMessage(message, isOutgoing);
                    }
                });
                
                // Verificar si estamos cerca del final antes de hacer scroll
                const nearBottom = (scrollHeightBefore - currentScrollTop - chatMessages.clientHeight) < 50;
                if (nearBottom) {
                    chatMessages.scrollTop = chatMessages.scrollHeight;
                }
            }
        })
        .catch(error => console.error('Error cargando mensajes:', error));
}

document.addEventListener('DOMContentLoaded', function() {
    // Referencias a elementos del DOM
    const chatForm = document.getElementById('chat-form');
    const sendButton = document.getElementById('send-button');
    const chatMessages = document.getElementById('chat-messages');
    const usersList = document.getElementById('users-list');
    const msgInput = document.getElementById('msg');
    const recipientSelector = document.getElementById('recipient-selector');
    const statusSelector = document.getElementById('status-selector');
    
    // Usar el nombre de usuario real
    const username = ')" + realUsername + R"(';
    
    // Cargar usuarios
    loadUsers();
    
    // Cargar mensajes
    loadMessages();
    
    // Configurar polling para actualizaciones
    setInterval(checkUpdates, 100);
    
    // Event listeners
    chatForm.addEventListener('submit', sendMessage);
    
    statusSelector.addEventListener('change', changeStatus);
    
    // Modificar el evento change del selector de destinatarios
    recipientSelector.addEventListener('change', function(e) {
        selectedRecipient = e.target.value;
        console.log('Destinatario seleccionado:', selectedRecipient);
    });
    
    // Funciones
    function loadUsers() {
        console.log("Solicitando lista de usuarios...");
        fetch('/api/users')
            .then(response => response.json())
            .then(data => {
                console.log("Usuarios recibidos:", data.users);
                
                const usersList = document.getElementById('users');
                if (!usersList) {
                    console.error("No se encontró el elemento con id 'users'");
                    return;
                }
                
                // Limpiar la lista de usuarios
                usersList.innerHTML = '';
                
                // Guardar el destinatario seleccionado actualmente
                const currentRecipient = recipientSelector.value;
                
                // Actualizar el selector de destinatarios preservando la selección
                recipientSelector.innerHTML = '<option value="all">Todos</option>';
                
                // Añadir cada usuario a la lista y al selector
                data.users.forEach(user => {
                    console.log("Agregando usuario a la lista:", user.username);
                    
                    // Añadir a la lista de usuarios
                    const li = document.createElement('li');
                    li.innerHTML = `
                        <div>
                            <span class="status-indicator status-${user.status}"></span>
                            ${user.username}
                        </div>
                        <span>${getStatusText(user.status)}</span>
                    `;
                    usersList.appendChild(li);
                    
                    // Añadir al selector de destinatarios (excepto el usuario actual)
                    if (user.username !== username) {
                        const option = document.createElement('option');
                        option.value = user.username;
                        option.textContent = user.username;
                        // Restaurar la selección previa si el usuario aún está disponible
                        if (user.username === currentRecipient) {
                            option.selected = true;
                        }
                        recipientSelector.appendChild(option);
                    }
                });
                
                // Si el destinatario anterior ya no está disponible, seleccionar "Todos"
                if (!Array.from(recipientSelector.options).some(opt => opt.value === currentRecipient)) {
                    recipientSelector.value = 'all';
                }
            })
            .catch(error => console.error('Error cargando usuarios:', error));
    }
    
    function changeStatus(e) {
        const status = e.target.value;
        
        fetch('/api/status', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                status: status
            })
        })
        .then(response => response.json())
        .then(data => {
            if (data.status === 'ok') {
                console.log('Estado actualizado a:', status);
            }
        })
        .catch(error => console.error('Error cambiando estado:', error));
    }
    
    function checkUpdates() {
        fetch('/api/updates')
            .then(response => response.json())
            .then(data => {
                if (data.type === 'update') {
                    console.log('Actualizando lista de usuarios...');
                    loadUsers();
                    loadMessages();
                }
            })
            .catch(error => console.error('Error en polling:', error));
    }
    
    function sendMessage(e) {
        e.preventDefault();
        
        const msg = msgInput.value.trim();
        const recipient = recipientSelector.value;
        
        if (msg) {
            console.log('Enviando mensaje a:', recipient, 'Contenido:', msg);
            
            fetch('/api/message', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    recipient: recipient,
                    content: msg
                })
            })
            .then(response => response.json())
            .then(data => {
                if (data.status === 'ok') {
                    console.log('Mensaje enviado con éxito, tipo:', data.type);
                    
                    // Crear objeto de mensaje para mostrar inmediatamente
                    const messageObj = {
                        sender: username,
                        receiver: recipient,
                        content: msg,
                        timestamp: getCurrentTime(),
                        type: recipient === 'all' ? 'broadcast' : 'private'
                    };
                    
                    // Mostrar mensaje en la interfaz
                    outputMessage(messageObj, true);
                    
                    // Limpiar input y mantener foco
                    msgInput.value = '';
                    msgInput.focus();
                } else {
                    console.error('Error al enviar mensaje:', data.message);
                    alert('Error al enviar mensaje: ' + data.message);
                }
            })
            .catch(error => {
                console.error('Error enviando mensaje:', error);
                alert('Error al enviar mensaje: ' + error.message);
            });
        }
    }
    
    function outputMessage(message, outgoing = false) {
        const div = document.createElement('div');
        div.classList.add('message');
        div.classList.add(message.type);
        
        if (outgoing) {
            div.classList.add('outgoing');
        }
        
        let senderText = message.sender;
        if (message.type === 'private') {
            senderText += ' → ' + (message.receiver === username ? 'ti' : message.receiver);
        }
        
        div.innerHTML = `
            <div class="meta">
                <span class="sender">${senderText}</span>
                <span class="time">${message.timestamp}</span>
            </div>
            <p class="text">${message.content}</p>
        `;
        
        chatMessages.appendChild(div);
        
        // Solo hacer scroll si estamos muy cerca del final (20 píxeles o menos)
        const isVeryNearBottom = chatMessages.scrollHeight - chatMessages.scrollTop - chatMessages.clientHeight <= 20;
        if (isVeryNearBottom || outgoing) {
            chatMessages.scrollTop = chatMessages.scrollHeight;
        }
    }
    
    function getCurrentTime() {
        const now = new Date();
        return `${now.getHours()}:${String(now.getMinutes()).padStart(2, '0')}`;
    }
    
    function getStatusText(status) {
        switch (status) {
            case 'ACTIVO': return 'Activo';
            case 'OCUPADO': return 'Ocupado';
            case 'INACTIVO': return 'Inactivo';
            default: return status;
        }
    }
});)", "application/javascript");
        
        std::cout << "Respuesta enviada para main.js" << std::endl;
    });

    // API endpoints
    server_.Get("/api/users", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetUsers(req, res);
    });

    server_.Post("/api/message", [this](const httplib::Request& req, httplib::Response& res) {
        handlePostMessage(req, res);
    });

    server_.Post("/api/status", [this](const httplib::Request& req, httplib::Response& res) {
        handlePostStatus(req, res);
    });

    // Endpoint para obtener mensajes recientes
    server_.Get("/api/messages", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetMessages(req, res);
    });

    // WebSocket endpoint usando long polling como alternativa
    server_.Get("/api/updates", [this](const httplib::Request& req, httplib::Response& res) {
        handleWebSocket(req, res);
    });
}

void WebUI::handleGetIndex(const httplib::Request& req, httplib::Response& res) {
    std::cout << "Solicitud recibida para index.html" << std::endl;
    
    // Verificar si el cliente está conectado
    if (!client_->isConnected()) {
        std::string errorHtml = "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <title>Error de Conexion</title>\n"
            "    <style>\n"
            "        body {\n"
            "            font-family: Arial, sans-serif;\n"
            "            margin: 0;\n"
            "            padding: 20px;\n"
            "            background-color: #f0f0f0;\n"
            "            display: flex;\n"
            "            justify-content: center;\n"
            "            align-items: center;\n"
            "            min-height: 100vh;\n"
            "        }\n"
            "        .error-container {\n"
            "            background-color: white;\n"
            "            padding: 2rem;\n"
            "            border-radius: 8px;\n"
            "            box-shadow: 0 2px 4px rgba(0,0,0,0.1);\n"
            "            text-align: center;\n"
            "            max-width: 500px;\n"
            "            width: 100%;\n"
            "        }\n"
            "        h1 {\n"
            "            color: #e74c3c;\n"
            "            margin-bottom: 1rem;\n"
            "        }\n"
            "        p {\n"
            "            color: #666;\n"
            "            line-height: 1.6;\n"
            "            margin-bottom: 1rem;\n"
            "        }\n"
            "        .button {\n"
            "            display: inline-block;\n"
            "            padding: 0.8rem 1.5rem;\n"
            "            background-color: #2c3e50;\n"
            "            color: white;\n"
            "            text-decoration: none;\n"
            "            border-radius: 4px;\n"
            "            margin-top: 1rem;\n"
            "        }\n"
            "        .button:hover {\n"
            "            background-color: #3c5aa6;\n"
            "        }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class=\"error-container\">\n"
            "        <h1>Error de Conexion</h1>\n"
            "        <p>No se pudo establecer la conexion con el servidor. El nombre de usuario ya esta en uso.</p>\n"
            "        <p>Por favor, intente conectarse con un nombre de usuario diferente.</p>\n"
            "        <a href=\"javascript:window.close()\" class=\"button\">Cerrar</a>\n"
            "    </div>\n"
            "</body>\n"
            "</html>";
        res.set_content(errorHtml, "text/html");
        return;
    }
    
    // Obtener el nombre de usuario real del cliente
    std::string realUsername = client_->getUsername();
    
    // Crear el HTML con el nombre de usuario real
    std::string html = R"(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZeroChat</title>
    <link rel="stylesheet" href="/static/css/styles.css">
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500&display=swap" rel="stylesheet">
</head>
<body>
    <div class="chat-container">
        <header class="chat-header">
            <h1>ZeroChat</h1>
            <div class="user-status">
                <span id="username">)" + realUsername + R"(</span>
                <select id="status-selector">
                    <option value="ACTIVO">Activo</option>
                    <option value="OCUPADO">Ocupado</option>
                    <option value="INACTIVO">Inactivo</option>
                </select>
            </div>
        </header>
        <main class="chat-main">
            <div class="chat-sidebar">
                <h3>Usuarios en línea</h3>
                <ul id="users"></ul>
            </div>
            <div class="chat-messages" id="chat-messages"></div>
        </main>
        <div class="chat-form-container">
            <form id="chat-form">
                <select id="recipient-selector">
                    <option value="all">Todos</option>
                </select>
                <input
                    id="msg"
                    type="text"
                    placeholder="Escribe un mensaje..."
                    required
                    autocomplete="off"
                />
                <button class="btn">Enviar</button>
            </form>
        </div>
    </div>

    <script>
        const chatForm = document.getElementById('chat-form');
        const chatMessages = document.getElementById('chat-messages');
        const usersList = document.getElementById('users');
        const msgInput = document.getElementById('msg');
        const recipientSelector = document.getElementById('recipient-selector');
        const statusSelector = document.getElementById('status-selector');
        const username = document.getElementById('username').textContent;
        
        // Conjunto para almacenar IDs de mensajes y evitar duplicados
        const messageIds = new Set();
        
        // Variable global para el destinatario seleccionado
        let selectedRecipient = 'all';
        
        // Cargar usuarios y mensajes al inicio
        loadUsers();
        loadMessages();
        
        // Configurar polling para actualizaciones
        setInterval(checkUpdates, 100);
        
        // Event listeners
        chatForm.addEventListener('submit', sendMessage);
        
        // Evento change para el selector de destinatarios
        recipientSelector.addEventListener('change', function(e) {
            selectedRecipient = e.target.value;
            console.log('Destinatario seleccionado:', selectedRecipient);
        });
        
        statusSelector.addEventListener('change', changeStatus);
        
        // Funciones
        function loadUsers() {
            console.log("Solicitando lista de usuarios...");
            fetch('/api/users')
                .then(response => response.json())
                .then(data => {
                    console.log("Usuarios recibidos:", data.users);
                    
                    const usersList = document.getElementById('users');
                    if (!usersList) {
                        console.error("No se encontró el elemento con id 'users'");
                        return;
                    }
                    
                    // Limpiar la lista de usuarios
                    usersList.innerHTML = '';
                    
                    // Guardar el destinatario seleccionado actualmente
                    const currentRecipient = recipientSelector.value;
                    
                    // Actualizar el selector de destinatarios preservando la selección
                    recipientSelector.innerHTML = '<option value="all">Todos</option>';
                    
                    // Añadir cada usuario a la lista y al selector
                    data.users.forEach(user => {
                        console.log("Agregando usuario a la lista:", user.username);
                        
                        // Añadir a la lista de usuarios
                        const li = document.createElement('li');
                        li.innerHTML = `
                            <div>
                                <span class="status-indicator status-${user.status}"></span>
                                ${user.username}
                            </div>
                            <span>${getStatusText(user.status)}</span>
                        `;
                        usersList.appendChild(li);
                        
                        // Añadir al selector de destinatarios (excepto el usuario actual)
                        if (user.username !== username) {
                            const option = document.createElement('option');
                            option.value = user.username;
                            option.textContent = user.username;
                            // Restaurar la selección previa si el usuario aún está disponible
                            if (user.username === currentRecipient) {
                                option.selected = true;
                            }
                            recipientSelector.appendChild(option);
                        }
                    });
                    
                    // Si el destinatario anterior ya no está disponible, seleccionar "Todos"
                    if (!Array.from(recipientSelector.options).some(opt => opt.value === currentRecipient)) {
                        recipientSelector.value = 'all';
                    }
                })
                .catch(error => console.error('Error cargando usuarios:', error));
        }
        
        function changeStatus(e) {
            const status = e.target.value;
            
            fetch('/api/status', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    status: status
                })
            })
            .then(response => response.json())
            .then(data => {
                if (data.status === 'ok') {
                    console.log('Estado actualizado a:', status);
                }
            })
            .catch(error => console.error('Error cambiando estado:', error));
        }
        
        function checkUpdates() {
            fetch('/api/updates')
                .then(response => response.json())
                .then(data => {
                    if (data.type === 'update') {
                        console.log('Actualizando lista de usuarios...');
                        loadUsers();
                        loadMessages();
                    }
                })
                .catch(error => console.error('Error en polling:', error));
        }
        
        function loadMessages() {
            fetch('/api/messages')
                .then(response => response.json())
                .then(data => {
                    console.log('Cargando mensajes, cantidad recibida:', data.messages.length);
                    
                    if (data.messages.length > 0) {
                        // Guardar la posición actual del scroll
                        const scrollHeightBefore = chatMessages.scrollHeight;
                        const currentScrollTop = chatMessages.scrollTop;
                        
                        // Limpiar todos los mensajes existentes
                        chatMessages.innerHTML = '';
                        messageIds.clear();
                        
                        // Añadir todos los mensajes
                        data.messages.forEach(message => {
                            const messageId = message.timestamp + '-' + message.sender + '-' + message.content;
                            if (!messageIds.has(messageId)) {
                                messageIds.add(messageId);
                                const isOutgoing = message.sender === username;
                                
                                if (message.receiver === "~") {
                                    message.type = 'broadcast';
                                } else {
                                    message.type = 'private';
                                }
                                
                                outputMessage(message, isOutgoing);
                            }
                        });
                        
                        // Verificar si estamos cerca del final antes de hacer scroll
                        const nearBottom = (scrollHeightBefore - currentScrollTop - chatMessages.clientHeight) < 50;
                        if (nearBottom) {
                            chatMessages.scrollTop = chatMessages.scrollHeight;
                        }
                    } else {
                        console.log('No hay nuevos mensajes para mostrar');
                    }
                })
                .catch(error => console.error('Error cargando mensajes:', error));
        }
        
        function sendMessage(e) {
            e.preventDefault();
            
            const msg = msgInput.value.trim();
            const recipient = recipientSelector.value;
            
            if (msg) {
                console.log('Enviando mensaje a:', recipient, 'Contenido:', msg);
                
                fetch('/api/message', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        recipient: recipient,
                        content: msg
                    })
                })
                .then(response => response.json())
                .then(data => {
                    if (data.status === 'ok') {
                        console.log('Mensaje enviado con éxito, tipo:', data.type);
                        
                        // Crear objeto de mensaje para mostrar inmediatamente
                        const messageObj = {
                            sender: username,
                            receiver: recipient,
                            content: msg,
                            timestamp: getCurrentTime(),
                            type: recipient === 'all' ? 'broadcast' : 'private'
                        };
                        
                        // Mostrar mensaje en la interfaz
                        outputMessage(messageObj, true);
                        
                        // Limpiar input y mantener foco
                        msgInput.value = '';
                        msgInput.focus();
                    } else {
                        console.error('Error al enviar mensaje:', data.message);
                        alert('Error al enviar mensaje: ' + data.message);
                    }
                })
                .catch(error => {
                    console.error('Error enviando mensaje:', error);
                    alert('Error al enviar mensaje: ' + error.message);
                });
            }
        }
        
        function outputMessage(message, outgoing = false) {
            const div = document.createElement('div');
            div.classList.add('message');
            div.classList.add(message.type);
            
            if (outgoing) {
                div.classList.add('outgoing');
            }
            
            let senderText = message.sender;
            if (message.type === 'private') {
                senderText += ' → ' + (message.receiver === username ? 'ti' : message.receiver);
            }
            
            div.innerHTML = `
                <div class="meta">
                    <span class="sender">${senderText}</span>
                    <span class="time">${message.timestamp}</span>
                </div>
                <p class="text">${message.content}</p>
            `;
            
            chatMessages.appendChild(div);
            
            // Solo hacer scroll si estamos muy cerca del final (20 píxeles o menos)
            const isVeryNearBottom = chatMessages.scrollHeight - chatMessages.scrollTop - chatMessages.clientHeight <= 20;
            if (isVeryNearBottom || outgoing) {
                chatMessages.scrollTop = chatMessages.scrollHeight;
            }
        }
        
        function getCurrentTime() {
            const now = new Date();
            return `${now.getHours()}:${String(now.getMinutes()).padStart(2, '0')}`;
        }
        
        function getStatusText(status) {
            switch (status) {
                case 'ACTIVO': return 'Activo';
                case 'OCUPADO': return 'Ocupado';
                case 'INACTIVO': return 'Inactivo';
                default: return status;
            }
        }
    </script>
</body>
</html>)";
    
    res.set_content(html, "text/html");
    std::cout << "Respuesta enviada para index.html" << std::endl;
}

void WebUI::handleGetMessages(const httplib::Request& req, httplib::Response& res) {
    // Crear un array de mensajes para devolver
    json messages = json::array();
    
    // Obtener los mensajes del cliente
    auto clientMessages = client_->getMessages();
    
    std::cout << "Número de mensajes del cliente: " << clientMessages.size() << std::endl;
    
    for (const auto& msg : clientMessages) {
        // Determinar el tipo de mensaje basado en el destinatario
        std::string type = (msg.getReceiver() == "~") ? "broadcast" : "private";
        
        json message = {
            {"sender", msg.getSender()},
            {"receiver", msg.getReceiver()},
            {"content", msg.getContent()},
            {"timestamp", msg.getTimestamp()},
            {"type", type}
        };
        messages.push_back(message);
        
        std::cout << "Mensaje procesado: " << msg.getSender() << " -> " 
                  << msg.getReceiver() << " (" << type << "): " 
                  << msg.getContent() << std::endl;
    }
    
    // Crear la respuesta
    json response = {
        {"messages", messages}
    };
    
    res.set_content(response.dump(), "application/json");
}

void WebUI::handlePostMessage(const httplib::Request& req, httplib::Response& res) {
    try {
        // Parsear el cuerpo de la solicitud
        json body = json::parse(req.body);
        
        // Obtener destinatario y contenido
        std::string recipient = body["recipient"];
        std::string content = body["content"];
        
        // Convertir "all" a "~" para broadcast
        if (recipient == "all") {
            recipient = "~";
        }
        
        // Enviar mensaje usando el cliente
        client_->sendMessage(recipient, content);
        
        // Responder con éxito y el tipo de mensaje
        json response = {
            {"status", "ok"},
            {"message", "Mensaje enviado correctamente"},
            {"type", (recipient == "~") ? "broadcast" : "private"}
        };
        
        res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
        // En caso de error, enviar respuesta de error
        json response = {
            {"status", "error"},
            {"message", std::string("Error al enviar mensaje: ") + e.what()}
        };
        res.set_content(response.dump(), "application/json");
    }
}

void WebUI::handleGetUsers(const httplib::Request& req, httplib::Response& res) {
    // Solicitar la lista de usuarios al servidor
    client_->listConnectedUsers();
    
    // Crear un array de usuarios para devolver
    json users = json::array();
    
    // Obtener la lista de usuarios del cliente
    auto connectedUsers = client_->getConnectedUsers();
    
    // Si la lista está vacía, añadir al menos el usuario actual
    if (connectedUsers.empty()) {
        json currentUser = {
            {"username", client_->getUsername()},
            {"status", client_->getStatus()}
        };
        users.push_back(currentUser);
    } else {
        // Añadir todos los usuarios conectados
        for (const auto& [username, status] : connectedUsers) {
            json user = {
                {"username", username},
                {"status", status}
            };
            users.push_back(user);
        }
    }
    
    // Crear la respuesta
    json response = {
        {"users", users}
    };
    
    res.set_content(response.dump(), "application/json");
    
    // Imprimir información de depuración
    std::cout << "Respuesta de usuarios: " << response.dump() << std::endl;
}

void WebUI::handlePostStatus(const httplib::Request& req, httplib::Response& res) {
    // Parsear el cuerpo de la solicitud
    json body = json::parse(req.body);
    
    // Obtener el nuevo estado
    std::string status = body["status"];
    
    // Actualizar el estado del cliente
        client_->setStatus(status);
        
    // Responder con éxito
    json response = {
        {"status", "ok"},
        {"message", "Estado actualizado correctamente"}
    };
    
    res.set_content(response.dump(), "application/json");
}

void WebUI::handleWebSocket(const httplib::Request& req, httplib::Response& res) {
    // Forzar actualización de mensajes y usuarios
    json response = {
        {"type", "update"},
        {"timestamp", std::time(nullptr)}
    };
    
    res.set_content(response.dump(), "application/json");
} 