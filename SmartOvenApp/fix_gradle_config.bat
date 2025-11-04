@echo off
chcp 65001 >nul
echo ========================================
echo    Android Studio Gradle配置修复工具
echo ========================================
echo.

echo [1/3] 检查当前Gradle配置...
if exist "gradle\wrapper\gradle-wrapper.properties" (
    echo ✓ gradle-wrapper.properties文件存在
    type "gradle\wrapper\gradle-wrapper.properties"
) else (
    echo ✗ gradle-wrapper.properties文件不存在
)

echo.
echo [2/3] 检查settings.gradle配置...
if exist "settings.gradle" (
    echo ✓ settings.gradle文件存在
    echo 显示前10行内容：
    type "settings.gradle" | head -n 10
) else (
    echo ✗ settings.gradle文件不存在
)

echo.
echo [3/3] 创建Gradle本地缓存配置...
if not exist "gradle.properties" (
    echo 创建gradle.properties文件...
    (
echo # Gradle配置优化
echo org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8
echo org.gradle.parallel=true
echo org.gradle.daemon=true
echo android.useAndroidX=true
echo android.enableJetifier=true
    ) > gradle.properties
    echo ✓ gradle.properties文件已创建
) else (
    echo ✓ gradle.properties文件已存在
)

echo.
echo ========================================
echo           修复完成！
echo ========================================
echo.
echo 现在请重新打开Android Studio，应该可以正常加载项目了。
echo.
echo 如果仍然有问题，请尝试：
echo 1. 删除项目根目录下的 .gradle 文件夹
echo 2. 重新打开Android Studio
echo 3. 等待Gradle同步完成
echo.
pause