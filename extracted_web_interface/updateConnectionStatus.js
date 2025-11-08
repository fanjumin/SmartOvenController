function updateConnectionStatus() {
            try {
                const response = await fetch('/status');
                if (response.ok) {
                    const data = await response.json();
                    
                    const statusElement = document.getElementById('connection-status');
                    const ssidElement = document.getElementById('wifi-ssid');
                    const ipElement = document.getElementById('ip-address');
                    const signalElement = document.getElementById('signal-strength');
                    
                    if (data.wifiConnected) {
                        statusElement.textContent = '已连接';
                        statusElement.className = 'status-value connected';
                        ssidElement.textContent = data.wifiSSID || '--';
                        ipElement.textContent = data.ipAddress || '--';
                        signalElement.textContent = (data.rssi || '--') + ' dBm';
                    }