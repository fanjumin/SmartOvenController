@echo off
chcp 65001 >nul
echo ========================================
echo SmartOven安卓APP最终编译脚本
echo ========================================
echo.

echo 当前Java版本:
java -version
echo.

echo 解决方案: 使用gradle-master目录中的Gradle版本
echo 这个版本应该兼容Java 8
echo.

REM 检查gradle-master目录是否存在
if not exist "..\gradle-master" (
    echo 错误: 找不到gradle-master目录
    echo 请确保项目根目录下有gradle-master文件夹
    pause
    exit /b 1
)

echo 使用gradle-master中的Gradle进行编译...
echo.

REM 切换到gradle-master目录并使用其中的gradlew
cd /d "..\gradle-master"

if exist "gradlew.bat" (
    echo 找到gradlew.bat，开始编译...
    call gradlew.bat --version
    echo.
    echo 编译SmartOvenApp项目...
    call gradlew.bat -p "..\SmartOvenApp" clean build
) else (
    echo 错误: gradle-master目录中没有gradlew.bat文件
    echo 尝试其他方法...
    pause
    exit /b 1
)

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo ✅ 编译成功！
    echo APK文件位置: ..\SmartOvenApp\app\build\outputs\apk\
    echo ========================================
) else (
    echo.
    echo ========================================
    echo ❌ 编译失败
    echo ========================================
)

pause