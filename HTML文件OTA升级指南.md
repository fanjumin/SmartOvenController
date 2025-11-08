# HTML文件OTA升级指南

## 概述

本指南介绍如何通过OTA（空中升级）方式更新ESP8266智能烤箱控制器上的HTML文件，无需重新烧录固件。

## 当前解决方案

### 1. 固件已支持的功能

- ✅ **LittleFS文件系统** - 支持存储HTML、CSS、JS文件
- ✅ **HTTP文件上传接口** - 通过Web接口上传文件
- ✅ **动态HTML加载** - 从文件系统读取HTML文件
- ✅ **设备重启接口** - 应用更改后重启设备

### 2. 为什么不能编译成单个.bin文件

**技术限制：**
- ESP8266闪存分区：引导程序、应用程序、文件系统分离
- Arduino编译系统默认只生成应用程序固件
- 文件系统内容需要单独处理

**解决方案：**
- 使用分段升级：先升级固件，再升级HTML文件
- 使用PlatformIO可以配置完整固件包

## 使用方法

### 方法一：使用Python工具（推荐）

1. **确保设备正常运行**
   ```bash
   # 编译并上传固件
   arduino-cli compile --fqbn esp8266:esp8266:d1_mini src.ino
   arduino-cli upload -p COM11 --fqbn esp8266:esp8266:d1_mini src.ino
   ```

2. **连接设备WiFi热点**
   - 设备启动后创建热点：`SmartOven-XXXX`
   - 连接热点，默认IP：`192.168.4.1`

3. **运行OTA升级工具**
   ```bash
   python ota_html_upgrade.py
   ```

4. **工具将自动：**
   - 检查设备连接状态
   - 上传data文件夹中的所有HTML文件
   - 可选重启设备应用更改

### 方法二：手动上传

1. **修改HTML文件**
   - 编辑`data/`目录中的HTML文件
   - 保存更改

2. **使用curl命令上传**
   ```bash
   # 上传单个文件示例
   curl -X POST http://192.168.4.1/upload_html \
        -H "Content-Type: application/json" \
        -d '{"filename":"index.html","content":"<h1>新页面</h1>"}'
   ```

3. **重启设备**
   ```bash
   curl http://192.168.4.1/reboot
   ```

## 文件结构

```
SmartOvenController/
├── src/
│   └── src.ino              # 主固件（已支持HTML上传）
├── data/                    # HTML文件目录
│   ├── index.html          # 主页面
│   ├── device_status.html  # 设备状态页面
│   ├── login.html          # 登录页面
│   ├── settings_help.html  # 设置帮助页面
│   └── wifi_config.html    # WiFi配置页面
├── upload_html_files.py     # 串口上传工具
├── ota_html_upgrade.py      # OTA升级工具
└── HTML文件OTA升级指南.md   # 本指南
```

## 技术细节

### 固件功能

1. **文件系统支持**
   ```cpp
   #include <LittleFS.h>
   if (LittleFS.begin()) {
       // 文件系统初始化成功
   }
   ```

2. **HTML文件服务**
   ```cpp
   if (LittleFS.exists("/index.html")) {
       File file = LittleFS.open("/index.html", "r");
       webServer.streamFile(file, "text/html");
       file.close();
   }
   ```

3. **文件上传接口**
   ```cpp
   webServer.on("/upload_html", HTTP_POST, []() {
       // 处理文件上传
   });
   ```

### 升级流程

1. **设备启动** → 创建WiFi热点
2. **用户连接** → 连接到设备热点
3. **运行工具** → 上传HTML文件
4. **设备重启** → 应用新界面
5. **用户访问** → 查看新界面

## 故障排除

### 常见问题

1. **无法连接到设备**
   - 检查设备是否正常启动
   - 确认WiFi热点名称和密码
   - 检查IP地址是否正确（默认192.168.4.1）

2. **文件上传失败**
   - 检查文件编码（必须为UTF-8）
   - 确认文件大小不超过闪存限制
   - 检查网络连接稳定性

3. **界面不更新**
   - 上传后需要重启设备
   - 清除浏览器缓存
   - 检查文件路径是否正确

### 调试方法

1. **查看串口日志**
   ```bash
   arduino-cli monitor -p COM11 --config baudrate=115200
   ```

2. **检查文件系统**
   - 固件启动时会列出文件系统中的文件
   - 通过串口查看文件列表

## 高级功能

### 增量更新
可以只更新修改过的文件，减少传输时间。

### 版本控制
可以为HTML文件添加版本号，支持回滚功能。

### 安全验证
添加文件校验和验证，确保文件完整性。

## 总结

通过本解决方案，您可以：

- ✅ **独立更新HTML界面**，无需重新烧录固件
- ✅ **快速迭代UI设计**，实时查看效果
- ✅ **降低开发成本**，界面和逻辑分离
- ✅ **支持团队协作**，设计师和工程师并行工作

这种架构将固件功能与用户界面分离，大大提高了开发效率和用户体验。