# 智能烤箱控制器交付包内容

## 交付文件
1. `SmartOvenController_v0.8.0.zip` - 完整的项目发布包（579,089字节）
2. `release/` - 发布目录，包含所有单独的文件

## 发布包内容详情
解压 `SmartOvenController_v0.8.0.zip` 后将包含以下文件：
- `firmware.bin` - 设备固件（375,392字节）
- `littlefs.bin` - 文件系统镜像（1,024,000字节）
- `platformio.ini` - 构建配置文件
- `BUILD_INSTRUCTIONS.md` - 构建说明
- `README.md` - 项目说明

## 使用说明
1. 将 `SmartOvenController_v0.8.0.zip` 下载到本地
2. 解压文件
3. 按照 `BUILD_INSTRUCTIONS.md` 或 `README.md` 中的说明进行烧录
4. 设备上电后，通过浏览器访问设备IP地址即可使用

## OTA升级功能
设备支持通过Web界面进行OTA升级：
1. 访问设备Web界面
2. 进入设置页面的OTA升级标签页
3. 选择要上传的固件(.bin)或文件系统文件
4. 点击上传并等待设备重启

## 技术支持
如遇到问题，请参考项目根目录下的以下文档：
- `故障排除指南.md`
- `代码烧录指南.md`
- `统一OTA升级指南.md`
- `USER_MANUAL.md`