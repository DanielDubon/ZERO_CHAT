let ws;
let username;

function connect() {
    ws = new WebSocket(`ws://${window.location.hostname}:8080`);
    
    ws.onopen = () => {
        console.log('Conectado al servidor');
        document.getElementById('status').disabled = false;
    };

    ws.onmessage = (event) => {
        const data = JSON.parse(event.data);
        handleMessage(data);
    };

    ws.onclose = () => {
        console.log('Desconectado del servidor');
        document.getElementById('status').disabled = true;
        setTimeout(connect, 1000);
    };
}

function handleMessage(data) {
    switch (data.type) {
        case 'welcome':
            username = data.username;
            document.getElementById('username').textContent = username;
            break;
            
        case 'message':
            addMessage(data.sender, data.content, data.sender === username);
            break;
            
        case 'userList':
            updateUserList(data.users);
            break;
            
        case 'status':
            updateUserStatus(data.username, data.status);
            break;
    }
}

function addMessage(sender, content, isSent) {
    const messages = document.getElementById('messages');
    const messageDiv = document.createElement('div');
    messageDiv.className = `message ${isSent ? 'sent' : 'received'}`;
    messageDiv.innerHTML = `
        <strong>${sender}:</strong>
        <p>${content}</p>
        <small>${new Date().toLocaleTimeString()}</small>
    `;
    messages.appendChild(messageDiv);
    messages.scrollTop = messages.scrollHeight;
}

function updateUserList(users) {
    const userList = document.getElementById('users');
    userList.innerHTML = '';
    const recipientSelect = document.getElementById('recipient');
    recipientSelect.innerHTML = '<option value="">Seleccionar usuario</option>';
    
    users.forEach(user => {
        if (user !== username) {
            const li = document.createElement('li');
            li.textContent = user;
            li.onclick = () => {
                document.getElementById('messageType').value = 'private';
                recipientSelect.value = user;
                document.getElementById('messageInput').focus();
            };
            userList.appendChild(li);
            
            const option = document.createElement('option');
            option.value = user;
            option.textContent = user;
            recipientSelect.appendChild(option);
        }
    });
}

document.getElementById('messageType').onchange = function() {
    const recipientSelect = document.getElementById('recipient');
    recipientSelect.style.display = this.value === 'private' ? 'block' : 'none';
};

document.getElementById('sendButton').onclick = function() {
    const input = document.getElementById('messageInput');
    const content = input.value.trim();
    if (!content) return;
    
    const messageType = document.getElementById('messageType').value;
    const recipient = document.getElementById('recipient').value;
    
    if (messageType === 'private' && !recipient) {
        alert('Por favor selecciona un destinatario');
        return;
    }
    
    ws.send(JSON.stringify({
        type: messageType,
        recipient: messageType === 'private' ? recipient : 'all',
        content: content
    }));
    
    input.value = '';
    input.focus();
};

document.getElementById('status').onchange = function() {
    ws.send(JSON.stringify({
        type: 'status',
        status: this.value
    }));
};

connect();
