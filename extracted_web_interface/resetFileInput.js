function resetFileInput() {
            const fileInput = document.getElementById('firmware-file');
            fileInput.value = '';
            selectedFirmwareFile = null;
            document.getElementById('upload-btn').disabled = true;
        }