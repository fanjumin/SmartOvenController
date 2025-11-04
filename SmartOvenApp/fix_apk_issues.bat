@echo off
chcp 65001 >nul

echo ========================================
echo    SmartOven App APK Issue Fix Script
echo ========================================
echo.

echo [1/5] Checking current APK files status...
if exist "app\build\outputs\apk\debug\app-debug.apk" (
    echo ✓ Debug APK exists, size: 
    for %%I in ("app\build\outputs\apk\debug\app-debug.apk") do echo  %%~zI bytes
) else (
    echo ✗ Debug APK not found
)

if exist "app\build\outputs\apk\release\app-release-unsigned.apk" (
    echo ✓ Release APK exists (unsigned)
    for %%I in ("app\build\outputs\apk\release\app-release-unsigned.apk") do echo  size: %%~zI bytes
) else (
    echo ✗ Release APK not found
)

echo.
echo [2/5] Generating installable debug APK...
call gradlew.bat assembleDebug

if %errorlevel% neq 0 (
    echo ✗ Debug build failed
    goto :error
)

if exist "app\build\outputs\apk\debug\app-debug.apk" (
    echo ✓ Debug APK generated successfully
    for %%I in ("app\build\outputs\apk\debug\app-debug.apk") do echo  file size: %%~zI bytes
    echo  ✓ This APK can be installed directly on devices
) else (
    echo ✗ Debug APK generation failed
    goto :error
)

echo.
echo [3/5] Generating release APK (requires signing)...
call gradlew.bat assembleRelease

if %errorlevel% neq 0 (
    echo ✗ Release build failed
    goto :error
)

if exist "app\build\outputs\apk\release\app-release-unsigned.apk" (
    echo ✓ Release APK generated successfully
    for %%I in ("app\build\outputs\apk\release\app-release-unsigned.apk") do echo  file size: %%~zI bytes
    echo  Note: This APK needs signing before installation
) else (
    echo ✗ Release APK generation failed
    goto :error
)

echo.
echo [4/5] Creating APK installation guide...
(
echo # APK Installation Guide
echo.
echo ## Available APK Files:
echo.
echo 1. **Debug Version (Recommended for testing)**
echo    - File: app\build\outputs\apk\debug\app-debug.apk
echo    - Status: Signed, ready for installation
echo    - Purpose: Development and testing
echo.
echo 2. **Release Version (Requires signing)**
echo    - File: app\build\outputs\apk\release\app-release-unsigned.apk
echo    - Status: Unsigned, cannot be installed directly
echo    - Purpose: For official release after signing
echo.
echo ## Installation Steps:
echo.
echo 1. Connect your phone to computer
echo 2. Enable "USB Debugging" on your phone
echo 3. Run this command to install debug APK:
echo    adb install app\build\outputs\apk\debug\app-debug.apk
echo.
echo ## Known Issues:
echo - Project has 30 lint warnings (does not affect installation)
echo - Recommended to view lint report in Android Studio for optimization
echo.
) > APK_INSTALL_GUIDE.txt

echo ✓ APK installation guide created: APK_INSTALL_GUIDE.txt

echo.
echo ========================================
echo   Fix completed successfully!
echo ========================================
echo.
echo Next steps:
echo 1. Use app-debug.apk for device testing
echo 2. Check APK_INSTALL_GUIDE.txt for detailed instructions
echo 3. Optimize lint warnings in Android Studio
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
echo 1. JDK 8 environment variables configuration
echo 2. Android SDK path settings
echo 3. Network connection status
echo.
pause

:end