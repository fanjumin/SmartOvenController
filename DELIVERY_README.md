# 智能烤箱控制器交付包

## 版本信息
- 版本号: v0.8.0
- 发布日期: 2025-11-10
- 项目状态: 已完成并测试通过

## 交付内容

### 1. 主要交付文件
- `SmartOvenController_v0.8.0.zip` - 完整项目发布包 (579,089字节)
  包含固件、文件系统、配置文件和使用说明

### 2. 源代码
- `src/SmartOvenController.ino` - 主程序文件
- `data/` - Web界面文件目录
  - `index.html` - 主控制界面
  - `login.html` - 登录页面
  - `device_status.html` - 设备状态页面
  - `wifi_config.html` - WiFi配置页面
  - `settings_help.html` - 设置帮助页面
  - `test_ota.html` - OTA测试页面
  - `temperature_calibration.html` - 温度校准页面

### 3. 构建产物 (在release目录和zip包中)
- `firmware.bin` - 设备固件 (375,392字节)
- `littlefs.bin` - 文件系统镜像 (1,024,000字节)
- `platformio.ini` - 构建配置文件

### 4. 文档资料
#### 快速入门
- `QUICK_START_GUIDE.md` - 快速上手指南

#### 完整文档
- `USER_MANUAL.md` - 用户手册
- `FINAL_PROJECT_SUMMARY.md` - 项目总结报告
- `PROJECT_COMPLETION_REPORT.md` - 项目完成报告

#### 技术指南
- `BUILD_INSTRUCTIONS.md` - 构建说明
- `代码烧录指南.md` - 烧录指南
- `统一OTA升级指南.md` - OTA升级指南
- `故障排除指南.md` - 故障排除指南
- `安装指南.md` - 安装指南

#### 专项文档
- `PID参数调优指南.md` - PID调优指南
- `物料清单.md` - 硬件物料清单
- `电路连接图.md` - 电路连接说明
- `状态指示灯说明.md` - 状态指示灯说明

## 功能特性
1. **温度监控**: 实时温度监测和显示
2. **加热控制**: PWM控制加热功率
3. **Web界面**: 响应式Web控制界面
4. **OTA升级**: 支持固件和文件系统在线升级
5. **WiFi配置**: Web界面配置WiFi网络
6. **移动端适配**: 支持手机浏览器访问
7. **安全认证**: 登录验证保护

## 使用说明

### 烧录固件
1. 解压 `SmartOvenController_v0.8.0.zip`
2. 使用esptool烧录固件和文件系统:
   ```bash
   esptool.exe --port COM11 write_flash 0x00000 firmware.bin
   esptool.exe --port COM11 write_flash 0x300000 littlefs.bin
   ```

### 初次使用
1. 连接硬件
2. 设备会创建WiFi热点 "SmartOven-XXXX"
3. 连接热点并配置WiFi网络
4. 访问设备IP地址开始使用

### OTA升级
1. 访问设备Web界面
2. 进入设置帮助页面的OTA升级标签
3. 选择固件或文件系统文件上传
4. 等待设备重启完成升级

## 技术支持
如遇到问题，请参考相关文档或联系技术支持。