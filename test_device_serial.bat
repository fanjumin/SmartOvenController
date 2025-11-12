@echo off
echo 正在检查设备状态...
echo.
echo 尝试连接COM11端口...

REM 使用mode命令检查串口状态
mode COM11

echo.
echo 如果设备正常，应该能看到串口配置信息
echo 如果设备死机，可能会显示错误信息
echo.
pause