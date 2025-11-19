import re
import json
import os

def load_translation_keys(file_path):
    """从chinese_strings_new.txt加载翻译键"""
    translations = {}
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    # 匹配格式: "中文字符串" : "translation_key"
    pattern = r'"([^"]+)"\s*:\s*"([^"]+)"'
    matches = re.findall(pattern, content)
    
    for chinese_text, key in matches:
        translations[chinese_text] = key
        
    return translations

def fix_hardcoded_chinese_in_html(file_path, translations):
    """修复HTML文件中的硬编码中文"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 创建一个副本用于替换
    new_content = content
    
    # 先修复重复的data-lang标签问题
    new_content = re.sub(
        r'<span data-lang="([^"]+)"><span data-lang="([^"]+)">([^<]+)</span></span>', 
        r'<span data-lang="\1">\3</span>', 
        new_content
    )
    
    # 特殊处理一些固定的中文文本
    special_replacements = {
        '正在连接设备...': 'connectingToDevice',
        '加热器正在工作': 'heaterWorking',
        '设备正在重启': 'deviceRebooting',
        '设备正在恢复出厂设置': 'factoryResetInProgress',
        '设备已恢复出厂设置，系统将自动重启。': 'factoryResetComplete'
    }
    
    # 处理特殊的纯文本替换
    for chinese_text, key in special_replacements.items():
        # 处理在HTML标签中的文本，避免重复替换
        # 先检查是否已经包含data-lang属性
        if f'data-lang="{key}"' not in new_content:
            pattern = f'>([^<]*{re.escape(chinese_text)}[^<]*)<'
            replacement = f'><span data-lang="{key}">\\1</span><'
            new_content = re.sub(pattern, replacement, new_content)
        
        # 处理在JavaScript中的文本
        new_content = new_content.replace(
            f"alert('{chinese_text}')", 
            f"alert(getTranslation('{key}'))"
        )
        
        # 处理在textContent中的文本
        new_content = new_content.replace(
            f"textContent = '{chinese_text}'", 
            f"textContent = getTranslation('{key}')"
        )
        
        # 处理在纯JavaScript字符串中的文本（避免误替换）
        js_pattern = f"(?<!data-lang=\")'{re.escape(chinese_text)}'"
        new_content = re.sub(js_pattern, f"getTranslation('{key}')", new_content)
    
    # 处理从chinese_strings_new.txt加载的翻译键
    for chinese_text, key in translations.items():
        # 跳过已经在data-lang属性中的文本
        if f'data-lang="{key}"' in new_content:
            continue
            
        # 替换HTML标签中的中文文本，避免重复替换
        escaped_chinese = re.escape(chinese_text)
        # 检查是否已经包含该中文文本但没有data-lang属性
        if f'>{chinese_text}<' in new_content and f'data-lang="{key}"' not in new_content:
            pattern = f'>([^<]*{escaped_chinese}[^<]*)<'
            replacement = f'><span data-lang="{key}">\\1</span><'
            new_content = re.sub(pattern, replacement, new_content)
    
    # 特别处理一些特殊情况
    # 处理"设备正在重启，请等待几分钟后重新连接。"
    if '设备正在重启，请等待几分钟后重新连接。' in new_content:
        new_content = re.sub(
            r'设备正在重启，请等待几分钟后重新连接。', 
            '<span data-lang="deviceRebooting">设备正在重启</span>，请等待几分钟后重新连接。', 
            new_content
        )
    
    # 最后再修复一次重复的data-lang标签问题
    new_content = re.sub(
        r'<span data-lang="([^"]+)"><span data-lang="([^"]+)">([^<]+)</span></span>', 
        r'<span data-lang="\1">\3</span>', 
        new_content
    )
    
    # 写入修复后的内容
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f"已修复 {file_path} 中的硬编码中文文本")

def main():
    # 加载翻译键
    translations = load_translation_keys('chinese_strings_new.txt')
    print(f"加载了 {len(translations)} 个翻译键")
    
    # 获取所有HTML文件
    html_files = [f for f in os.listdir('.') if f.endswith('.html')]
    
    # 修复所有HTML文件
    for html_file in html_files:
        fix_hardcoded_chinese_in_html(html_file, translations)
    
    print("所有HTML文件中的硬编码中文文本修复完成")

if __name__ == "__main__":
    main()