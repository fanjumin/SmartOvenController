@echo off
chcp 65001 >nul

echo ================================================
echo 🎯 使用本地Gradle 6.9.4编译SmartOvenApp
echo ================================================
echo.

REM 检查本地Gradle 6.9.4是否存在
if not exist "..\gradle-6.9.4\bin\gradle.bat" (
    echo ❌ 错误：未找到本地Gradle 6.9.4
echo   请确保gradle-6.9.4文件夹位于项目根目录下
echo   当前项目路径：%CD%
echo   期望路径：..\gradle-6.9.4\
echo.
    pause
    exit /b 1
)

echo ✅ 找到本地Gradle 6.9.4
echo.

REM 显示Gradle版本信息
echo 📋 检查Gradle版本...
"..\gradle-6.9.4\bin\gradle.bat" --version
echo.

REM 清理项目
echo 🧹 清理项目...
"..\gradle-6.9.4\bin\gradle.bat" clean
echo.

REM 编译项目
echo 🔨 开始编译SmartOvenApp...
echo   使用Gradle 6.9.4 + Android Gradle Plugin 4.2.2
echo   这是官方推荐的兼容组合
echo.

"..\gradle-6.9.4\bin\gradle.bat" build

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ 编译成功！
echo.
echo 📱 APK文件位置：
echo    app\build\outputs\apk\debug\app-debug.apk
echo.
echo 💡 提示：
echo   - 您也可以使用Android Studio编译：Build -> Make Project
echo   - 这个组合是最稳定的官方兼容版本
echo.
) else (
    echo.
    echo ❌ 编译失败，错误代码：%ERRORLEVEL%
echo.
echo 🔧 建议：
echo   - 检查网络连接（需要下载依赖）
echo   - 尝试使用Android Studio编译
echo   - 检查settings.gradle配置
echo.
)

pause