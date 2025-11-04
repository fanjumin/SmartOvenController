@echo off
echo ========================================
echo SmartOvenApp - Download and Install JDK 8
echo ========================================
echo.

echo [1/6] Checking current Java installation...
java -version 2>nul
if %ERRORLEVEL% neq 0 (
    echo Java is not installed or not in PATH
) else (
    echo Java is installed
)

echo [2/6] Checking for JDK installation...
where javac >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo JDK is already installed
    goto :build
)

echo [3/6] JDK not found. Downloading JDK 8...
echo Please download JDK 8 from the official Oracle website:
echo https://www.oracle.com/java/technologies/javase/javase8-archive-downloads.html
echo.
echo After downloading, install JDK 8 to the default location.
echo.

:wait_installation
echo [4/6] Waiting for JDK 8 installation...
echo Please install JDK 8 and press any key to continue...
pause >nul

:check_installation
echo [5/6] Checking JDK installation...
where javac >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo JDK not found. Please make sure JDK 8 is installed correctly.
    echo.
    echo Installation instructions:
    echo 1. Download JDK 8 from Oracle website
    echo 2. Run the installer
    echo 3. Accept the license agreement
    echo 4. Use default installation path
    echo.
    goto :wait_installation
)

echo JDK found! Setting up environment...
for /f "tokens=2*" %%i in ('reg query "HKLM\SOFTWARE\JavaSoft\Java Development Kit\1.8" /v JavaHome 2^>nul') do set JAVA_HOME=%%j
if "%JAVA_HOME%"=="" (
    for /f "tokens=2*" %%i in ('reg query "HKLM\SOFTWARE\JavaSoft\JDK\1.8" /v JavaHome 2^>nul') do set JAVA_HOME=%%j
)
if "%JAVA_HOME%"=="" (
    echo Could not find JDK 8 installation path automatically.
    echo Please set JAVA_HOME environment variable manually.
    set JAVA_HOME=C:\Program Files\Java\jdk1.8.0_XXX
)

echo JAVA_HOME set to: %JAVA_HOME%

:build
echo [6/6] Starting project build with JDK 8...
set JAVA_HOME=%JAVA_HOME%
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

pause