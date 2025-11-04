@echo off
chcp 65001 >nul
echo ========================================
echo SmartOven安卓APP最终编译解决方案
echo ========================================
echo.

echo 当前环境检查:
java -version
echo.

echo 问题分析:
echo - Android Gradle Plugin 4.2.2需要Gradle 6.7.1-6.9.4
echo - Gradle 7.6.4与Android Gradle Plugin 4.2.2不完全兼容
echo.

echo 解决方案: 使用完全兼容的组合
echo - Gradle 6.9.4 + Android Gradle Plugin 4.2.2 + Java 8
echo.

echo 步骤1: 下载Gradle 6.9.4 (完全兼容组合)
if not exist "..\gradle-6.9.4" (
    echo 正在下载Gradle 6.9.4...
    powershell -Command "Invoke-WebRequest -Uri 'https://services.gradle.org/distributions/gradle-6.9.4-bin.zip' -OutFile 'gradle-6.9.4-bin.zip'"
    if %ERRORLEVEL% neq 0 (
        echo 下载失败，尝试离线方案...
        goto offline_solution
    )
    echo 解压Gradle 6.9.4...
    powershell -Command "Expand-Archive -Path 'gradle-6.9.4-bin.zip' -DestinationPath '..\' -Force"
    if %ERRORLEVEL% neq 0 (
        echo 解压失败
        goto offline_solution
    )
    echo 重命名目录...
    ren "..\gradle-6.9.4" "gradle-6.9.4"
    echo 清理临时文件...
    del "gradle-6.9.4-bin.zip"
) else (
    echo 已找到本地Gradle 6.9.4
)

echo.
echo 步骤2: 使用Gradle 6.9.4编译项目
if exist "..\gradle-6.9.4\bin\gradle.bat" (
    echo Gradle版本信息:
    "..\gradle-6.9.4\bin\gradle.bat" --version
    echo.
    echo 开始编译SmartOvenApp...
    "..\gradle-6.9.4\bin\gradle.bat" clean build
    
    if %ERRORLEVEL% equ 0 goto success
    echo 编译失败，尝试离线模式...
    "..\gradle-6.9.4\bin\gradle.bat" clean build --offline
    if %ERRORLEVEL% equ 0 goto success
) else (
    echo 错误: 找不到Gradle 6.9.4
)

:offline_solution
echo.
echo 离线解决方案: 使用Android Studio GUI界面
echo 这是最可靠的方法，无需命令行编译
echo.
echo 步骤:
echo 1. 打开Android Studio
echo 2. 选择 File -> Open -> 选择SmartOvenApp文件夹
echo 3. 等待项目同步完成
echo 4. 点击 Build -> Make Project
echo 5. APK文件位置: app\build\outputs\apk\debug\app-debug.apk
echo.
echo 这是最推荐的解决方案！
goto end

:success
echo.
echo ========================================
echo ✅ 编译成功！
echo APK文件位置: app\build\outputs\apk\
echo ========================================

:end
pause