@echo off
chcp 65001 >nul
echo ===============================================
echo 离线Gradle编译环境设置指南
echo ===============================================
echo.

echo 由于网络连接问题，Gradle无法下载gradle-7.0.2-bin.zip
echo 请按照以下步骤设置离线编译环境：
echo.

echo 步骤1: 手动下载Gradle发行版
echo 请访问: https://services.gradle.org/distributions/gradle-7.0.2-bin.zip
echo 或者使用其他网络环境下载该文件
echo.

echo 步骤2: 将下载的文件放入Gradle缓存目录
echo 默认缓存目录: %%USERPROFILE%%\.gradle\wrapper\dists\gradle-7.0.2-bin\[随机哈希]\
echo.

echo 步骤3: 设置离线模式
echo 创建或编辑 gradle.properties 文件，添加以下内容：
echo org.gradle.parallel=true
echo org.gradle.daemon=true
echo org.gradle.configureondemand=true
echo org.gradle.caching=true
echo.

echo 推荐方案: 使用Android Studio GUI编译
echo 这是最简单的方法，完全绕过网络问题：
echo 1. 打开Android Studio
echo 2. 导入SmartOvenApp项目
echo 3. 等待Gradle同步完成
echo 4. 点击Build -> Make Project
echo.

echo APK文件位置: app\build\outputs\apk\debug\app-debug.apk
echo.
pause