@echo off
echo 智能烤箱文件系统OTA更新测试
echo ======================================

set DEVICE_IP=192.168.16.103
set FS_FILE=.pio\build\nodemcuv2\littlefs.bin

echo 1. 检查设备状态...
curl -X GET "http://%DEVICE_IP%/status"
if %errorlevel% neq 0 (
    echo 设备不可访问
    pause
    exit /b 1
)

echo.
echo 2. 检查文件是否存在...
if not exist "%FS_FILE%" (
    echo 文件不存在: %FS_FILE%
    echo 请先运行: pio run -e nodemcuv2 -t buildfs
    pause
    exit /b 1
)

for %%A in ("%FS_FILE%") do set FILE_SIZE=%%~zA
echo 文件大小: %FILE_SIZE% 字节

echo.
echo 3. 上传文件系统镜像...
curl -X POST "http://%DEVICE_IP%/filesystem_update" -F "filesystem=@%FS_FILE%" -v

echo.
echo 4. 等待设备重启(10秒)...
timeout /t 10 /nobreak > nul

echo.
echo 5. 验证更新结果...
curl -I "http://%DEVICE_IP%/index.html"

echo.
echo 测试完成！
pause