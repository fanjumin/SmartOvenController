@echo off
chcp 65001 >nul

echo ================================================
echo 🎯 SmartOvenApp智能编译解决方案
echo ================================================
echo.

REM 检查Gradle 6.9.4
if not exist "..\gradle-6.9.4\bin\gradle.bat" (
    echo ❌ 错误：未找到Gradle 6.9.4
echo   请确保gradle-6.9.4文件夹位于项目根目录下
echo.
    pause
    exit /b 1
)

echo ✅ 找到Gradle 6.9.4（官方兼容版本）
echo.

REM 显示版本信息
echo 📋 Gradle版本信息：
"..\gradle-6.9.4\bin\gradle.bat" --version
echo.

REM 尝试正常编译
echo 🔄 尝试正常编译（需要网络连接）...
echo   如果失败，可能是网络或代理问题
echo.

"..\gradle-6.9.4\bin\gradle.bat" clean build

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ 编译成功！
echo.
echo 📱 APK文件位置：
echo    app\build\outputs\apk\debug\app-debug.apk
echo.
    goto SUCCESS
) else (
    echo.
    echo ❌ 编译失败，错误代码：%ERRORLEVEL%
echo.
)

REM 如果编译失败，提供解决方案
echo 🔧 编译失败解决方案：
echo.
echo 方案1：检查网络连接和代理设置
echo   - 确保您的网络可以访问Google和Maven仓库
echo   - 如果使用代理，请配置代理设置
echo.
echo 方案2：使用Android Studio（推荐）
echo   - 打开Android Studio
echo   - File -> Open -> 选择SmartOvenApp文件夹
echo   - Build -> Make Project
echo   - APK位置：app\build\outputs\apk\debug\app-debug.apk
echo.
echo 方案3：手动配置代理
echo   - 运行 setup_proxy.bat 查看代理设置指南
echo   - 或运行 android_studio_proxy_guide.bat
echo.

:SUCCESS
echo.
echo 💡 提示：
echo   - 这是最稳定的官方兼容组合：Gradle 6.9.4 + AGP 4.2.2
echo   - 如果命令行编译失败，强烈推荐使用Android Studio
echo.

pause