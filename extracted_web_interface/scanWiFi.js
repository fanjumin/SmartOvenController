function scanWiFi() {
            const container = document.getElementById('wifi-list-container');
            container.innerHTML = `
                <div class="loading">
                    <div class="spinner"></div>
                    <div>正在扫描可用网络...</div>
                </div>
            `;

            try {
                const response = await fetch('/scanwifi');
                if (response.ok) {
                    const networks = await response.json();
                    displayWiFiNetworks(networks);
                }