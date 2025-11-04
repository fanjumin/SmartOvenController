@echo off
chcp 65001 >nul

echo ========================================
echo SmartOvenApp最终修复方案
echo ========================================
echo.

echo 问题: settings.gradle中的dependencyResolutionManagement配置冲突
echo 解决方案: 简化配置，使用传统方式
echo.

echo 步骤1: 备份原始settings.gradle...
if not exist "settings.gradle.bak" (
    copy "settings.gradle" "settings.gradle.bak"
    echo 已备份settings.gradle
)

echo.
echo 步骤2: 创建简化的settings.gradle...
echo 使用传统仓库配置方式...
echo.

(
echo // 简化的settings.gradle - 兼容Gradle 7.6.4和Android Gradle Plugin 4.2.2
echo pluginManagement {
    echo     repositories {
    echo         gradlePluginPortal()
    echo         google()
    echo         mavenCentral()
    echo     }
    echo }
    echo 
    echo rootProject.name = "SmartOvenApp"
    echo include ':app'
) > settings.gradle

echo.
echo 步骤3: 检查配置...
type settings.gradle
echo.

echo 步骤4: 使用gradle-7.6.4编译...
if exist "..\gradle-7.6.4\bin\gradle.bat" (
    echo Gradle版本信息:
    "..\gradle-7.6.4\bin\gradle.bat" --version
    echo.
    echo 开始编译...
    "..\gradle-7.6.4\bin\gradle.bat" clean build
    if %ERRORLEVEL% equ 0 goto success
)

echo.
echo ========================================
echo 编译失败，尝试备用方案
echo ========================================
echo.
echo 方案1: 恢复原始配置并尝试离线编译
echo.
copy "settings.gradle.bak" "settings.gradle"
"..\gradle-7.6.4\bin\gradle.bat" clean build --offline
if %ERRORLEVEL% equ 0 goto success

echo.
echo 方案2: 使用Android Studio GUI编译（最推荐）
echo 这是最可靠的方法！
echo.
goto end

:success
echo.
echo ========================================
echo ✅ 编译成功！
echo ========================================
echo APK文件位置: app\build\outputs\apk\debug\app-debug.apk
echo.
if exist "app\build\outputs\apk\debug\app-debug.apk" (
    echo APK文件已生成！
    dir "app\build\outputs\apk\debug\app-debug.apk"
)

:end
echo.
echo 按任意键退出...
pause >nul