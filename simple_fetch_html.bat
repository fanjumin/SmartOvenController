@echo off
cls

echo ===========================================
echo 智能烤箱控制器HTML文件获取工具
echo ===========================================

set /p DEVICE_IP=请输入设备IP地址: 

mkdir data 2>nul

echo 
set HTML_FILES=index.html temperature_control.html wifi_config.html device_status.html settings_help.html

echo 开始从设备获取HTML文件...
echo -------------------------------------------

for %%f in (%HTML_FILES%) do (
    echo 正在获取: %%f
    echo curl -s http://%DEVICE_IP%/%%f -o data/%%f
    curl -s http://%DEVICE_IP%/%%f -o data/%%f
    if exist data/%%f (
        echo 成功: %%f 已保存到 data/%%f
    ) else (
        echo 失败: 无法获取 %%f
    )
    echo.
)

echo -------------------------------------------
echo HTML文件获取完成！
echo 所有文件已保存在 data 目录中
echo.
echo 注意: 如果无法获取文件，请检查:
1. 设备IP地址是否正确
2. 设备是否在线且可访问
3. 设备Web服务器是否正常运行
echo.
echo 按任意键退出...
pause > nul