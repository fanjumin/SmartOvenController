function applyCalibration() {
            const calibrationInput = document.getElementById('calibration-input');
            if (!calibrationInput || !calibrationInput.value) {
                alert('请先输入校准值');
                return;
            }