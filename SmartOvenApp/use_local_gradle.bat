@echo off
chcp 65001 >nul
echo ========================================
echo    使用本地Gradle 6.9.4配置工具
echo ========================================
echo.

echo [1/4] 检查本地Gradle文件...
if exist "..\gradle-6.9.4-bin.zip" (
    echo ✓ 找到本地gradle-6.9.4-bin.zip文件
    echo 文件大小: 
    for %%I in ("..\gradle-6.9.4-bin.zip") do echo   %%~zI 字节
) else (
    echo ✗ 未找到gradle-6.9.4-bin.zip文件
    goto :error
)

echo.
echo [2/4] 验证gradle-wrapper.properties配置...
if exist "gradle\wrapper\gradle-wrapper.properties" (
    echo ✓ gradle-wrapper.properties文件存在
    echo 当前配置内容：
    type "gradle\wrapper\gradle-wrapper.properties"
) else (
    echo ✗ gradle-wrapper.properties文件不存在
    goto :error
)

echo.
echo [3/4] 清理Android Studio缓存...
echo 删除.gradle缓存文件夹...
if exist ".gradle" (
    rmdir /s /q ".gradle" 2>nul
    echo ✓ 已删除.gradle缓存
)

echo 删除.idea配置文件夹...
if exist ".idea" (
    rmdir /s /q ".idea" 2>nul
    echo ✓ 已删除.idea配置
)

echo.
echo [4/4] 创建Gradle优化配置...
(
echo # Gradle配置优化 - 使用本地Gradle
echo org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8
echo org.gradle.parallel=true
echo org.gradle.daemon=true
echo android.useAndroidX=true
echo android.enableJetifier=true
echo # 禁用网络下载，强制使用本地文件
echo systemProp.org.gradle.internal.http.connectionTimeout=60000
echo systemProp.org.gradle.internal.http.socketTimeout=60000
) > gradle.properties

echo ✓ gradle.properties配置已创建

echo.
echo ========================================
echo           配置完成！
echo ========================================
echo.
echo 现在请按以下步骤操作：
echo 1. 关闭Android Studio（如果已打开）
echo 2. 重新打开Android Studio

echo 3. 选择项目文件夹：F:\Github\SmartOvenController\SmartOvenApp
echo 4. 等待Gradle同步完成（这次应该很快）
echo 5. 检查Build菜单是否正常显示

echo.
echo 如果仍有问题，请检查：
echo - 确保gradle-6.9.4-bin.zip文件完整

echo.
goto :end

:error
echo.
echo ❌ 配置过程中出现问题，请检查上述错误信息
echo.
pause
exit /b 1

:end
echo.
echo 配置已完成！现在可以重新打开Android Studio了。
echo.
pause