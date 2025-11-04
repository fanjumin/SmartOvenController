@echo off
echo ========================================
echo    Gradle 9.2.0 最终修复脚本
echo ========================================
echo.

echo [1/4] 检查当前Gradle配置...
type gradle\wrapper\gradle-wrapper.properties
echo.

echo [2/4] 清理Android Studio缓存...
if exist "%USERPROFILE%\.AndroidStudio*" (
    echo 清理Android Studio缓存目录...
    rmdir /s /q "%USERPROFILE%\.AndroidStudio*\system\caches" 2>nul
    rmdir /s /q "%USERPROFILE%\.AndroidStudio*\system\gradle" 2>nul
)

echo [3/4] 清理项目Gradle缓存...
if exist ".gradle" (
    echo 删除.gradle目录...
    rmdir /s /q .gradle 2>nul
)

echo [4/4] 验证本地Gradle文件...
if exist "..\gradle-9.2.0-bin.zip" (
    echo ✓ 找到 gradle-9.2.0-bin.zip 文件
) else (
    echo ✗ 未找到 gradle-9.2.0-bin.zip 文件
    goto error
)

echo.
echo ========================================
echo   修复完成！现在可以重新打开Android Studio
echo ========================================
echo.
echo 下一步操作：
echo 1. 关闭Android Studio（如果正在运行）
echo 2. 重新打开项目：F:\Github\SmartOvenController\SmartOvenApp
echo 3. 等待Gradle同步完成
echo 4. 使用Build菜单编译项目
echo.
pause
goto end

:error
echo.
echo 错误：未找到必要的Gradle文件
echo 请确保 gradle-9.2.0-bin.zip 文件存在于项目根目录
echo.
pause

:end