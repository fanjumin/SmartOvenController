@echo off
cls

echo ===========================================
echo Smart Oven Controller HTML File Fetcher
echo ===========================================

set /p DEVICE_IP=Enter device IP address: 

mkdir data 2>nul

echo.
set HTML_FILES=index.html temperature_control.html wifi_config.html device_status.html settings_help.html

echo Starting to fetch HTML files from device...
echo -------------------------------------------

for %%f in (%HTML_FILES%) do (
    echo Fetching: %%f
    curl -s http://%DEVICE_IP%/%%f -o data/%%f
    if exist data/%%f (
        echo Success: %%f saved to data/%%f
    ) else (
        echo Failed: Cannot fetch %%f
    )
    echo.
)

echo -------------------------------------------
echo HTML fetch completed!
echo All files are saved in the data directory
echo.
echo Note: If files cannot be fetched, check:
1. Device IP address is correct
2. Device is online and accessible
3. Device web server is running properly
echo.
echo Press any key to exit...
pause > nul