# 智能电烤箱控制器 v0.8.3 发布说明

## 版本信息
- 版本号：v0.8.3
- 发布日期：2025年11月16日
- 兼容固件：v0.8.3

## 更新内容

### 版本号统一更新
- 将所有HTML文件及相关资源中的版本信息统一更新为v0.8.2
- 确保前端界面与固件版本保持一致

### 文件更新
- 更新了所有HTML页面中的版本显示
- 更新了JavaScript和Python服务端文件中的版本信息
- 生成了完整的v0.8.3版本发布包

## 文件列表
- `data/` - 包含所有HTML界面文件
  - `index.html` - 主控制界面
  - `settings_help.html` - 设置与帮助界面
  - `temperature_calibration.html` - 温度校准界面
  - `login.html` - 登录界面
  - `device_status.html` - 设备状态界面
  - `wifi_config.html` - WiFi配置界面
  - `mobile_utils.js` - 移动端工具脚本
  - `server.py` - 本地测试服务器
  - `css/title-styles.css` - 样式表文件

## 使用说明
1. 将`data`文件夹中的所有文件上传到设备的文件系统中
2. 确保设备固件版本也为v0.8.3以保证兼容性
3. 重启设备以应用更新

## 注意事项
- 更新前请备份现有配置
- 确保在稳定的网络环境下进行OTA升级
- 如遇到问题，请参考故障排除指南