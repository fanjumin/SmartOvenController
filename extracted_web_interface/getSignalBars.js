function getSignalBars(rssi) {
            let bars = 0;
            if (rssi > -50) bars = 4;
            else if (rssi > -60) bars = 3;
            else if (rssi > -70) bars = 2;
            else if (rssi > -80) bars = 1;
            
            let html = '';
            for (let i = 0; i < 4; i++) {
                html += `<div class="signal-bar ${i < bars ? 'active' : ''}