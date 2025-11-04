# SmartOven 智能电烤箱控制器 - arduino-cli烧录指南

## 🚀 快速开始烧录

### 1. 检查设备连接
```bash
# 检查可用的开发板和端口
arduino-cli board list
```
应该显示类似：
```
COM11 serial   Serial Port (USB) Unknown
```

### 2. 编译代码
```bash
# 编译SmartOvenController.ino
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

### 3. 烧录到设备
```bash
# 烧录到COM11端口
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

## 📋 完整烧录流程

### 步骤1：环境准备
```bash
# 1. 更新核心索引
arduino-cli core update-index

# 2. 安装ESP8266核心
arduino-cli core install esp8266:esp8266

# 3. 检查安装的核心
arduino-cli core list
```

### 步骤2：安装必要的库
```bash
# 安装ESP8266WiFi库
arduino-cli lib install "ESP8266WiFi"

# 安装ESP8266WebServer库
arduino-cli lib install "ESP8266WebServer"

# 安装MAX6675库
arduino-cli lib install "MAX6675"

# 检查已安装的库
arduino-cli lib list
```

### 步骤3：修改WiFi配置
编辑 `SmartOvenController.ino` 文件，修改以下配置：
```cpp
// 修改为您的实际WiFi信息
const char* sta_ssid = "您的WiFi名称";
const char* sta_password = "您的WiFi密码";
```

### 步骤4：编译代码
```bash
# 基本编译
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

# 详细编译（显示更多信息）
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --verbose SmartOvenController.ino

# 带优化编译
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-Os" SmartOvenController.ino
```

### 步骤5：烧录到设备
```bash
# 基本烧录
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

# 详细烧录（显示进度）
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 --verbose SmartOvenController.ino

# 强制烧录（忽略警告）
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 --verify SmartOvenController.ino
```

## 🔧 高级烧录选项

### 批量烧录脚本
创建 `flash_all.bat` 批处理文件：
```batch
@echo off
echo 开始烧录SmartOven控制器...

:: 检查设备连接
arduino-cli board list

:: 编译代码
echo 正在编译代码...
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

if %errorlevel% neq 0 (
    echo 编译失败！请检查错误信息
    pause
    exit /b 1
)

:: 烧录到设备
echo 正在烧录到COM11...
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

if %errorlevel% neq 0 (
    echo 烧录失败！请检查设备连接
    pause
    exit /b 1
)

echo 烧录成功！
echo 请打开串口监视器查看设备状态
pause
```

### 多设备烧录
如果连接了多个设备：
```bash
# 检查所有可用端口
arduino-cli board list

# 烧录到特定端口
arduino-cli upload -p COM3 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
arduino-cli upload -p COM4 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

## 🔍 烧录验证

### 1. 串口监视器验证
```bash
# 打开串口监视器（波特率115200）
arduino-cli monitor -p COM11 --config baudrate=115200
```

应该看到类似输出：
```
✅ SmartOven控制器启动成功
📡 WiFi连接中...
🌐 连接到WiFi: 您的WiFi名称
📊 设备IP地址: 192.168.1.100
🔍 设备发现服务已启动
```

### 2. 快速验证脚本
创建 `verify_flash.bat`：
```batch
@echo off
echo 验证烧录结果...

:: 检查设备是否响应
arduino-cli board list | find "COM11"
if %errorlevel% neq 0 (
    echo 错误：COM11端口未检测到设备
    pause
    exit /b 1
)

echo 设备检测正常，正在检查串口输出...

:: 短暂打开串口监视器（5秒）
start /B arduino-cli monitor -p COM11 --config baudrate=115200 --timeout 5

echo 验证完成！
pause
```

## 🐛 故障排除

### 常见问题及解决方案

#### 1. 编译错误 - 库缺失
```bash
# 检查缺失的库
arduino-cli lib search "库名称"

# 安装特定版本的库
arduino-cli lib install "库名称@版本号"
```

#### 2. 烧录失败 - 端口被占用
```bash
# 检查端口状态
netstat -ano | findstr "COM11"

# 强制关闭占用端口的进程
taskkill /F /PID 进程ID
```

#### 3. 设备无法识别
```bash
# 重新扫描设备
arduino-cli board list --watch

# 检查驱动程序
pnputil /enum-devices /connected | findstr "USB"
```

#### 4. 烧录超时
```bash
# 增加超时时间
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 --timeout 60 SmartOvenController.ino

# 重置设备进入编程模式
# NodeMCU: 按住FLASH按钮，按RESET，松开FLASH
```

### 调试命令
```bash
# 详细模式查看编译过程
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --verbose SmartOvenController.ino

# 查看编译输出目录
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-path ./build SmartOvenController.ino

# 清理编译缓存
arduino-cli cache clean
```

## 📊 性能优化

### 编译优化选项
```bash
# 最小尺寸优化
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-Os" SmartOvenController.ino

# 调试版本（带符号）
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-g" SmartOvenController.ino

# 性能优化
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-O2" SmartOvenController.ino
```

### 批量烧录优化
创建 `flash_optimized.bat`：
```batch
@echo off
echo 优化烧录流程...

:: 并行编译和烧录准备
start /B arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

:: 等待编译完成
timeout /t 10

:: 批量烧录
for %%p in (COM3 COM4 COM5 COM6 COM7 COM8 COM9 COM10 COM11 COM12) do (
    arduino-cli board list | find "%%p" >nul
    if not errorlevel 1 (
        echo 烧录到 %%p...
        arduino-cli upload -p %%p --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
    )
)

echo 批量烧录完成！
pause
```

## 🔄 自动化脚本

### 完整的烧录脚本 `auto_flash.bat`
```batch
@echo off
title SmartOven自动烧录脚本
color 0A

echo ========================================
echo      SmartOven控制器自动烧录工具
echo ========================================
echo.

:: 步骤1：环境检查
echo [1/5] 检查环境...
arduino-cli --help >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误：arduino-cli未安装或未在PATH中
    pause
    exit /b 1
)

:: 步骤2：设备检测
echo [2/5] 检测设备...
set DEVICE_FOUND=0
for /f "tokens=*" %%i in ('arduino-cli board list') do (
    echo %%i | find "COM" >nul
    if !errorlevel! equ 0 set DEVICE_FOUND=1
)

if %DEVICE_FOUND% equ 0 (
    echo 警告：未检测到设备，请检查USB连接
    echo 继续执行编译...
)

:: 步骤3：编译代码
echo [3/5] 编译代码...
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
if %errorlevel% neq 0 (
    echo 错误：编译失败
    pause
    exit /b 1
)

echo 编译成功！

:: 步骤4：烧录到设备
echo [4/5] 烧录到设备...
if %DEVICE_FOUND% equ 1 (
    arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
    if %errorlevel% neq 0 (
        echo 错误：烧录失败
        pause
        exit /b 1
    )
    echo 烧录成功！
else
    echo 跳过烧录（未检测到设备）
fi

:: 步骤5：验证
echo [5/5] 验证烧录...
echo 请手动打开串口监视器查看设备状态
echo 命令：arduino-cli monitor -p COM11 --config baudrate=115200

echo.
echo ========================================
echo     烧录流程完成！
echo ========================================
pause
```

## 📞 技术支持

### 获取帮助
```bash
# 查看arduino-cli帮助
arduino-cli help
arduino-cli compile --help
arduino-cli upload --help

# 查看核心信息
arduino-cli core search esp8266
```

### 日志和调试
```bash
# 启用详细日志
arduino-cli --log-level debug compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

# 保存日志到文件
arduino-cli --log-level debug --log-file arduino.log compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

---

## 🎯 烧录成功标志

当您看到以下输出时，表示烧录成功：

```
✅ 编译成功 - 代码大小：xxx bytes
✅ 烧录成功 - 设备已更新
✅ 串口输出显示设备正常启动
✅ Web界面可以正常访问
```

**使用arduino-cli烧录比Arduino IDE更快速、更稳定！** 🚀