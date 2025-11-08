function showFactoryResetMessage() {
            const messageDiv = document.createElement('div');
            messageDiv.style.cssText = `
                position: fixed;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
                background: rgba(0, 0, 0, 0.9);
                color: white;
                padding: 30px;
                border-radius: 15px;
                text-align: center;
                z-index: 1000;
                font-size: 16px;
                max-width: 300px;
            `;
            messageDiv.innerHTML = `
                <div style="font-size: 24px; margin-bottom: 15px;">⚠️</div>
                <div style="font-weight: 600; margin-bottom: 10px;">设备正在恢复出厂设置</div>
                <div style="font-size: 14px; opacity: 0.8;">
                    所有设置将被清除，设备将重启并恢复出厂状态。
                </div>
            `;
            document.body.appendChild(messageDiv);
            
            // 5秒后自动刷新页面
            setTimeout(() => {
                window.location.reload();
            }