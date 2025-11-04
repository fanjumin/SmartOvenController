@echo off
chcp 65001 >nul

echo ================================================
echo 🔧 修复Gradle下载超时问题
echo ================================================
echo.

echo 📊 当前问题：Gradle 7.6.4下载超时
echo ❌ 错误信息：java.net.SocketTimeoutException: Connect timed out
echo.

echo 💡 解决方案：使用本地已下载的Gradle 6.9.4
echo.

echo 🚀 开始修复...
echo.

:: 方法1：直接使用本地Gradle 6.9.4编译
echo 方法1：直接使用本地Gradle 6.9.4编译
echo 执行命令：..\gradle-6.9.4\bin\gradle.bat clean build
echo.

..\gradle-6.9.4\bin\gradle.bat clean build
if %errorlevel% equ 0 goto :success

echo.
echo ❌ 方法1失败，尝试方法2...
echo.

:: 方法2：修改Gradle Wrapper配置
echo 方法2：修改Gradle Wrapper配置使用本地Gradle
echo.

:: 创建修复后的gradle-wrapper.properties
echo distributionBase=GRADLE_USER_HOME > gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionPath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed
echo # 使用本地Gradle 6.9.4，避免下载超时 >> gradle\wrapper\gradle-wrapper.properties.fixed
echo distributionUrl=gradle-6.9.4 >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStoreBase=GRADLE_USER_HOME >> gradle\wrapper\gradle-wrapper.properties.fixed
echo zipStorePath=wrapper/dists >> gradle\wrapper\gradle-wrapper.properties.fixed

copy gradle\wrapper\gradle-wrapper.properties.fixed gradle\wrapper\gradle-wrapper.properties >nul
del gradle\wrapper\gradle-wrapper.properties.fixed >nul

echo ✅ Gradle Wrapper配置已更新
echo.

echo 🚀 使用修复后的配置重新编译...
echo.

gradlew.bat clean build
if %errorlevel% equ 0 goto :success

echo.
echo ❌ 方法2失败，使用最终方案...
echo.

:: 最终方案：使用Android Studio
echo 最终方案：使用Android Studio
echo.
echo 💡 Android Studio会自动处理Gradle下载问题
echo.
echo 📋 操作步骤：
echo 1. 打开Android Studio
echo 2. File -> Open -> 选择SmartOvenApp文件夹
echo 3. Build -> Make Project
echo 4. Android Studio会自动下载正确的Gradle版本
echo.

echo ✅ 这是最可靠的解决方案！
goto :end

:success
echo.
echo 🎉 恭喜！编译成功！
echo 📱 APK文件位置：app\build\outputs\apk\
echo.

:end
echo.
echo ================================================
echo 修复完成
echo ================================================
pause