function connectToWiFi() {
            const ssid = document.getElementById('ssid-input').value.trim();
            const password = document.getElementById('password-input').value;
            
            if (!ssid) {
                showToast('请输入WiFi名称');
                return;
            }