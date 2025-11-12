@echo off
title Smart Oven Controller - Quick Test Tool
echo ==========================================
echo Smart Oven Controller - Quick Test Script
echo ==========================================

echo.
echo 1. Checking device online status...
ping -n 1 192.168.16.104 >nul
if %errorlevel% == 0 (
    echo    [OK] Device is online
) else (
    echo    [ERROR] Device is offline
    echo    Please check device power and network connection
    pause
    exit /b
)

echo.
echo 2. Running comprehensive functionality test...
python final_comprehensive_test.py

echo.
echo 3. Verifying page titles...
python check_page_title_utf8.py

echo.
echo ==========================================
echo Test completed
echo ==========================================
echo.
echo Press any key to exit...
pause >nul