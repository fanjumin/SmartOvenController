@echo off
chcp 65001 >nul

echo ================================================
echo 🚀 终极编译解决方案
echo ================================================
echo.

echo 📊 检测系统环境...

:: 检查Gradle版本
echo 🔍 检查Gradle版本...
if exist "..\gradle-6.9.4\bin\gradle.bat" (
    echo ✅ 找到Gradle 6.9.4
    set GRADLE_PATH=..\gradle-6.9.4\bin\gradle.bat
) else (
    echo ❌ 未找到Gradle 6.9.4
    goto :gradle_error
)

echo.
echo 🔧 配置Gradle Wrapper使用本地Gradle...

:: 创建正确的gradle-wrapper.properties
echo distributionBase=GRADLE_USER_HOME > gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionPath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed
echo # 使用本地Gradle 6.9.4，避免下载超时 >> gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionUrl=gradle-6.9.4 >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStoreBase=GRADLE_USER_HOME >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStorePath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed

copy gradle\wrapper\gradle-wrapper.properties.fixed gradle\wrapper\gradle-wrapper.properties >nul
del gradle\wrapper\gradle-wrapper.properties.fixed >nul

echo ✅ Gradle配置已更新
echo.

echo 🌐 配置仓库镜像源...

:: 创建优化的settings.gradle
echo // 终极编译配置 - 使用多镜像源 > settings.gradle.ultimate
echo pluginManagement { >> settings.gradle.ultimate
echo     repositories { >> settings.gradle.ultimate
echo         gradlePluginPortal() >> settings.gradle.ultimate
echo         // 华为云镜像 >> settings.gradle.ultimate
echo         maven { url 'https://repo.huaweicloud.com/repository/maven/' } >> settings.gradle.ultimate
echo         // 阿里云镜像 >> settings.gradle.ultimate
echo         maven { url 'https://maven.aliyun.com/repository/public' } >> settings.gradle.ultimate
echo         // 官方源 >> settings.gradle.ultimate
echo         google() >> settings.gradle.ultimate
echo         mavenCentral() >> settings.gradle.ultimate
echo     } >> settings.gradle.ultimate
echo } >> settings.gradle.ultimate
echo. >> settings.gradle.ultimate
echo dependencyResolutionManagement { >> settings.gradle.ultimate
echo     repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS) >> settings.gradle.ultimate
echo     repositories { >> settings.gradle.ultimate
echo         // 多镜像源 >> settings.gradle.ultimate
echo         maven { url 'https://repo.huaweicloud.com/repository/maven/' } >> settings.gradle.ultimate
echo         maven { url 'https://maven.aliyun.com/repository/public' } >> settings.gradle.ultimate
echo         maven { url 'https://maven.aliyun.com/repository/google' } >> settings.gradle.ultimate
echo         // 官方源 >> settings.gradle.ultimate
echo         google() >> settings.gradle.ultimate
echo         mavenCentral() >> settings.gradle.ultimate
echo     } >> settings.gradle.ultimate
echo } >> settings.gradle.ultimate
echo. >> settings.gradle.ultimate
echo rootProject.name = "SmartOvenApp" >> settings.gradle.ultimate
echo include ':app' >> settings.gradle.ultimate

copy settings.gradle.ultimate settings.gradle >nul
del settings.gradle.ultimate >nul

echo ✅ 仓库配置已优化
echo.

echo 🚀 开始编译项目...
echo.

echo 📋 编译步骤：
echo 1. 清理项目
echo 2. 下载依赖包
echo 3. 编译APK
echo.

:: 方法1：使用本地Gradle直接编译
echo 🔧 尝试方法1：直接使用本地Gradle编译...
%GRADLE_PATH% clean build --info
if %errorlevel% equ 0 goto :success

echo.
echo ❌ 方法1失败，尝试方法2...
echo.

:: 方法2：使用gradlew编译
echo 🔧 尝试方法2：使用Gradle Wrapper编译...
gradlew.bat clean build --info
if %errorlevel% equ 0 goto :success

echo.
echo ❌ 方法2失败，使用最终方案...
echo.

:: 最终方案：分步编译
echo 🔧 最终方案：分步编译...
echo.

echo 📦 步骤1：清理项目...
%GRADLE_PATH% clean
if %errorlevel% neq 0 (
    echo ❌ 清理失败
    goto :final_fail
)

echo.
echo 📦 步骤2：编译Debug版本...
%GRADLE_PATH% assembleDebug --stacktrace
if %errorlevel% equ 0 goto :debug_success

echo.
echo ❌ Debug编译失败，尝试Release版本...
echo.

echo 📦 步骤3：编译Release版本...
%GRADLE_PATH% assembleRelease --stacktrace
if %errorlevel% equ 0 goto :release_success

echo.
echo ❌ 所有编译方法都失败
goto :final_fail

:debug_success
echo.
echo ✅ Debug版本编译成功！
goto :apk_check

:release_success
echo.
echo ✅ Release版本编译成功！
goto :apk_check

:success
echo.
echo ✅ 项目编译成功！

:apk_check
echo.
echo 🔍 检查APK文件生成...
if exist "app\build\outputs\apk\debug\app-debug.apk" (
    echo ✅ 找到Debug APK: app\build\outputs\apk\debug\app-debug.apk
    set APK_FOUND=1
)
if exist "app\build\outputs\apk\release\app-release.apk" (
    echo ✅ 找到Release APK: app\build\outputs\apk\release\app-release.apk
    set APK_FOUND=1
)

if "%APK_FOUND%"=="1" goto :complete_success

echo.
echo ⚠️ 未找到APK文件，但编译过程成功
goto :complete_success

:complete_success
echo.
echo 🎉 终极编译完成！
echo 📱 APK文件位置：app\build\outputs\apk\
echo.
goto :end

:gradle_error
echo.
echo ❌ Gradle 6.9.4未找到，请确保已下载并解压到gradle-6.9.4文件夹
echo 💡 解决方案：使用Android Studio编译
echo.
goto :android_studio

:final_fail
echo.
echo ❌ 编译失败，建议使用Android Studio
echo.

:android_studio
echo 💡 推荐使用Android Studio：
echo 1. 打开Android Studio
echo 2. File -> Open -> 选择SmartOvenApp文件夹
echo 3. Build -> Make Project
echo 4. Android Studio会自动处理所有问题
echo.

echo ✅ 这是最可靠的解决方案！

:end
echo.
echo ================================================
echo 脚本执行完成
echo ================================================
echo.
echo 📋 总结：
if "%APK_FOUND%"=="1" (
    echo ✅ 编译成功，APK已生成
) else (
    echo ⚠️ 建议使用Android Studio获得最佳结果
)
echo.
pause