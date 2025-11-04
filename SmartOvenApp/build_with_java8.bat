@echo off
echo ========================================
echo SmartOvenApp Build Script (Java 8 Compatible)
echo ========================================
echo.

echo [1/3] Setting Java 8 environment...
set JAVA_HOME=C:\Program Files\Java\jre1.8.0_471
echo JAVA_HOME set to: %JAVA_HOME%

echo [2/3] Checking Java version...
"%JAVA_HOME%\bin\java" -version

echo [3/3] Starting project build...
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
    echo Build failed, please check error messages
    echo ========================================
)

pause