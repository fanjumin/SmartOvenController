import re
import shutil
from datetime import datetime

def load_translations():
    """Load existing translations from lang.js"""
    translations = {}
    try:
        with open('data/lang.js', 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Extract the zh translations
        zh_match = re.search(r'"zh"\s*:\s*{([^}]+)}', content, re.DOTALL)
        if zh_match:
            zh_content = zh_match.group(1)
            # Parse key-value pairs
            pairs = re.findall(r'"([^"]+)"\s*:\s*"([^"]+)"', zh_content)
            for key, value in pairs:
                translations[value] = key
    except Exception as e:
        print(f"Error loading translations: {e}")
    
    return translations

def create_translation_key(chinese_text, existing_keys):
    """Create a translation key from Chinese text"""
    # Simple mapping for common words
    word_map = {
        '设备': 'device', '设置': 'settings', '配置': 'config', '清除': 'clear',
        '重启': 'restart', '恢复': 'restore', '出厂': 'factory', '状态': 'status',
        '加热': 'heating', '温度': 'temperature', '时间': 'time', '模式': 'mode',
        '自动': 'auto', '手动': 'manual', '自定义': 'custom', '切换': 'switch',
        '显示': 'show', '提示': 'message', '确认': 'confirm', '取消': 'cancel',
        '保存': 'save', '失败': 'failed', '成功': 'success', '错误': 'error',
        '警告': 'warning', '信息': 'info', '网络': 'network', '连接': 'connect',
        '断开': 'disconnect', '密码': 'password', '登录': 'login', '登出': 'logout',
        '系统': 'system', '监控': 'monitor', '正常': 'normal', '分钟': 'minutes',
        '开始': 'start', '停止': 'stop', '关闭': 'turnOff', '开启': 'turnOn',
        '工作': 'working', '将': 'will', '被': 'be', '请': 'please', '输入': 'enter',
        '选择': 'select', '更新': 'update', '固件': 'firmware', 'WiFi': 'wifi'
    }
    
    # Generate key based on first characters
    key_parts = []
    for char in chinese_text:
        if char in word_map:
            key_parts.append(word_map[char])
        else:
            # Use pinyin-like representation
            key_parts.append(char)
    
    base_key = ''.join(key_parts)
    # Ensure uniqueness
    counter = 1
    final_key = base_key
    while final_key in existing_keys:
        final_key = f"{base_key}{counter}"
        counter += 1
    
    return final_key

def fix_hardcoded_chinese():
    """Fix hardcoded Chinese text in HTML files"""
    # Backup the original file
    backup_name = f"data/index.html.backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # Load existing translations
    translations = load_translations()
    existing_keys = set(translations.values())
    
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Find hardcoded Chinese text (more comprehensive approach)
    # Look for Chinese characters that are not inside data-lang attributes
    # We'll look for Chinese text between > and < tags
    chinese_pattern = r'(>[^<]*[\u4e00-\u9fff]+[^<]*<)'
    
    # Find all matches
    matches = re.findall(chinese_pattern, content)
    
    # Process matches
    key_mapping = {}
    replacements = []
    
    for match in matches:
        # Extract the text content (remove > and <)
        text_content = match[1:-1]
        
        # Skip if it's just whitespace or very short
        if len(text_content.strip()) < 2:
            continue
        
        # Skip if it already contains data-lang attribute
        if 'data-lang=' in match:
            continue
            
        # Skip if it looks like a URL or code snippet
        if 'http' in text_content or 'www' in text_content:
            continue
            
        # Check if we already have a translation for this text
        if text_content in translations:
            key = translations[text_content]
        else:
            # Create translation key if not exists
            key = create_translation_key(text_content, existing_keys)
            translations[text_content] = key
            existing_keys.add(key)
            key_mapping[text_content] = key
            print(f"Created translation key: '{text_content}' -> '{key}'")
        
        # Prepare replacement
        replacement = f'>{text_content}<'
        new_replacement = f'><span data-lang="{key}">{text_content}</span><'
        replacements.append((replacement, new_replacement))
    
    # Apply replacements
    for old, new in replacements:
        content = content.replace(old, new)
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    # Update lang.js with new translations
    if key_mapping:
        update_lang_js(key_mapping)
    
    print(f"Fixed {len(key_mapping)} hardcoded Chinese texts")
    return len(key_mapping)

def update_lang_js(new_translations):
    """Update lang.js with new translations"""
    try:
        # Read current lang.js
        with open('data/lang.js', 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Add new translations to both zh and en sections
        for chinese_text, key in new_translations.items():
            # Add to zh section
            zh_pattern = r'("zh"\s*:\s*{)'
            replacement_zh = f'\\1\n    "{key}": "{chinese_text}",'
            content = re.sub(zh_pattern, replacement_zh, content, count=1)
            
            # Add to en section with placeholder
            en_pattern = r'("en"\s*:\s*{)'
            english_placeholder = ''.join([c if ord(c) < 128 else '?' for c in chinese_text])
            replacement_en = f'\\1\n    "{key}": "{english_placeholder}",'
            content = re.sub(en_pattern, replacement_en, content, count=1)
        
        # Write back to file
        with open('data/lang.js', 'w', encoding='utf-8') as f:
            f.write(content)
            
        print(f"Updated lang.js with {len(new_translations)} new translations")
    except Exception as e:
        print(f"Error updating lang.js: {e}")

if __name__ == "__main__":
    count = fix_hardcoded_chinese()
    print(f"Process completed. Fixed {count} instances.")