@echo off
cls

echo ========== 从设备获取最新HTML文件 ==========
echo.

:: 设置默认IP
set DEVICE_IP=192.168.4.1

:: 创建目录
md device_files 2>nul

echo 请确保设备已连接到WiFi或处于AP模式
echo 设备默认IP: %DEVICE_IP%
echo.

:: 获取文件列表
set FILES=index.html device_status.html login.html settings_help.html wifi_config.html smart_oven_controller.html temperature_control.html

:: 使用curl获取文件
echo 正在获取文件...
echo ----------------------------------------

for %%f in (%FILES%) do (
    echo 获取 %%f...
    
    :: 尝试直接路径
    curl -s http://%DEVICE_IP%/%%f -o device_files\%%f
    if exist device_files\%%f (
        echo ✓ 成功获取 %%f
    ) else (
        :: 尝试data路径
        echo 尝试data路径...
        curl -s http://%DEVICE_IP%/data/%%f -o device_files\%%f
        if exist device_files\%%f (
            echo ✓ 成功获取 %%f
        ) else (
            echo ✗ 无法获取 %%f
        )
    )
    echo.
)

echo ----------------------------------------
echo 获取完成！请查看device_files目录
echo.
echo 提示:
echo 1. 如果无法获取文件，请检查设备IP和连接状态
echo 2. 手动将device_files目录中的文件复制到data目录以更新

echo.
dir device_files\*.html

echo.
pause