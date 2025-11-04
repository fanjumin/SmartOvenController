@echo off
chcp 65001 >nul

echo ========================================
echo SmartOvenApp完美编译解决方案
echo ========================================
echo.

echo 问题分析:
echo - Android Gradle Plugin 4.2.2需要Gradle 6.7.1-6.9.4
echo - Gradle 7.6.4不完全兼容
echo.

echo 解决方案: 使用完全兼容的Gradle 6.9.4
echo.

if not exist "..\gradle-6.9.4" (
    echo 正在下载Gradle 6.9.4...
    powershell -Command "Invoke-WebRequest -Uri 'https://services.gradle.org/distributions/gradle-6.9.4-bin.zip' -OutFile 'gradle-6.9.4-bin.zip'"
    if %ERRORLEVEL% equ 0 (
        echo 解压Gradle 6.9.4...
        powershell -Command "Expand-Archive -Path 'gradle-6.9.4-bin.zip' -DestinationPath '..\' -Force"
        if %ERRORLEVEL% equ 0 (
            echo 重命名目录...
            ren "..\gradle-6.9.4" "gradle-6.9.4"
            echo 清理临时文件...
            del "gradle-6.9.4-bin.zip"
        )
    )
)

if exist "..\gradle-6.9.4\bin\gradle.bat" (
    echo Gradle 6.9.4版本信息:
    "..\gradle-6.9.4\bin\gradle.bat" --version
    echo.
    echo 开始编译...
    "..\gradle-6.9.4\bin\gradle.bat" clean build
    if %ERRORLEVEL% equ 0 goto success
)

echo.
echo ========================================
echo 推荐使用Android Studio GUI编译
echo ========================================
echo.
echo 这是最可靠的方法:
echo 1. 打开Android Studio
echo 2. File -> Open -> 选择SmartOvenApp文件夹
echo 3. Build -> Make Project
echo 4. APK位置: app\build\outputs\apk\debug\app-debug.apk
echo.
goto end

:success
echo.
echo ========================================
echo 编译成功！
echo ========================================
echo APK文件位置: app\build\outputs\apk\

:end
pause