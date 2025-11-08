function changePassword() {
            const newPassword = prompt('请输入新密码（至少8位）:');
            if (newPassword && newPassword.length >= 8) {
                localStorage.setItem('adminPassword', newPassword);
                alert('密码修改成功！');
            }