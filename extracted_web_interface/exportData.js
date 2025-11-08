function exportData() {
            const data = {
                timestamp: new Date().toISOString(),
                temperatureHistory: temperatureHistory,
                deviceInfo: {
                    currentTemp: document.getElementById('current-temp').textContent,
                    targetTemp: document.getElementById('target-temp').textContent,
                    connectionStatus: document.getElementById('connection-status').textContent
                }