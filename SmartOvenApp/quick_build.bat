@echo off
echo ========================================
echo SmartOven APP快速编译脚本
echo ========================================
echo.

echo 项目信息:
echo - 项目名称: SmartOven APP
echo - 包名: com.smartoven.app
echo - 目标SDK: 31
echo - 最低SDK: 21
echo.

echo 编译方法选择:
echo 1. 使用Android Studio GUI界面（推荐）
echo 2. 手动下载Gradle Wrapper文件
echo 3. 使用系统已安装的Gradle
echo.

echo 方法1详细步骤:
echo 1. 打开Android Studio
echo 2. 选择 "Open" -> 导航到当前目录
echo 3. 等待项目同步完成（Android Studio会自动下载Gradle）
echo 4. 点击 "Build" -> "Make Project" (Ctrl+F9)
echo 5. APK文件将生成在: app\\build\\outputs\\apk\\debug\\app-debug.apk
echo.

echo 方法2详细步骤:
echo 1. 下载gradle-wrapper.jar文件:
echo    https://github.com/gradle/gradle/raw/v7.4.2/gradle/wrapper/gradle-wrapper.jar
echo 2. 保存到: gradle\\wrapper\\gradle-wrapper.jar
echo 3. 运行: gradlew.bat assembleDebug
echo.

echo 方法3详细步骤:
echo 1. 确保系统已安装Gradle
echo 2. 运行: gradle assembleDebug
echo.

echo 当前建议:
echo 强烈推荐使用方法1（Android Studio GUI）
echo 这是最简单可靠的方法，无需额外配置
echo.

echo APK安装说明:
echo 1. 将app-debug.apk复制到安卓手机
echo 2. 在手机上允许安装未知来源应用
echo 3. 安装并运行SmartOven APP
echo 4. 确保手机和烤箱控制器在同一WiFi网络
echo 5. 在APP中输入烤箱控制器的IP地址（如192.168.1.100）
echo.

echo 连接测试:
echo - 烤箱控制器IP: 从Arduino串口监视器获取
echo - 端口: 通常为23（Telnet端口）
echo - 确保网络连接正常
echo.
pause