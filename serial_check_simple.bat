@echo off
cls

echo ==========================
echo Serial Port Connection Check
echo ==========================
echo.

REM List available COM ports
echo Available COM ports:
echo.
mode | find "COM"
echo.

REM Instructions
echo Instructions:
echo 1. Make sure Python is installed
echo 2. Install pyserial: pip install pyserial
echo 3. Run: python serial_communication_tool.py
echo.
echo Press any key to exit...
pause > nul