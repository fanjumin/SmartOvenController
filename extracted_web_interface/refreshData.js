function refreshData() {
            try {
                const response = await fetch('/status');
                if (response.ok) {
                    const data = await response.json();
                    updateStatusDisplay(data);
                }