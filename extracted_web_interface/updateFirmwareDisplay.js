function updateFirmwareDisplay() {
            const firmwareElement = document.getElementById('current-firmware');
            if (firmwareElement && deviceStatus.firmwareVersion) {
                firmwareElement.textContent = deviceStatus.firmwareVersion;
            }