function getStoredPassword() {
            return localStorage.getItem('adminPassword') || DEFAULT_PASSWORD;
        }