@echo off
echo ========================================
echo 使用本地Gradle 9.2.0编译Android项目
echo ========================================
echo.

REM 设置Gradle路径
echo 使用本地Gradle 9.2.0版本...
set GRADLE_HOME=..\gradle-9.2.0
set GRADLE_BAT=%GRADLE_HOME%\bin\gradle.bat

REM 检查Gradle是否可用
if not exist "%GRADLE_BAT%" (
    echo 错误: 找不到Gradle可执行文件
    echo 请确保gradle-9.2.0目录存在于项目根目录
    pause
    exit /b 1
)

echo Gradle版本信息:
"%GRADLE_BAT%" --version

if %ERRORLEVEL% neq 0 (
    echo 错误: Gradle版本检查失败
    pause
    exit /b 1
)

echo.
echo 开始编译Android项目...
echo ========================================

REM 执行编译（使用离线模式避免网络问题）
"%GRADLE_BAT%" --offline clean build

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo 编译成功！
    echo APK文件位置: app\build\outputs\apk\
    echo ========================================
) else (
    echo.
    echo ========================================
    echo 编译失败，请检查错误信息
    echo ========================================
)

pause