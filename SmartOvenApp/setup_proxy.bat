@echo off
echo ========================================
echo HTTP代理设置指南
echo ========================================
echo.

echo 方法1: 设置系统环境变量（推荐）
echo 步骤:
echo 1. 右键点击"此电脑" -> "属性"
echo 2. 点击"高级系统设置"
echo 3. 点击"环境变量"
echo 4. 在"系统变量"中新建以下变量:
echo    - 变量名: HTTP_PROXY
echo    - 变量值: http://代理服务器IP:端口
echo    - 变量名: HTTPS_PROXY
echo    - 变量值: http://代理服务器IP:端口
echo 5. 重启命令行窗口
echo.

echo 方法2: 在命令行中临时设置
echo 在PowerShell中运行:
echo set HTTP_PROXY=http://代理服务器IP:端口
echo set HTTPS_PROXY=http://代理服务器IP:端口
echo.

echo 方法3: 使用netsh设置系统代理
echo 以管理员身份运行命令提示符，然后执行:
echo netsh winhttp set proxy 代理服务器IP:端口
echo.

echo 常见的代理服务器地址示例:
echo - 公司内部代理: http://proxy.company.com:8080
echo - 本地代理: http://127.0.0.1:8080
echo - 其他: http://192.168.1.100:8080
echo.

echo 检查当前代理设置:
echo netsh winhttp show proxy
echo.

echo 清除代理设置:
echo netsh winhttp reset proxy
echo.

echo 针对Gradle的特殊设置:
echo 在项目根目录创建gradle.properties文件，添加:
echo systemProp.http.proxyHost=代理服务器IP
echo systemProp.http.proxyPort=端口
echo systemProp.https.proxyHost=代理服务器IP
echo systemProp.https.proxyPort=端口
echo.

echo 如果您不知道代理服务器信息:
echo 1. 联系网络管理员获取代理服务器地址
echo 2. 或者尝试使用无需代理的编译方法
echo.

echo 无需代理的替代方案:
echo 直接使用Android Studio GUI界面编译（推荐）
echo Android Studio会自动处理网络连接问题
echo.
pause