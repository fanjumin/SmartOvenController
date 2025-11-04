@echo off
chcp 65001 >nul
echo 使用本地Gradle 9.2.0编译Android项目...
echo.

REM 设置Gradle路径
set GRADLE_HOME=..\gradle-9.2.0
set PATH=%GRADLE_HOME%\bin;%PATH%

REM 检查Gradle是否可用
if not exist "%GRADLE_HOME%\bin\gradle.bat" (
    echo 错误：找不到Gradle 9.2.0
    echo 请确保gradle-9.2.0目录存在于项目根目录
    pause
    exit /b 1
)

echo Gradle版本：
"%GRADLE_HOME%\bin\gradle.bat" --version

echo.
echo 开始编译Android项目...
"%GRADLE_HOME%\bin\gradle.bat" build

if %errorlevel% equ 0 (
    echo.
    echo ✅ 编译成功！
    echo APK文件位置：app\build\outputs\apk\debug\app-debug.apk
) else (
    echo.
    echo ❌ 编译失败，错误代码：%errorlevel%
)

pause