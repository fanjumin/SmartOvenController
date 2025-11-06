@echo off
echo ========================================
echo   智能烤箱温度校准服务器启动脚本
echo ========================================
echo.

echo 正在启动温度校准服务器...
echo 服务器将运行在 http://localhost:8080
echo.

REM 检查Python是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到Python，请先安装Python 3.x
    echo 下载地址: https://www.python.org/downloads/
    pause
    exit /b 1
)

REM 检查必要的Python模块
python -c "import serial" >nul 2>&1
if errorlevel 1 (
    echo 正在安装pyserial模块...
    pip install pyserial
    if errorlevel 1 (
        echo 错误: 安装pyserial失败
        pause
        exit /b 1
    )
)

python -c "import http.server" >nul 2>&1
if errorlevel 1 (
    echo 错误: Python标准库不完整，请重新安装Python
    pause
    exit /b 1
)

echo.
echo 所有依赖检查通过，启动服务器...
echo 按 Ctrl+C 停止服务器
echo.

REM 启动温度校准服务器
python temperature_calibration_server.py

if errorlevel 1 (
    echo.
    echo 服务器启动失败，请检查:
    echo 1. 串口设备是否连接 (默认COM11)
    echo 2. 端口8080是否被占用
    echo 3. 设备固件是否支持校准命令
    echo.
    pause
)

pause