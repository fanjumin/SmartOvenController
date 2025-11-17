# 智能烤箱控制器 v0.8.3 发布说明

## 版本信息
- 版本号: v0.8.3
- 发布日期: 2025-11-16

## 包含文件
1. `firmware.bin` - 编译后的固件文件
2. `littlefs.bin` - 文件系统镜像（包含Web界面）
3. `SmartOvenController.ino` - 主程序源代码
4. `platformio.ini` - PlatformIO配置文件
5. `data/` - Web界面源文件

## 构建信息
- 固件大小: 377,952 字节
- 文件系统大小: 1,024,000 字节
- 构建时间: 2025-11-13 13:27

## 烧录说明

### 方法1: 使用PlatformIO命令行
```bash
# 烧录固件
pio run -t upload

# 烧录文件系统
pio run -t uploadfs
```

### 方法2: 使用esptool手动烧录
```bash
# 烧录固件 (地址: 0x00000)
esptool.exe --port COM11 write_flash 0x00000 firmware.bin

# 烧录文件系统 (地址: 0x300000)
esptool.exe --port COM11 write_flash 0x300000 littlefs.bin
```

## OTA升级
设备支持通过Web界面进行OTA升级：
1. 访问设备的Web界面
2. 进入OTA升级页面
3. 选择要上传的固件(.bin)或文件系统文件
4. 点击上传并等待设备重启

## 注意事项
1. 确保在platformio.ini中正确配置了串口端口
2. 烧录前请确保设备已正确连接并进入烧录模式
3. 文件系统地址(0x300000)必须与platformio.ini中的配置一致