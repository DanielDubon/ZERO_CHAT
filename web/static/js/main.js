document.addEventListener('DOMContentLoaded', function() {
    // Referencias a elementos del DOM
    const chatForm = document.getElementById('chat-form');
    const sendButton = document.getElementById('send-button');
    const chatMessages = document.getElementById('chat-messages');
    const usersList = document.getElementById('users-list');
    const msgInput = document.getElementById('msg');
    const recipientSelector = document.getElementById('recipient-selector');
    const statusSelector = document.getElementById('status-selector');
    const usernameElement = document.getElementById('username');
    
    // Obtener nombre de usuario (simulado por ahora)
    const username = 'Usuario' + Math.floor(Math.random() * 1000);
    usernameElement.textContent = username;
    
    // Cargar usuarios
    loadUsers();
    
    // Cargar mensajes
    loadMessages();
    
    // Configurar polling para actualizaciones
    setInterval(checkUpdates, 3000);
    
    // Event listeners
    chatForm.addEventListener('submit', function(e) {
        e.preventDefault(); // Prevenir el envío del formulario
        sendMessage();
    });
    
    // Añadir evento al botón también
    sendButton.addEventListener('click', function() {
        sendMessage();
    });
    
    // Permitir enviar con Enter
    msgInput.addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            e.preventDefault();
            sendMessage();
        }
    });
    
    statusSelector.addEventListener('change', changeStatus);
    
    // Funciones
    function loadUsers() {
        fetch('/api/users')
            .then(response => response.json())
            .then(data => {
                usersList.innerHTML = '';
                data.users.forEach(user => {
                    const li = document.createElement('li');
                    li.innerHTML = `
                        <div>
                            <span class="status-indicator status-${user.status}"></span>
                            ${user.username}
                        </div>
                        <span class="user-status">${getStatusText(user.status)}</span>
                    `;
                    usersList.appendChild(li);
                    
                    // Añadir al selector de destinatarios si no es el usuario actual
                    if (user.username !== username) {
                        const option = document.createElement('option');
                        option.value = user.username;
                        option.textContent = user.username;
                        recipientSelector.appendChild(option);
                    }
                });
            })
            .catch(error => console.error('Error cargando usuarios:', error));
    }
    
    function loadMessages() {
        fetch('/api/messages')
            .then(response => response.json())
            .then(data => {
                chatMessages.innerHTML = '';
                data.messages.forEach(message => {
                    outputMessage(message);
                });
                // Scroll hacia abajo
                chatMessages.scrollTop = chatMessages.scrollHeight;
            })
            .catch(error => console.error('Error cargando mensajes:', error));
    }
    
    function sendMessage() {
        // Obtener mensaje y destinatario
        const msg = msgInput.value;
        const recipient = recipientSelector.value;
        
        if (!msg) return;
        
        console.log("Enviando mensaje:", msg, "a:", recipient);
        
        // Enviar mensaje al servidor
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
            console.log("Respuesta del servidor:", data);
            if (data.status === 'ok') {
                // Añadir mensaje a la interfaz
                const messageObj = {
                    sender: username,
                    content: msg,
                    timestamp: getCurrentTime(),
                    type: recipient === 'all' ? 'broadcast' : 'private'
                };
                outputMessage(messageObj, true);
                
                // Limpiar input y hacer scroll
                msgInput.value = '';
                msgInput.focus();
                chatMessages.scrollTop = chatMessages.scrollHeight;
            }
        })
        .catch(error => console.error('Error enviando mensaje:', error));
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
                    // Actualizar usuarios y mensajes si hay cambios
                    loadUsers();
                    loadMessages();
                }
            })
            .catch(error => console.error('Error en polling:', error));
    }
    
    function outputMessage(message, outgoing = false) {
        const div = document.createElement('div');
        div.classList.add('message');
        div.classList.add(message.type);
        if (outgoing) div.classList.add('outgoing');
        
        div.innerHTML = `
            <div class="meta">
                <span class="sender">${message.sender}</span>
                <span class="time">${message.timestamp}</span>
            </div>
            <p class="text">${message.content}</p>
        `;
        
        chatMessages.appendChild(div);
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
}); 