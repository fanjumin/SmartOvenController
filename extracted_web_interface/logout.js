function logout() {
            sessionStorage.removeItem('isLoggedIn');
            window.location.href = 'login.html';
        }