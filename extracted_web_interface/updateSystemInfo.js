function updateSystemInfo(data) {
            document.getElementById('uptime').textContent = 
                formatUptime(data.uptime || 0);
            document.getElementById('memory-usage').textContent = 
                (data.freeHeap || 0) + ' KB';
            document.getElementById('last-update').textContent = 
                new Date().toLocaleString();
        }