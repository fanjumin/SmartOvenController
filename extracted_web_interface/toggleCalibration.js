function toggleCalibration() {
            const content = document.getElementById('calibration-content');
            const toggleBtn = document.getElementById('calibration-toggle');
            
            if (content.style.display === 'none') {
                content.style.display = 'block';
                toggleBtn.textContent = '收起';
            }