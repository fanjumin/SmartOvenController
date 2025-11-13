# 智能烤箱控制器构建说明

## 构建环境
- PlatformIO (通过Python安装)
- ESP8266开发环境
- Arduino框架

## 构建步骤

### 1. 构建固件
```bash
pio run
```

这将生成固件文件：
- `.pio/build/nodemcuv2/firmware.bin` (大小: 375,392 字节)

### 2. 构建文件系统
```bash
pio run -t buildfs
```

这将生成文件系统镜像：
- `.pio/build/nodemcuv2/littlefs.bin` (大小: 1,024,000 字节)

## 烧录步骤

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
esptool.exe --port COM11 write_flash 0x00000 .pio/build/nodemcuv2/firmware.bin

# 烧录文件系统 (地址: 0x300000，根据platformio.ini配置)
esptool.exe --port COM11 write_flash 0x300000 .pio/build/nodemcuv2/littlefs.bin
```

## OTA升级
设备支持通过Web界面进行OTA升级：
1. 访问设备的Web界面
2. 进入OTA升级页面
3. 选择要上传的固件(.bin)或文件系统文件
4. 点击上传并等待设备重启

## 文件说明
- `firmware.bin`: 设备固件，包含所有控制逻辑
- `littlefs.bin`: 文件系统镜像，包含Web界面文件(index.html等)
- `platformio.ini`: 项目配置文件，定义了硬件、库依赖和构建选项

## 注意事项
1. 确保在platformio.ini中正确配置了串口端口
2. 烧录前请确保设备已正确连接并进入烧录模式
3. 文件系统地址(0x300000)必须与platformio.ini中的配置一致