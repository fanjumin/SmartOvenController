@echo off
chcp 65001 >nul

echo ========================================
echo 使用gradle-7.6.4编译SmartOvenApp
echo ========================================
echo.

echo 确认gradle-7.6.4可用性...
if not exist "..\gradle-7.6.4\bin\gradle.bat" (
    echo 错误: 找不到gradle-7.6.4
    echo 请确保gradle-7.6.4在项目根目录下
    pause
    exit /b 1
)

echo 检查Gradle版本...
"..\gradle-7.6.4\bin\gradle.bat" --version
echo.

echo 检查Java版本...
java -version
echo.

echo 开始编译SmartOvenApp...
echo 使用gradle-7.6.4 + Android Gradle Plugin 4.2.2
echo.

echo 步骤1: 清理项目...
"..\gradle-7.6.4\bin\gradle.bat" clean
if %ERRORLEVEL% neq 0 (
    echo 清理失败，继续编译...
)

echo.
echo 步骤2: 编译项目...
echo.
"..\gradle-7.6.4\bin\gradle.bat" build

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo 编译成功！
    echo ========================================
    echo APK文件位置: app\build\outputs\apk\debug\app-debug.apk
    echo.
    if exist "app\build\outputs\apk\debug\app-debug.apk" (
        echo APK文件已生成！
        dir "app\build\outputs\apk\debug\app-debug.apk"
    )
) else (
    echo.
    echo ========================================
    echo 编译失败
    echo ========================================
    echo 尝试解决方案:
    echo 1. 使用Android Studio打开项目编译
    echo 2. 运行兼容性脚本: compatible_solution.bat
)

echo.
pause