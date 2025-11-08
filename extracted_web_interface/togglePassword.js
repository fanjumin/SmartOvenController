function togglePassword() {
            const passwordInput = document.getElementById('password-input');
            const toggleBtn = document.querySelector('.toggle-btn');
            
            if (isPasswordVisible) {
                passwordInput.type = 'password';
                toggleBtn.textContent = '显示';
            }