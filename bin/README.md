# 智能烤箱控制器固件和文件系统镜像

## 文件说明

### 固件文件 (Firmware)
- `firmware-nodemcuv2.bin` - 完整版固件，包含MAX6675温度传感器支持
- `firmware-nodemcuv2-fixed.bin` - 精简版固件，不使用MAX6675库

### 文件系统镜像 (LittleFS)
- `littlefs-nodemcuv2.bin` - 完整版文件系统镜像，包含所有Web界面文件
- `littlefs-nodemcuv2-fixed.bin` - 精简版文件系统镜像

## 烧录说明

### 使用esptool烧录固件和文件系统：

1. 烧录固件：
   ```
   esptool.py --port COM3 write_flash 0x00000 firmware-nodemcuv2.bin
   ```

2. 烧录文件系统：
   ```
   esptool.py --port COM3 write_flash 0x300000 littlefs-nodemcuv2.bin
   ```

### 使用PlatformIO烧录：

1. 烧录固件：
   ```
   platformio run --target upload
   ```

2. 烧录文件系统：
   ```
   platformio run --target uploadfs
   ```

## 文件大小信息

- 固件大小：约383KB
- 文件系统大小：约1024KB

## 注意事项

1. 确保在烧录前正确连接ESP8266设备
2. 根据您的硬件配置选择合适的固件版本
3. 烧录文件系统前请确保设备已正确连接并处于烧录模式