function showOtaMessage(message, type = '') {
            const messageElement = document.getElementById('ota-message');
            if (messageElement) {
                messageElement.textContent = message;
                messageElement.className = 'ota-message';
                if (type) {
                    messageElement.classList.add(type);
                }