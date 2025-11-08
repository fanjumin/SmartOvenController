function updateTemperatureChart() {
            if (temperatureHistory.length < 2) return;
            
            const chartWidth = 300;
            const chartHeight = 120;
            const maxTemp = Math.max(...temperatureHistory.map(d => d.temp));
            const minTemp = Math.min(...temperatureHistory.map(d => d.temp));
            const tempRange = Math.max(maxTemp - minTemp, 10); // æœ€å°èŒƒå›´10åº¦
            
            // ç”ŸæˆæŠ˜çº¿è·¯å¾„
            let linePath = '';
            let areaPath = '';
            
            temperatureHistory.forEach((point, index) => {
                const x = (index / (temperatureHistory.length - 1)) * chartWidth;
                const y = chartHeight - ((point.temp - minTemp) / tempRange) * chartHeight;
                
                if (index === 0) {
                    linePath = `M ${x}