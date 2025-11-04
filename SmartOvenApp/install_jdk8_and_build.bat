@echo off
echo ========================================
echo SmartOvenApp - Install JDK 8 and Build
echo ========================================
echo.

echo [1/5] Checking current Java installation...
java -version 2>nul
if %ERRORLEVEL% neq 0 (
    echo Java is not installed or not in PATH
) else (
    echo Java is installed
)

echo [2/5] Checking for JDK installation...
where javac >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo JDK is already installed
    goto :build
)

echo [3/5] JDK not found. Attempting to use Android Studio JDK with compatibility fixes...
echo Setting up Android Studio JDK with module compatibility...

set JAVA_HOME=C:\Program Files\Android\Android Studio\jbr
set GRADLE_OPTS=--add-opens java.base/java.io=ALL-UNNAMED --add-opens java.base/java.lang=ALL-UNNAMED --add-opens java.base/java.util=ALL-UNNAMED --add-opens java.base/java.lang.reflect=ALL-UNNAMED --add-opens java.base/java.text=ALL-UNNAMED --add-opens java.desktop/java.awt.font=ALL-UNNAMED

echo [4/5] Creating Gradle properties with compatibility settings...
echo # Compatibility settings for Android Studio JDK > gradle_compat.properties
echo org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8 --add-opens java.base/java.io=ALL-UNNAMED --add-opens java.base/java.lang=ALL-UNNAMED --add-opens java.base/java.util=ALL-UNNAMED >> gradle_compat.properties

:build
echo [5/5] Starting project build...
call gradlew.bat clean build -Dorg.gradle.java.home="%JAVA_HOME%"

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo Build successful!
    echo APK location: app\build\outputs\apk\
    echo ========================================
) else (
    echo.
    echo ========================================
    echo Build failed. Possible solutions:
    echo 1. Install JDK 8 manually from Oracle website
    echo 2. Use Android Studio to build the project
    echo 3. Check the error messages above
    echo ========================================
)

pause