function restartDevice() {
            const confirmed = confirm('确定要重启设备吗？重启过程可能需要几分钟时间。');
            if (!confirmed) return;
            
            try {
                const response = await fetch('/restart', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    }