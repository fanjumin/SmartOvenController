@echo off
chcp 65001 >nul

echo ================================================
echo 📦 Google Maven仓库详细下载指南
echo ================================================
echo.

echo 🌐 您已成功访问：https://maven.google.com/web/index.html
echo.

echo 📋 需要下载的依赖包：
echo.
echo 1. androidx.appcompat:appcompat:1.4.0
echo 2. com.google.android.material:material:1.4.0
echo 3. androidx.constraintlayout:constraintlayout:2.1.2
echo.

echo 🔍 具体下载步骤：
echo.
echo 步骤1：在搜索框中输入包名
echo   例如输入：androidx.appcompat
echo.
echo 步骤2：点击搜索结果中的包名
echo   会显示所有版本列表
echo.
echo 步骤3：找到版本 1.4.0 并点击
echo.
echo 步骤4：下载文件（通常下载 .aar 文件）
echo   - appcompat-1.4.0.aar
echo   - 或对应的 .pom 文件（包含依赖信息）
echo.

echo 📁 包的具体路径：
echo.
echo 🔹 androidx.appcompat:appcompat:1.4.0
echo   路径：androidx/appcompat/appcompat/1.4.0/
echo   文件：appcompat-1.4.0.aar
echo.
echo 🔹 com.google.android.material:material:1.4.0
echo   路径：com/google/android/material/material/1.4.0/
echo   文件：material-1.4.0.aar
echo.
echo 🔹 androidx.constraintlayout:constraintlayout:2.1.2
echo   路径：androidx/constraintlayout/constraintlayout/2.1.2/
echo   文件：constraintlayout-2.1.2.aar
echo.

echo ⚠️ 重要提醒：
echo.
echo ❗ 手动下载依赖包非常复杂！
echo   每个主依赖都有多个子依赖（传递依赖）
echo   需要下载完整的依赖树
echo.
echo 💡 强烈推荐使用Android Studio：
echo.
echo 方案A：使用Android Studio（最简单）
echo   1. 打开Android Studio
echo   2. File -> Open -> 选择SmartOvenApp文件夹
echo   3. Build -> Make Project
echo   4. Android Studio会自动下载所有依赖
echo.
echo 方案B：检查网络设置
echo   1. 运行 setup_proxy.bat 查看代理设置
echo   2. 确保网络可以访问国外网站
echo.
echo 方案C：使用镜像源（如果网络受限）
echo   修改 settings.gradle 使用国内镜像
echo.

echo 🚀 立即操作建议：
echo   请优先尝试方案A（Android Studio）！
echo   这是最直接有效的解决方案。
echo.

pause