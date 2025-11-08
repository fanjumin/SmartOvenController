function applyTemperature() {
            const tempInput = document.getElementById('temp-input');
            if (tempInput && tempInput.value) {
                const temp = parseInt(tempInput.value);
                if (temp >= 0 && temp <= 300) {
                    setTargetTemp(temp);
                    tempInput.value = ''; // 清空输入框
                }