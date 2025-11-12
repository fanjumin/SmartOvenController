@echo off
echo ================================
echo 智能烤箱控制器构建脚本
echo ================================

echo.
echo [1/4] 构建固件...
pio run
if %errorlevel% neq 0 (
    echo 固件构建失败
    pause
    exit /b 1
)
echo 固件构建完成

echo.
echo [2/4] 构建文件系统...
pio run -t buildfs
if %errorlevel% neq 0 (
    echo 文件系统构建失败
    pause
    exit /b 1
)
echo 文件系统构建完成

echo.
echo [3/4] 创建发布目录...
if not exist "release" mkdir release

echo.
echo [4/4] 复制生成的文件...
copy ".pio\build\nodemcuv2\firmware.bin" "release\"
copy ".pio\build\nodemcuv2\littlefs.bin" "release\"
copy "platformio.ini" "release\"
copy "BUILD_INSTRUCTIONS.md" "release\"

echo.
echo ================================
echo 构建完成！生成的文件已复制到 release 目录：
echo - firmware.bin (固件)
echo - littlefs.bin (文件系统)
echo - platformio.ini (配置文件)
echo - BUILD_INSTRUCTIONS.md (构建说明)
echo ================================
pause