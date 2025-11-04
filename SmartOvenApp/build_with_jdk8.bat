@echo off
chcp 65001 >nul

echo ========================================
echo SmartOvenApp - Build with JDK 8
echo ========================================
echo.

echo [1/4] Setting JDK 8 environment variables...
set JAVA_HOME=C:\Program Files\Java\jdk1.8.0_202
echo JAVA_HOME set to: %JAVA_HOME%

set PATH=%JAVA_HOME%\bin;%PATH%

echo [2/4] Verifying Java installation...
java -version
echo.
javac -version
echo.

echo [3/4] Starting project build...
call gradlew.bat clean build

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo Build successful!
    echo APK location: app\build\outputs\apk\
    echo ========================================
) else (
    echo.
    echo ========================================
    echo Build failed. Please check error messages above.
    echo ========================================
)

echo [4/4] Done
pause