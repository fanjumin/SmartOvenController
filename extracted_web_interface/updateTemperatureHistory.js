function updateTemperatureHistory(currentTemp) {
            temperatureHistory.push({
                temp: currentTemp,
                time: new Date()
            }