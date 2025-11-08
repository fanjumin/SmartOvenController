function updateProgress(percentage) {
            const progressFill = document.getElementById('progress-fill');
            const progressText = document.getElementById('progress-text');
            
            if (progressFill && progressText) {
                progressFill.style.width = percentage + '%';
                progressText.textContent = percentage + '%';
            }