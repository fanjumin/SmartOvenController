function disconnectWiFi() {
            try {
                const response = await fetch('/wificonfig', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    }