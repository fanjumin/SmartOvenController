import os
import re
import json

def update_lang_file():
    """更新lang.js文件，添加新的翻译键和对应的中英文翻译"""
    
    # 读取现有的lang.js文件
    lang_file_path = os.path.join(os.path.dirname(__file__), 'lang.js')
    
    if not os.path.exists(lang_file_path):
        print("文件不存在:", lang_file_path)
        return
    
    with open(lang_file_path, 'r', encoding='utf-8') as f:
        lang_content = f.read()
    
    # 读取chinese_strings_new.txt文件中的中文字符串
    chinese_file_path = os.path.join(os.path.dirname(__file__), 'chinese_strings_new.txt')
    
    if not os.path.exists(chinese_file_path):
        print("文件不存在:", chinese_file_path)
        return
    
    with open(chinese_file_path, 'r', encoding='utf-8') as f:
        chinese_lines = f.readlines()
    
    # 收集所有中文字符串
    chinese_texts = []
    for line in chinese_lines:
        if line.strip() and not line.startswith('#'):
            # 提取引号中的内容
            if '"' in line:
                parts = line.split('"')
                if len(parts) >= 2:
                    chinese_text = parts[1]
                    # 检查是否包含中文字符
                    if any('\u4e00' <= char <= '\u9fff' for char in chinese_text):
                        chinese_texts.append(chinese_text)
    
    # 为每个中文字符串生成翻译键
    translation_dict = {}
    key_counts = {}  # 用于跟踪重复的键
    
    for text in chinese_texts:
        # 生成新的翻译键
        # 1. 转换为小写
        key = text.lower()
        # 2. 只保留中文字符、英文字母、数字和空格
        key = re.sub(r'[^\w\u4e00-\u9fff\s]', '', key)
        # 3. 将空格替换为下划线
        key = re.sub(r'\s+', '_', key)
        # 4. 移除首尾下划线
        key = key.strip('_')
        # 5. 如果键为空或太短，使用文本的拼音首字母（简化处理）
        if len(key) < 2:
            # 简单处理：使用文本的前几个字符
            key = ''.join([char if '\u4e00' <= char <= '\u9fff' else char for char in text[:10]])
            key = re.sub(r'[^\w\u4e00-\u9fff]', '', key)
        
        # 处理重复键
        base_key = key
        counter = 1
        while key in translation_dict.values() or key in [k for k in translation_dict.values()]:
            key = f"{base_key}_{counter}"
            counter += 1
        
        translation_dict[text] = key
    
    # 生成英文翻译
    en_translations = {
        "正在连接设备...": "Connecting to device...",
        "烘焙控制": "Baking Control",
        "系统管理": "System Management",
        "API端点": "API Endpoint",
        "IP地址": "IP Address",
        "OTA固件升级": "OTA Firmware Update",
        "WiFi信号": "WiFi Signal",
        "WiFi状态": "WiFi Status",
        "WiFi配置": "WiFi Configuration",
        "自定义模式": "Custom Mode",
        "手动模式": "Manual Mode",
        "中/En": "Zh/En",
        "修改密码": "Change Password",
        "内存使用": "Memory Usage",
        "加热状态": "Heating Status",
        "固件更新": "Firmware Update",
        "开始": "Start",
        "当前温度": "Current Temperature",
        "恢复出厂设置": "Factory Reset",
        "手动模式设置": "Manual Mode Settings",
        "控制加热器开关": "Control Heater Switch",
        "智能烤箱": "Smart Oven",
        "设备信息": "Device Information",
        "系统监控": "System Monitoring",
        "系统状态": "System Status",
        "正常": "Normal",
        "℃": "°C",
        "分钟": "minutes",
        "停止加热": "Stop Heating",
        "关闭加热": "Turn Off Heating",
        "加热器已关闭": "Heater Off",
        "加热器正在工作": "Heater Working",
        "加热已停止": "Heating Stopped",
        "启动加热": "Start Heating",
        "已停止": "Stopped",
        "已切换到手动模式，请设置温度和时间": "Switched to manual mode, please set temperature and time",
        "已切换到自动模式": "Switched to automatic mode",
        "已切换到自定义模式，您可以手动调整温度和时间": "Switched to custom mode, you can manually adjust temperature and time",
        "已连接": "Connected",
        "开启加热": "Turn On Heating",
        "披萨": "Pizza",
        "烤肉": "BBQ",
        "自定义": "Custom",
        "蛋糕": "Cake",
        "解冻": "Defrost",
        "面包": "Bread",
        "确定要退出登录吗？": "Are you sure you want to logout?",
        "确定要重启设备吗？设备将在5秒后重启。": "Are you sure you want to restart the device? It will restart in 5 seconds.",
        "网络已恢复": "Network restored",
        "网络已断开": "Network disconnected",
        "网络请求失败，请检查连接": "Network request failed, please check connection",
        "网络连接失败": "Network connection failed",
        "获取系统健康数据失败:": "Failed to get system health data:",
        "设备重启命令已发送，请等待约30秒后重新连接": "Device restart command sent, please wait about 30 seconds to reconnect",
        "设备重启命令已发送，设备将在几秒钟后重启。": "Device restart command sent, device will restart in a few seconds.",
        "密码修改成功": "Password changed successfully",
        "密码修改失败，请检查当前密码是否正确": "Password change failed, please check if current password is correct",
        "新密码输入不一致": "New passwords do not match",
        "退出登录失败": "Logout failed",
        "演示模式": "Demo Mode",
        "数据已刷新": "Data refreshed",
        "设置加热状态失败": "Failed to set heating status",
        "控制命令发送失败": "Control command sending failed",
        "控制设备失败:": "Device control failed:",
        "加热控制功能": "Heating Control Function",
        "温度调节功能": "Temperature Adjustment Function",
        "网络连接失败:": "Network connection failed:",
        "读取数据失败:": "Failed to read data:",
        "加载设置失败:": "Failed to load settings:",
        "删除数据失败:": "Failed to delete data:",
        "温度设置失败": "Temperature setting failed",
        "断开WiFi失败": "Failed to disconnect WiFi",
        "断开WiFi失败:": "Failed to disconnect WiFi:",
        "重启设备失败": "Failed to restart device",
        "重启设备失败:": "Failed to restart device:",
        "恢复出厂设置失败": "Factory reset failed",
        "恢复出厂设置失败:": "Factory reset failed:",
        "恢复出厂设置失败，密码可能不正确": "Factory reset failed, password may be incorrect",
        "恢复出厂设置命令发送失败": "Failed to send factory reset command",
        "恢复出厂设置命令已发送，设备将重启并进入配网模式": "Factory reset command sent, device will restart and enter configuration mode",
        "设备已恢复出厂设置，系统将自动重启。": "Device has been restored to factory settings, system will automatically restart.",
        "振动功能不可用:": "Vibration function not available:",
        "未处理的Promise拒绝:": "Unhandled Promise rejection:",
        "未知错误": "Unknown error",
        "未连接": "Not connected",
        "温度范围应为 50-250°C": "Temperature range should be 50-250°C",
        "时间范围应为 1-120 分钟": "Time range should be 1-120 minutes",
        "智能电烤箱管理员页面已加载": "Smart electric oven admin page loaded",
        "确定要恢复出厂设置吗？所有配置将被清除，设备将重启。": "Are you sure you want to restore factory settings? All configurations will be cleared and the device will restart.",
        "请求超时": "Request timeout",
        "请先输入要设置的温度": "Please enter the temperature to set first",
        "请输入当前密码：": "Please enter current password:",
        "请输入新密码：": "Please enter new password:",
        "请确认新密码：": "Please confirm new password:",
        "请输入管理员密码确认操作：": "Please enter admin password to confirm operation:",
        "获取设备状态失败:": "Failed to get device status:",
        "设置温度失败:": "Failed to set temperature:",
        "设置加热状态失败:": "Failed to set heating status:",
        "重启命令发送失败": "Failed to send restart command",
        "退出登录失败:": "Logout failed:",
        "连接检查失败:": "Connection check failed:",
        "页面错误:": "Page error:",
        "页面出现错误，请刷新重试": "Page error occurred, please refresh and try again",
        "饼干": "Cookies"
    }
    
    # 为没有预定义翻译的文本生成占位符
    for text in chinese_texts:
        if text not in en_translations:
            # 简单的占位符翻译：将中文字符转换为拼音首字母（这里简化处理）
            en_translations[text] = text  # 保持原样作为占位符
    
    # 提取现有的中文和英文翻译
    zh_pattern = r"'zh': \{([^}]+)\}"
    en_pattern = r"'en': \{([^}]+)\}"
    
    zh_match = re.search(zh_pattern, lang_content, re.DOTALL)
    en_match = re.search(en_pattern, lang_content, re.DOTALL)
    
    if not zh_match or not en_match:
        print("无法找到现有的中文或英文翻译部分")
        return
    
    # 获取现有的翻译内容
    existing_zh_content = zh_match.group(1)
    existing_en_content = en_match.group(1)
    
    # 创建新的翻译内容
    new_zh_entries = []
    new_en_entries = []
    
    # 添加新的翻译条目
    for text, key in translation_dict.items():
        # 转义引号
        escaped_text = text.replace('"', '\\"')
        escaped_en_text = en_translations[text].replace('"', '\\"')
        
        new_zh_entries.append(f"        '{key}': '{escaped_text}',")
        new_en_entries.append(f"        '{key}': '{escaped_en_text}',")
    
    # 如果没有新的条目需要添加，直接返回
    if not new_zh_entries and not new_en_entries:
        print("没有新的翻译条目需要添加")
        return
    
    # 插入新的翻译条目到现有内容中
    # 找到插入位置（在现有条目的最后）
    zh_last_entry_pattern = r"        '[^']+'\s*:\s*'[^']+',?\s*\n"
    en_last_entry_pattern = r"        '[^']+'\s*:\s*'[^']+',?\s*\n"
    
    zh_last_entry_matches = list(re.finditer(zh_last_entry_pattern, existing_zh_content))
    en_last_entry_matches = list(re.finditer(en_last_entry_pattern, existing_en_content))
    
    if not zh_last_entry_matches or not en_last_entry_matches:
        print("无法找到插入位置")
        return
    
    zh_last_entry_match = zh_last_entry_matches[-1]
    en_last_entry_match = en_last_entry_matches[-1]
    
    # 在最后一个条目后插入新的条目
    zh_insert_pos = zh_last_entry_match.end()
    en_insert_pos = en_last_entry_match.end()
    
    # 构建新的内容
    new_zh_content = existing_zh_content[:zh_insert_pos] + "\n" + "\n".join(new_zh_entries) + "\n" + existing_zh_content[zh_insert_pos:]
    new_en_content = existing_en_content[:en_insert_pos] + "\n" + "\n".join(new_en_entries) + "\n" + existing_en_content[en_insert_pos:]
    
    # 替换整个lang.js文件中的内容
    new_lang_content = re.sub(zh_pattern, f"'zh': {{\n{new_zh_content}\n    }}", lang_content, flags=re.DOTALL)
    new_lang_content = re.sub(en_pattern, f"'en': {{\n{new_en_content}\n    }}", new_lang_content, flags=re.DOTALL)
    
    # 写入更新后的内容到lang.js文件
    with open(lang_file_path, 'w', encoding='utf-8') as f:
        f.write(new_lang_content)
    
    print(f"成功添加了 {len(new_zh_entries)} 个新的翻译条目到 lang.js 文件中")

if __name__ == "__main__":
    update_lang_file()