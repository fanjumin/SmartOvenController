#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# 定义要查找的硬编码中文文本
texts = [
    '密码修改成功！',
    '请输入 0°C 到 300°C 之间的温度值',
    '请输入新密码（至少8位）:',
    '确定要重启设备吗？设备将在5秒后重启。',
    '确定要恢复出厂设置吗？所有配置将被清除，设备将重启。',
    '确定要重启设备吗？重启过程可能需要几分钟时间。',
    '⚠️ 警告：此操作将清除所有设置并恢复设备到出厂状态！\n\n确定要继续吗？',
    '确定要退出登录吗？'
]

# 读取index.html文件
with open('index.html', 'r', encoding='utf-8') as f:
    content = f.read()

# 检查每个文本是否存在
for text in texts:
    if text in content:
        print(f'Found: {text}')
    else:
        print(f'Not found: {text}')