@echo off
chcp 65001 >nul
echo ========================================
echo       完全离线编译解决方案
echo ========================================
echo.

echo [1/5] 检查本地Gradle版本...
if exist "..\gradle-7.6.4" (
    echo ✓ 找到Gradle 7.6.4（推荐使用）
    set GRADLE_VERSION=7.6.4
) else if exist "..\gradle-9.2.0" (
    echo ✓ 找到Gradle 9.2.0
    set GRADLE_VERSION=9.2.0
) else if exist "..\gradle-6.9.4" (
    echo ✓ 找到Gradle 6.9.4
    set GRADLE_VERSION=6.9.4
) else (
    echo ✗ 未找到本地Gradle版本
    goto :error
)

echo.
echo [2/5] 配置使用本地Gradle %GRADLE_VERSION%...
(
echo distributionBase=GRADLE_USER_HOME
echo distributionPath=wrapper/dists
echo # 使用本地Gradle %GRADLE_VERSION% 完全离线
echo distributionUrl=file\:///F:/Github/SmartOvenController/gradle-%GRADLE_VERSION%-bin.zip
echo zipStoreBase=GRADLE_USER_HOME
echo zipStorePath=wrapper/dists
) > gradle\wrapper\gradle-wrapper.properties

echo ✓ Gradle配置已更新为使用 %GRADLE_VERSION%

echo.
echo [3/5] 创建离线编译配置...
(
echo # 完全离线编译配置
echo org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8
echo org.gradle.parallel=true
echo org.gradle.daemon=true
echo android.useAndroidX=true
echo android.enableJetifier=true
echo # 强制离线模式
echo org.gradle.offline=true
echo systemProp.org.gradle.internal.http.connectionTimeout=60000
echo systemProp.org.gradle.internal.http.socketTimeout=60000
) > gradle.properties

echo ✓ 离线配置已创建

echo.
echo [4/5] 清理缓存...
if exist ".gradle" (
    rmdir /s /q ".gradle" 2>nul
    echo ✓ 已清理.gradle缓存
)

if exist ".idea" (
    rmdir /s /q ".idea" 2>nul
    echo ✓ 已清理.idea配置
)

echo.
echo [5/5] 创建命令行编译脚本...
(
echo @echo off
echo chcp 65001 ^>nul
echo echo 开始离线编译...
echo.
echo echo 使用本地Gradle编译...
echo ..\gradle-%GRADLE_VERSION%\bin\gradle.bat clean assembleDebug
echo.
echo echo 编译完成！检查app\build\outputs\apk\目录
echo if exist "app\build\outputs\apk\" (
echo     dir "app\build\outputs\apk\"
echo     echo ✓ APK文件已生成
echo ) else (
echo     echo ✗ 编译失败，未生成APK文件
echo )
echo pause
) > compile_with_local_gradle.bat

echo ✓ 命令行编译脚本已创建

echo.
echo ========================================
echo           离线配置完成！
echo ========================================
echo.
echo 现在您有两个选择：
echo.
echo 选项1：使用Android Studio（推荐）
echo   1. 关闭Android Studio
echo   2. 重新打开项目
echo   3. 等待Gradle同步（这次使用本地文件）
echo   4. 使用Build菜单编译
echo.
echo 选项2：使用命令行编译
echo   运行： .\compile_with_local_gradle.bat
echo.
echo 当前配置：使用Gradle %GRADLE_VERSION% 完全离线模式
echo.
goto :end

:error
echo.
echo ❌ 配置过程中出现问题
echo 请确保有可用的本地Gradle版本
echo.
pause
exit /b 1

:end
echo.
echo 配置已完成！现在可以尝试编译了。
echo.
pause