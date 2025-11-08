@echo off
cls

echo ========== 从设备获取最新HTML文件 ==========
echo.
echo 提示: 请确保设备已连接且IP地址正确
echo 默认设备IP为 192.168.4.1 (AP模式)
echo.

:: 设置默认IP
set DEFAULT_IP=192.168.4.1

:: 获取用户输入的IP
set DEVICE_IP=
set /p DEVICE_IP=请输入设备IP地址 [默认: %DEFAULT_IP%]: 
if "%DEVICE_IP%"=="" set DEVICE_IP=%DEFAULT_IP%

echo 使用设备IP: %DEVICE_IP%
echo.

:: 创建目录
md data_latest 2>nul
md data_backup_%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2% 2>nul

:: 定义要获取的文件列表
set FILES=index.html device_status.html login.html settings_help.html wifi_config.html smart_oven_controller.html temperature_control.html

:: 备份现有文件
if exist data\*.html (
    echo 正在备份现有文件...
    for %%f in (%FILES%) do (
        if exist data\%%f (
            copy data\%%f data_backup_%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%\%%f >nul
        )
    )
)

echo 开始获取文件...
echo ----------------------------------------

:: 获取文件
set SUCCESS_COUNT=0
for %%f in (%FILES%) do (
    echo 获取 %%f...
    
    :: 尝试直接路径
    powershell -Command "try { Invoke-WebRequest -Uri 'http://%DEVICE_IP%/%%f' -OutFile 'data_latest\%%f' -TimeoutSec 5; Write-Host '✓ 成功' } catch { Write-Host '✗ 失败'; exit 1 }" >nul 2>&1
    
    if errorlevel 1 (
        :: 尝试data路径
        echo 尝试从data路径获取...
        powershell -Command "try { Invoke-WebRequest -Uri 'http://%DEVICE_IP%/data/%%f' -OutFile 'data_latest\%%f' -TimeoutSec 5; Write-Host '✓ 成功' } catch { Write-Host '✗ 失败' }" >nul 2>&1
    )
    
    if exist data_latest\%%f (
        set /a SUCCESS_COUNT+=1
    )
    echo.
)

echo ----------------------------------------
echo 获取完成！成功: %SUCCESS_COUNT%/%FILES: =+1% 个文件
echo.
echo 最新文件已保存到: data_latest目录
echo.

:: 询问是否更新data目录
if %SUCCESS_COUNT% gtr 0 (
    set /p UPDATE=是否要更新主data目录？(y/n): 
    if /i "%UPDATE%"=="y" (
        echo 更新data目录中...
        md data 2>nul
        for %%f in (%FILES%) do (
            if exist data_latest\%%f (
                copy data_latest\%%f data\%%f >nul
                echo 已更新: %%f
            )
        )
        echo.
        echo ✓ 更新完成！
    )
)

echo.
echo 提示:
echo 1. 如果无法连接设备，请检查设备是否开机且IP正确
echo 2. 请确认设备是否处于WiFi连接状态或AP模式
echo 3. 可直接查看data_latest目录中的文件内容
echo.

pause