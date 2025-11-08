function updateTemperatureControlDisplay() {
            // 更新当前温度显示
            const currentTempDisplay = document.getElementById('current-temp-display');
            if (currentTempDisplay) {
                currentTempDisplay.textContent = deviceStatus.currentTemp.toFixed(1) + '°C';
            }