@echo off
chcp 65001 >nul

echo ========================================
echo    Java Environment Fix Script
echo ========================================
echo.

echo [1/4] Checking current Java environment...
echo Java version:
java -version 2>&1
echo.

echo javac version:
javac -version 2>&1
echo.

echo [2/4] Setting JDK 8 environment variables...
set JAVA_HOME=C:\Program Files\Java\jdk1.8.0_202
set PATH=%JAVA_HOME%\bin;%PATH%

echo JAVA_HOME set to: %JAVA_HOME%
echo.

echo [3/4] Verifying Java installation...
echo Java version after setting environment:
"%JAVA_HOME%\bin\java" -version 2>&1
echo.

echo javac version after setting environment:
"%JAVA_HOME%\bin\javac" -version 2>&1
echo.

echo [4/4] Building APK with correct JDK...
call gradlew.bat clean assembleDebug

if %errorlevel% neq 0 (
    echo ✗ Build failed
    goto :error
)

echo.
echo ========================================
echo   Build completed successfully!
echo ========================================
echo.
echo APK files generated:
if exist "app\build\outputs\apk\debug\app-debug.apk" (
    echo ✓ Debug APK: app\build\outputs\apk\debug\app-debug.apk
    for %%I in ("app\build\outputs\apk\debug\app-debug.apk") do echo   Size: %%~zI bytes
)

if exist "app\build\outputs\apk\release\app-release-unsigned.apk" (
    echo ✓ Release APK: app\build\outputs\apk\release\app-release-unsigned.apk
    for %%I in ("app\build\outputs\apk\release\app-release-unsigned.apk") do echo   Size: %%~zI bytes
    echo   Note: This APK needs signing
)

echo.
echo Next steps:
echo 1. Install app-debug.apk on your device for testing
echo 2. Use adb install app\build\outputs\apk\debug\app-debug.apk
echo.

pause
goto :end

:error
echo.
echo ========================================
echo   Error occurred during build process
echo ========================================
echo.
echo Please check:
echo 1. JDK installation at C:\Program Files\Java\jdk1.8.0_202
echo 2. Android SDK configuration
echo.
pause

:end