@echo off
chcp 65001 > nul
cls

echo ==================================================
echo          智能烤箱控制器 - 串口连接检查工具          
echo ==================================================
echo.

REM 列出所有可用的串口
echo 正在检测可用的串口...
echo.
mode | find "COM" /i
if errorlevel 1 (
    echo 未发现可用的串口设备
goto exit
)

echo.
echo 请输入要测试的串口名称（例如：COM3）：
set /p port="串口名称: "

REM 检查用户输入是否以COM开头
if not "%port:~0,3%" == "COM" (
    echo 错误：请输入正确的串口名称，格式如COM3
    goto exit
)

REM 尝试设置串口参数来测试连接
echo.
echo 正在测试连接到 %port%...
echo.
mode %port% BAUD=115200 PARITY=N DATA=8 STOP=1
if errorlevel 1 (
    echo 错误：无法连接到 %port%
    echo 可能的原因：
    echo  1. 串口名称不正确
    echo  2. 设备未连接或未开机
    echo  3. 串口被其他程序占用
    goto exit
) else (
    echo 成功连接到 %port%
    echo 串口参数设置为：波特率115200，无校验，8数据位，1停止位
)

echo.
echo 提示：
if exist "serial_communication_tool.py" (
    echo 1. 要进行完整的串口通信，请确保已安装Python
    echo 2. 安装所需依赖：pip install pyserial
    echo 3. 运行：python serial_communication_tool.py
)

:exit
echo.
echo 按任意键退出...
pause > nul