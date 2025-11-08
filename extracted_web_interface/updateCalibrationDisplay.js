function updateCalibrationDisplay() {
            const calibrationValue = document.getElementById('current-calibration');
            if (calibrationValue) {
                calibrationValue.textContent = currentCalibration.toFixed(1) + '°C';
            }