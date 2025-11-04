@echo off
echo ========================================
echo Gradle安装脚本
echo ========================================
echo.

echo 检查Java环境...
java -version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo 错误: 未找到Java环境，请先安装Java
    pause
    exit /b 1
)

echo Java环境正常
echo.

echo 方法1: 使用Android Studio的Gradle（推荐）
echo 由于您已经安装了Android Studio，可以直接使用Android Studio来编译项目
echo.
echo 方法2: 手动下载Gradle
echo 1. 访问 https://gradle.org/releases/
echo 2. 下载 gradle-7.6.4-bin.zip
echo 3. 解压到 C:\gradle
echo 4. 添加环境变量 GRADLE_HOME=C:\gradle
echo 5. 添加 PATH=%GRADLE_HOME%\bin
echo.

echo 方法3: 使用包管理器（需要网络）
echo 如果您有winget或chocolatey，可以运行：
echo winget install gradle
echo 或
echo choco install gradle
echo.

echo 当前建议：直接使用Android Studio GUI界面编译
echo 这是最简单的方法，无需额外安装Gradle
echo.
pause