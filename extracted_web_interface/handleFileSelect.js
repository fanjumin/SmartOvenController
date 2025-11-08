function handleFileSelect(event) {
            const file = event.target.files[0];
            if (file) {
                // 检查文件类型
                const allowedTypes = ['.bin', '.hex', '.ino'];
                const fileExtension = '.' + file.name.split('.').pop().toLowerCase();
                
                if (!allowedTypes.includes(fileExtension)) {
                    showOtaMessage('请选择有效的固件文件 (.bin, .hex, .ino)', 'error');
                    resetFileInput();
                    return;
                }