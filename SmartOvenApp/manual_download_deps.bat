@echo off
chcp 65001 >nul

echo ================================================
echo 📦 SmartOvenApp依赖包手动下载指南
echo ================================================
echo.

echo 📋 项目依赖包列表：
echo.
echo 1. androidx.appcompat:appcompat:1.4.0
echo 2. com.google.android.material:material:1.4.0  
echo 3. androidx.constraintlayout:constraintlayout:2.1.2
echo 4. junit:junit:4.13.2
echo 5. androidx.test.ext:junit:1.1.3
echo 6. androidx.test.espresso:espresso-core:3.4.0
echo.

echo 🌐 下载地址：
echo.
echo 🔹 Google Maven仓库：
echo    https://maven.google.com/
echo    - androidx/appcompat/appcompat/1.4.0/
echo    - com/google/android/material/material/1.4.0/
echo    - androidx/constraintlayout/constraintlayout/2.1.2/
echo.
echo 🔹 Maven Central仓库：
echo    https://repo1.maven.org/maven2/
echo    - junit/junit/4.13.2/
echo    - androidx/test/ext/junit/1.1.3/
echo    - androidx/test/espresso/espresso-core/3.4.0/
echo.

echo 📁 本地缓存路径：
echo   %USERPROFILE%\.gradle\caches\modules-2\files-2.1
echo.

echo 🔧 手动下载步骤：
echo.
echo 1. 访问上述仓库网址
echo 2. 按包名路径找到对应的jar/aar文件
echo 3. 下载到本地gradle缓存目录
echo 4. 或直接放入项目libs文件夹
echo.

echo 💡 更简单的解决方案：
echo.
echo 方案A：使用Android Studio（推荐）
echo   - Android Studio会自动下载所有依赖
echo   - 打开项目后，Build -> Make Project
echo.
echo 方案B：配置系统代理
echo   - 运行 setup_proxy.bat 查看代理设置
echo   - 或运行 android_studio_proxy_guide.bat
echo.
echo 方案C：使用离线包
echo   - 从其他能正常编译的电脑复制.gradle缓存
echo.

echo 📱 立即操作建议：
echo   强烈推荐使用Android Studio编译！
echo   它会自动处理所有依赖下载问题。
echo.

pause