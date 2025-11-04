@echo off
echo ========================================
echo Android Studio Setup Wizard HTTP Proxy设置指南
echo ========================================
echo.

echo 方法1: 在Android Studio首次启动时设置
echo 步骤:
echo 1. 打开Android Studio
echo 2. 在Welcome界面，点击"Configure" -> "Settings"
echo 3. 在左侧选择"Appearance & Behavior" -> "System Settings" -> "HTTP Proxy"
echo 4. 选择"Manual proxy configuration"
echo 5. 填写以下信息:
echo    - Proxy host: 代理服务器IP地址（如192.168.1.100）
echo    - Proxy port: 端口号（如8080）
echo    - 如果需要认证:
echo      * No proxy for: 填写不需要代理的地址（如localhost,127.0.0.1）
echo 6. 点击"Check connection"测试连接
echo 7. 点击"OK"保存设置
echo.

echo 方法2: 在Android Studio运行中设置
echo 步骤:
echo 1. 打开Android Studio
echo 2. 点击菜单栏 "File" -> "Settings" (Windows) 或 "Android Studio" -> "Preferences" (Mac)
echo 3. 导航到: "Appearance & Behavior" -> "System Settings" -> "HTTP Proxy"
echo 4. 选择"Manual proxy configuration"
echo 5. 填写代理服务器信息
echo 6. 点击"Check connection"测试
echo 7. 点击"OK"保存
echo.

echo 方法3: 使用自动检测
echo 1. 在HTTP Proxy设置中，选择"Auto-detect proxy settings"
echo 2. Android Studio会自动检测系统代理设置
echo 3. 或者选择"Use system proxy settings"
echo.

echo 代理服务器信息示例:
echo - 公司内部代理: proxy.company.com:8080
echo - 本地代理: 127.0.0.1:8080
echo - 其他: 192.168.1.100:3128
echo.

echo 常见问题解决:
echo 1. 如果连接测试失败，检查代理服务器是否可用
echo 2. 确保端口号正确
echo 3. 如果需要用户名密码认证，在相应字段填写
echo 4. 在"No proxy for"中添加本地地址避免循环代理
echo.

echo 设置完成后:
echo 1. Android Studio会自动使用代理下载Gradle和依赖库
echo 2. 项目同步和构建将能够正常进行
echo 3. 您可以正常编译SmartOven APP的APK文件
echo.

echo 如果不知道代理服务器信息:
echo 1. 联系网络管理员获取代理服务器地址和端口
echo 2. 或者查看系统网络设置中的代理配置
echo 3. 在Windows中: 设置 -> 网络和Internet -> 代理
echo.

echo 设置完成后，您可以:
echo 1. 打开SmartOvenApp项目
echo 2. 等待Gradle同步完成
echo 3. 点击"Build" -> "Make Project"编译APK
echo 4. APK文件位置: app\\build\\outputs\\apk\\debug\\app-debug.apk
echo.
pause