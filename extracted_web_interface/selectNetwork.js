function selectNetwork(ssid) {
            document.getElementById('ssid-input').value = ssid;
            document.getElementById('password-input').focus();
            showToast(`已选择网络: ${ssid}