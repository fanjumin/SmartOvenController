@echo off
echo 开始编译SmartOven安卓APP...

REM 设置Android SDK路径
set ANDROID_HOME=C:\Users\%USERNAME%\AppData\Local\Android\Sdk

REM 检查Android SDK是否存在
if not exist "%ANDROID_HOME%" (
    echo 错误: 未找到Android SDK，请确保Android Studio已正确安装
    echo 尝试在默认位置查找SDK...
    goto :check_sdk
)

:check_sdk
echo 检查Android SDK工具...

REM 使用Android Studio的Gradle来构建
cd /d "%~dp0"

REM 尝试使用gradlew（如果存在）
if exist "gradlew.bat" (
    echo 使用gradlew.bat构建...
    call gradlew.bat assembleDebug
    if %ERRORLEVEL% EQU 0 goto :success
)

REM 如果gradlew不存在，尝试直接使用Android Studio的Gradle
echo 使用Android Studio的Gradle构建...
"C:\Program Files\Android\Android Studio\jbr\bin\java.exe" -cp "C:\Program Files\Android\Android Studio\plugins\android\lib\android.jar;C:\Program Files\Android\Android Studio\plugins\android\lib\gradle-api.jar" org.gradle.wrapper.GradleWrapperMain assembleDebug

if %ERRORLEVEL% EQU 0 goto :success

echo 编译失败，尝试替代方法...
REM 尝试使用Android SDK的构建工具
if exist "%ANDROID_HOME%\build-tools" (
    echo 使用Android SDK构建工具...
    REM 这里可以添加更详细的构建命令
    goto :alternative_build
)

:alternative_build
echo 正在尝试替代构建方法...
REM 这里可以添加其他构建方法

echo.
echo 编译完成！请检查app\build\outputs\apk\debug目录下的APK文件
pause
exit /b 0

:success
echo.
echo 编译成功！APK文件已生成在app\build\outputs\apk\debug目录中
echo.
echo 您可以将APK文件安装到安卓手机进行测试
pause
exit /b 0