@echo off
chcp 65001 >nul

echo ================================================
echo 🤖 智能依赖下载和编译脚本
echo ================================================
echo.

echo 📊 检测网络连接状态...
ping -n 3 www.google.com >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ 可以访问Google，网络连接正常
    goto :direct_download
) else (
    echo ❌ 无法访问Google，可能需要代理
    goto :proxy_check
)

:direct_download
echo.
echo 🌐 尝试直接下载依赖包...
echo 使用官方仓库配置...

:: 恢复使用官方仓库配置
echo // 使用官方仓库配置 > settings.gradle.temp
echo pluginManagement { >> settings.gradle.temp
echo     repositories { >> settings.gradle.temp
echo         gradlePluginPortal() >> settings.gradle.temp
echo         google() >> settings.gradle.temp
echo         mavenCentral() >> settings.gradle.temp
echo     } >> settings.gradle.temp
echo } >> settings.gradle.temp
echo. >> settings.gradle.temp
echo dependencyResolutionManagement { >> settings.gradle.temp
echo     repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS) >> settings.gradle.temp
echo     repositories { >> settings.gradle.temp
echo         google() >> settings.gradle.temp
echo         mavenCentral() >> settings.gradle.temp
echo     } >> settings.gradle.temp
echo } >> settings.gradle.temp
echo. >> settings.gradle.temp
echo rootProject.name = "SmartOvenApp" >> settings.gradle.temp
echo include ':app' >> settings.gradle.temp

copy settings.gradle.temp settings.gradle >nul
del settings.gradle.temp >nul

echo.
echo 🚀 开始编译项目...
..\gradle-6.9.4\bin\gradle.bat clean build
if %errorlevel% equ 0 (
    echo ✅ 编译成功！
    goto :success
) else (
    echo ❌ 编译失败，尝试其他方案...
    goto :proxy_check
)

:proxy_check
echo.
echo 🔍 检查当前代理设置...
netsh winhttp show proxy
echo.
echo 📋 代理配置选项：
echo 1. 使用Android Studio（推荐）
echo 2. 配置系统代理
echo 3. 使用离线包
echo.
set /p choice="请选择方案 (1/2/3): "

if "%choice%"=="1" goto :android_studio
if "%choice%"=="2" goto :setup_proxy  
if "%choice%"=="3" goto :offline_mode

echo ❌ 无效选择，默认使用Android Studio方案
goto :android_studio

:android_studio
echo.
echo 💡 Android Studio方案：
echo 1. 打开Android Studio
echo 2. File -> Open -> 选择SmartOvenApp文件夹
echo 3. Build -> Make Project
echo 4. Android Studio会自动处理网络和依赖问题
echo.
echo ✅ 这是最推荐的解决方案！
goto :end

:setup_proxy
echo.
echo 🔧 代理配置方案：
echo 请运行 setup_proxy.bat 查看详细配置指南
echo 或联系网络管理员获取代理服务器信息
start setup_proxy.bat
goto :end

:offline_mode
echo.
echo 📦 离线模式：
echo 需要手动下载依赖包到本地缓存
start google_maven_download_guide.bat
goto :end

:success
echo.
echo 🎉 恭喜！项目编译成功！
echo 📱 APK文件位置：app\build\outputs\apk\
echo.

:end
echo.
echo ================================================
echo 脚本执行完成
echo ================================================
pause