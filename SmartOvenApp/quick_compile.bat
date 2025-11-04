@echo off
chcp 65001 >nul
echo ========================================
echo SmartOven安卓APP快速编译脚本
echo ========================================
echo.

echo 使用您已下载的gradle-7.6.4进行离线编译
echo.

if not exist "..\gradle-7.6.4" (
    echo 错误: 找不到gradle-7.6.4目录
    echo 请确保gradle-7.6.4在项目根目录下
    pause
    exit /b 1
)

echo 检查Gradle版本:
"..\gradle-7.6.4\bin\gradle.bat" --version
echo.

echo 开始编译SmartOvenApp...
echo 使用离线模式避免网络问题...
echo.

"..\gradle-7.6.4\bin\gradle.bat" clean build --offline

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo ✅ 编译成功！
    echo ========================================
    echo APK文件位置: app\build\outputs\apk\debug\app-debug.apk
    echo.
    echo 如果编译失败，请尝试以下解决方案:
    echo 1. 使用Android Studio打开项目编译
    echo 2. 运行compatible_solution.bat下载兼容版本
) else (
    echo.
    echo ========================================
    echo ❌ 编译失败
    echo ========================================
    echo 推荐使用Android Studio GUI编译:
    echo 1. 打开Android Studio
    echo 2. File -> Open -> 选择SmartOvenApp文件夹
    echo 3. Build -> Make Project
    echo 4. APK位置: app\build\outputs\apk\debug\app-debug.apk
)

pause