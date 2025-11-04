@echo off
chcp 65001 >nul
echo ===============================================
echo 使用本地gradle-wrapper.jar编译SmartOvenApp
echo ===============================================
echo.

echo 检测到您已下载gradle-master目录，其中包含gradle-wrapper.jar文件
echo 文件位置: ..\gradle-master\gradle\wrapper\gradle-wrapper.jar
echo.

echo 步骤1: 复制gradle-wrapper.jar到当前项目的gradle/wrapper目录
if not exist "gradle\wrapper" mkdir "gradle\wrapper"
copy "..\gradle-master\gradle\wrapper\gradle-wrapper.jar" "gradle\wrapper\gradle-wrapper.jar"

if %errorlevel% neq 0 (
    echo 错误: 复制gradle-wrapper.jar失败
    echo 请检查文件路径是否正确
    pause
    exit /b 1
)

echo 步骤2: 检查gradle-wrapper.properties文件
if not exist "gradle\wrapper\gradle-wrapper.properties" (
    echo 创建默认的gradle-wrapper.properties文件
    echo distributionUrl=https://services.gradle.org/distributions/gradle-7.0.2-bin.zip > "gradle\wrapper\gradle-wrapper.properties"
    echo distributionSha256Sum=8de7c86697e008f0d8c616d258a9d5d130d4c6d8c0f9c366d3c6c2793c847a4c >> "gradle\wrapper\gradle-wrapper.properties"
)

echo 步骤3: 使用gradlew.bat编译项目
echo 开始编译SmartOvenApp...
echo.

if exist "gradlew.bat" (
    echo 执行: gradlew.bat build
    call gradlew.bat build
) else (
    echo 错误: gradlew.bat文件不存在
    echo 将尝试直接使用Java运行gradle-wrapper.jar
    echo.
    java -jar "gradle\wrapper\gradle-wrapper.jar" build
)

if %errorlevel% equ 0 (
    echo.
    echo ===============================================
    echo 编译成功！
    echo APK文件位置: app\build\outputs\apk\debug\app-debug.apk
    echo ===============================================
) else (
    echo.
    echo ===============================================
    echo 编译失败，请检查错误信息
    echo ===============================================
)

pause