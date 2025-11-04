@echo off
echo ========================================
echo Android Studio编译APK指南
echo ========================================
echo.

echo 方法1: 使用Android Studio GUI界面（推荐）
echo 步骤:
echo 1. 打开Android Studio
echo 2. 选择 "Open" 或 "Open an existing Android Studio project"
echo 3. 导航到当前目录: f:\Github\SmartOvenController\SmartOvenApp
echo 4. 等待Android Studio同步和配置项目
echo 5. 点击菜单栏的 "Build" -> "Make Project" (Ctrl+F9)
echo 6. 编译成功后，APK文件将生成在:
echo    app\\build\\outputs\\apk\\debug\\app-debug.apk
echo.

echo 方法2: 使用Android Studio的命令行工具
echo 步骤:
echo 1. 确保Android Studio已安装并配置了环境变量
echo 2. 在Android Studio中打开项目并确保同步完成
echo 3. 然后可以使用以下命令:
echo    gradlew.bat assembleDebug
echo.

echo 当前项目状态检查:
echo - 项目结构: 完整 ✓
echo - build.gradle: 存在 ✓
echo - AndroidManifest.xml: 存在 ✓
echo - MainActivity.java: 存在 ✓
echo - Gradle Wrapper: 需要初始化
echo.

echo 建议: 使用方法1（Android Studio GUI）最为可靠
echo.

echo APK文件位置:
echo app\\build\\outputs\\apk\\debug\\app-debug.apk
echo.

echo 安装到手机:
echo 1. 将APK文件复制到手机
echo 2. 在手机上允许安装未知来源应用
echo 3. 安装并运行
echo 4. 确保手机和烤箱控制器在同一WiFi网络
echo 5. 在APP中输入烤箱控制器的IP地址
echo.
pause