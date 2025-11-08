function updateOtaStatus(status) {
            const statusElement = document.getElementById('ota-status');
            if (statusElement) {
                statusElement.textContent = status;
            }