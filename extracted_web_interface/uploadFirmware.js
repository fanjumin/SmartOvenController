function uploadFirmware() {
            if (!selectedFirmwareFile) {
                showOtaMessage('请先选择固件文件', 'error');
                return;
            }