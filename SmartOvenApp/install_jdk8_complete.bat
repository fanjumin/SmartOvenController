@echo off
echo ========================================
echo SmartOvenApp - 安装完整JDK 8
echo ========================================
echo.

echo [1/6] 检查当前Java安装...
java -version 2>nul
if %ERRORLEVEL% neq 0 (
    echo Java未安装或不在PATH中
) else (
    echo Java已安装
)

echo [2/6] 检查JDK安装...
where javac >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo JDK已安装
    goto :build
)

echo [3/6] JDK未找到。需要安装完整JDK 8...
echo.
echo 请按以下步骤操作：
echo 1. 访问Oracle官网下载JDK 8
echo 2. 下载链接：https://www.oracle.com/java/technologies/javase/javase8-archive-downloads.html
echo 3. 选择 "Java SE Development Kit 8u381"
echo 4. 下载Windows x64版本：jdk-8u381-windows-x64.exe
echo 5. 运行安装程序，使用默认路径安装
echo.

:wait_installation
echo [4/6] 等待JDK 8安装...
echo 请安装JDK 8后按任意键继续...
pause >nul

:check_installation
echo [5/6] 检查JDK安装...
where javac >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo JDK未找到。请确认已正确安装JDK 8。
    echo.
    echo 安装说明：
    echo 1. 从Oracle官网下载JDK 8
    echo 2. 运行安装程序
    echo 3. 接受许可协议
    echo 4. 使用默认安装路径
    echo.
    goto :wait_installation
)

echo JDK找到！设置环境...
for /f "tokens=2*" %%i in ('reg query "HKLM\SOFTWARE\JavaSoft\Java Development Kit\1.8" /v JavaHome 2^>nul') do set JAVA_HOME=%%j
if "%JAVA_HOME%"=="" (
    for /f "tokens=2*" %%i in ('reg query "HKLM\SOFTWARE\JavaSoft\JDK\1.8" /v JavaHome 2^>nul') do set JAVA_HOME=%%j
)
if "%JAVA_HOME%"=="" (
    echo 无法自动找到JDK 8安装路径。
    echo 请手动设置JAVA_HOME环境变量。
    set JAVA_HOME=C:\Program Files\Java\jdk1.8.0_381
)

echo JAVA_HOME设置为: %JAVA_HOME%

:build
echo [6/6] 使用JDK 8开始项目构建...
set JAVA_HOME=%JAVA_HOME%
call gradlew.bat clean build

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo 构建成功！
    echo APK位置: app\build\outputs\apk\
    echo ========================================
) else (
    echo.
    echo ========================================
    echo 构建失败。请检查上面的错误信息。
    echo ========================================
)

pause