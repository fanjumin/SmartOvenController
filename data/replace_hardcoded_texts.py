#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re

# 定义硬编码中文文本及其对应的翻译键
replacements = {
    "密码修改成功！": "t('passwordChangeSuccessMessage')",
    "温度范围必须在 0°C 到 300°C 之间": "t('temperatureRangeError')",
    "请输入 0°C 到 300°C 之间的温度值": "t('enterTemperatureInRange')",
    "请输入新密码（至少8位）:": "t('enterNewPasswordPrompt')",
    "确定要重启设备吗？设备将在5秒后重启。": "t('confirmDeviceRestart')",
    "确定要恢复出厂设置吗？所有配置将被清除，设备将重启。": "t('confirmFactoryResetWarning')",
    "确定要重启设备吗？重启过程可能需要几分钟时间。": "t('confirmDeviceReboot')",
    "⚠️ 警告：此操作将清除所有设置并恢复设备到出厂状态！\\n\\n确定要继续吗？": "t('warningFactoryResetConfirmation')",
    "确定要退出登录吗？": "t('confirmLogoutQuestion')"
}

# 读取index.html文件
with open('index.html', 'r', encoding='utf-8') as f:
    content = f.read()

# 替换硬编码文本
modified_content = content
replacements_made = []

for chinese_text, translation_key in replacements.items():
    # 计算替换前的数量
    count_before = modified_content.count(chinese_text)
    
    if count_before > 0:
        # 转义特殊字符用于正则表达式
        escaped_text = re.escape(chinese_text)
        # 替换alert、prompt、confirm中的文本
        modified_content = re.sub(
            f'(alert|prompt|confirm)\\s*\\(\\s*[\'\"]{escaped_text}[\'\"]\\s*\\)',
            f'\\1({translation_key})',
            modified_content
        )
        
        # 计算替换后的数量
        count_after = modified_content.count(chinese_text)
        replaced_count = count_before - count_after
        if replaced_count > 0:
            replacements_made.append(f'Replaced "{chinese_text}" with "{translation_key}" ({replaced_count} times)')

# 将修改后的内容写回文件
with open('index.html', 'w', encoding='utf-8') as f:
    f.write(modified_content)

print("Replacement summary:")
for replacement in replacements_made:
    print(f"  {replacement}")

print(f"\nTotal {len(replacements_made)} replacements made.")
print("Hardcoded Chinese texts replaced successfully!")