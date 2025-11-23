#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import json

# 定义硬编码中文文本及其推荐的翻译键
hardcoded_texts = {
    "密码修改成功！": "passwordChangeSuccessMessage",
    "请输入 0°C 到 300°C 之间的温度值": "enterTemperatureInRange",
    "请输入新密码（至少8位）:": "enterNewPasswordPrompt",
    "确定要重启设备吗？设备将在5秒后重启。": "confirmDeviceRestart",
    "确定要恢复出厂设置吗？所有配置将被清除，设备将重启。": "confirmFactoryResetWarning",
    "确定要重启设备吗？重启过程可能需要几分钟时间。": "confirmDeviceReboot",
    "⚠️ 警告：此操作将清除所有设置并恢复设备到出厂状态！\n\n确定要继续吗？": "warningFactoryResetConfirmation",
    "确定要退出登录吗？": "confirmLogoutQuestion"
}

# 读取现有的翻译文件
with open('chinese_strings_new.txt', 'r', encoding='utf-8') as f:
    existing_content = f.read()

# 添加新的翻译键到文件末尾
with open('chinese_strings_new.txt', 'a', encoding='utf-8') as f:
    # 添加一个空行分隔
    f.write('\n')
    
    # 添加新的翻译键
    for chinese_text, translation_key in hardcoded_texts.items():
        # 检查是否已存在
        if chinese_text not in existing_content:
            f.write(f'"{chinese_text}" : "{translation_key}",\n')
            print(f'Added: "{chinese_text}" -> "{translation_key}"')
        else:
            print(f'Already exists: "{chinese_text}"')

print("Translation keys added successfully!")