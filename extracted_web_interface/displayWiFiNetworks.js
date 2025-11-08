function displayWiFiNetworks(networks) {
            const container = document.getElementById('wifi-list-container');
            
            if (!networks || networks.length === 0) {
                container.innerHTML = '<div style="text-align: center; padding: 30px; opacity: 0.7;">未发现可用网络</div>';
                return;
            }