function updateStatusDisplay(data) {
            deviceStatus = {
                currentTemp: data.temperature || 0,
                targetTemp: data.target_temperature || 180,
                heatingEnabled: data.heating_enabled || false,
                wifiConnected: data.wifi_connected || false,
                wifiSSID: data.wifiSSID || '',
                ipAddress: data.ip_address || '',
                deviceId: data.device_id || '',
                firmwareVersion: data.firmware_version || '',
                calibration: data.calibration || 0.0
            }