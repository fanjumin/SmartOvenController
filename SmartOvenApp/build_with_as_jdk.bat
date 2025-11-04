@echo off
echo ========================================
echo SmartOvenApp Build Script (Android Studio JDK)
echo ========================================
echo.

echo [1/3] Setting Android Studio JDK environment...
set JAVA_HOME=C:\Program Files\Android\Android Studio\jbr
echo JAVA_HOME set to: %JAVA_HOME%

echo [2/3] Checking Java version...
"%JAVA_HOME%\bin\java" -version

echo [3/3] Starting project build with module compatibility...
set GRADLE_OPTS=--add-opens java.base/java.io=ALL-UNNAMED --add-opens java.base/java.lang=ALL-UNNAMED --add-opens java.base/java.util=ALL-UNNAMED
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