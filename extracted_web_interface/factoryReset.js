function factoryReset() {
            const confirmed = confirm('⚠️ 警告：此操作将清除所有设置并恢复设备到出厂状态！\n\n确定要继续吗？');
            if (!confirmed) return;
            
            const password = prompt('请输入管理员密码确认操作：');
            if (!password) return;
            
            try {
                const response = await fetch('/factoryreset', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    }