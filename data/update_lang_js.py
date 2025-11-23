#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# 定义新的翻译键
new_translations = {
    "passwordChangeSuccessMessage": "密码修改成功！",
    "enterTemperatureInRange": "请输入 0°C 到 300°C 之间的温度值",
    "enterNewPasswordPrompt": "请输入新密码（至少8位）:",
    "confirmDeviceRestart": "确定要重启设备吗？设备将在5秒后重启。",
    "confirmFactoryResetWarning": "确定要恢复出厂设置吗？所有配置将被清除，设备将重启。",
    "confirmDeviceReboot": "确定要重启设备吗？重启过程可能需要几分钟时间。",
    "warningFactoryResetConfirmation": "⚠️ 警告：此操作将清除所有设置并恢复设备到出厂状态！\\n\\n确定要继续吗？",
    "confirmLogoutQuestion": "确定要退出登录吗？"
}

# 读取lang.js文件
with open('lang.js', 'r', encoding='utf-8') as f:
    content = f.read()

# 在适当的位置插入新的翻译键（在文件的翻译键部分）
# 找到一个已知的翻译键作为插入点
insertion_point = '"confirmLogout" : "确定要退出登录吗？",'

# 如果找不到精确的插入点，使用近似的插入点
if insertion_point not in content:
    insertion_point = '"confirmLogoutQuestion" : "确定要退出登录吗？",'

# 准备要插入的新翻译键字符串
new_entries = []
for key, value in new_translations.items():
    # 检查是否已存在
    if f'"{key}"' not in content:
        new_entries.append(f'  "{key}": "{value}",')

# 如果有新条目要添加
if new_entries:
    # 在插入点后添加新条目
    if insertion_point in content:
        # 找到插入点的位置
        insert_pos = content.find(insertion_point) + len(insertion_point)
        # 插入新条目
        new_content = content[:insert_pos] + '\n' + '\n'.join(new_entries) + content[insert_pos:]
    else:
        # 如果找不到插入点，在文件末尾添加
        new_content = content.rstrip() + '\n' + '\n'.join(new_entries) + '\n}'
    
    # 写回文件
    with open('lang.js', 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f"Added {len(new_entries)} new translation keys to lang.js")
else:
    print("All translation keys already exist in lang.js")

print("lang.js updated successfully!")