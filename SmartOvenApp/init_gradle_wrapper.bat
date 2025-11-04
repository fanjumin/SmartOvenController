@echo off
echo ========================================
echo Gradle Wrapper初始化脚本
echo ========================================
echo.

echo 检查Java环境...
"C:\Program Files\Android\Android Studio\jbr\bin\java.exe" -version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo 错误: Android Studio的Java环境不可用
    pause
    exit /b 1
)

echo Java环境正常
echo.

echo 创建gradle-wrapper.jar文件...

REM 创建一个简单的gradle-wrapper.jar替代方案
echo 由于网络连接问题，将使用Android Studio直接编译
echo.

echo 方法1: 使用Android Studio GUI编译（推荐）
echo 1. 打开Android Studio
echo 2. 选择 "Open" 并导航到当前目录
echo 3. Android Studio会自动配置Gradle
echo 4. 点击 "Build" -> "Make Project"
echo.

echo 方法2: 手动下载gradle-wrapper.jar
echo 1. 访问: https://github.com/gradle/gradle/tree/v7.4.2/gradle/wrapper
echo 2. 下载 gradle-wrapper.jar 文件
echo 3. 保存到 gradle\\wrapper\\ 目录
echo 4. 然后运行: gradlew.bat assembleDebug
echo.

echo 当前建议: 使用Android Studio GUI界面编译
echo 这是最简单可靠的方法
echo.
pause