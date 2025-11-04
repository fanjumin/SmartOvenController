@echo off
chcp 65001 >nul

echo ================================================
echo Android Studio 编译指南
echo ================================================
echo.
echo 如果您在Android Studio中找不到Build菜单，请按以下步骤操作：
echo.
echo 步骤1：正确打开项目
echo   - 启动Android Studio
echo   - 选择"Open an existing project"
echo   - 选择 F:\Github\SmartOvenController\SmartOvenApp 文件夹
echo.
echo 步骤2：等待项目同步
echo   - 查看底部状态栏的"Gradle Sync"进度
echo   - 等待同步完成
echo.
echo 步骤3：编译方法（任选其一）
echo.
echo 方法A：使用工具栏
echo   - 查找工具栏上的"锤子"图标 🔨
echo   - 点击这个图标编译项目
echo.
echo 方法B：使用右键菜单
echo   - 在左侧项目面板中，右键点击"app"模块
echo   - 选择"Make Module 'app'"
echo.
echo 方法C：使用终端
echo   - 打开Android Studio底部的"Terminal"标签
echo   - 输入命令：gradlew assembleDebug
echo.
echo 方法D：使用快捷键
echo   - Ctrl+Shift+F9：重新编译项目
echo   - Shift+F10：运行项目
echo.
echo ================================================
echo 如果以上方法都不行，可能是以下原因：
echo.
echo 1. Android Studio版本问题
echo 2. 项目配置需要更新
echo 3. 需要安装Android SDK
echo.
echo 建议解决方案：
echo 1. 更新Android Studio到最新版本
echo 2. 确保已安装Android SDK和必要的构建工具
echo 3. 检查项目中的local.properties文件
echo ================================================
pause