@echo off
echo 创建测试文件...
echo This is a test file for filesystem update > littlefs.bin

echo.
echo 使用curl测试文件系统更新功能...
echo 请确保设备已连接并处于配网模式（默认IP为192.168.4.1）
echo.

curl -X POST http://192.168.4.1/update ^
     -F "filesystem=@littlefs.bin" ^
     -H "Content-Type: multipart/form-data"

echo.
echo 测试完成
pause