@echo off
echo ========================================
echo SmartOven安卓APP编译工具
echo ========================================
echo.

echo 检查项目结构...
if not exist "app\src\main\java\com\smartoven\app\MainActivity.java" (
    echo 错误: 未找到MainActivity.java文件
    pause
    exit /b 1
)

if not exist "app\build.gradle" (
    echo 错误: 未找到build.gradle文件
    pause
    exit /b 1
)

echo 项目结构完整，可以编译
echo.
echo 由于缺少Gradle包装器文件，建议使用以下方法:
echo.
echo 方法1: 使用Android Studio GUI界面
echo   1. 打开Android Studio
echo   2. File -> Open -> 选择SmartOvenApp文件夹
echo   3. Build -> Make Project
echo.
echo 方法2: 手动初始化Gradle包装器
echo   在项目目录执行: gradle wrapper
echo   然后执行: gradlew assembleDebug
echo.
echo 方法3: 使用系统Gradle
echo   在项目目录执行: gradle assembleDebug
echo.
pause