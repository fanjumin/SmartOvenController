#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
重建index.html文件，确保使用正确的UTF-8编码
"""

import re
import json

def load_translation_keys(file_path):
    """从chinese_strings_new.txt加载翻译键"""
    translations = {}
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    # 匹配格式: "中文字符串" : "translation_key"
    pattern = r'"([^"]+)"\s*:\s*"([^"]+)"'
    matches = re.findall(pattern, content)
    
    for chinese_text, key in matches:
        translations[key] = chinese_text
        
    return translations

def rebuild_index_html():
    """重建index.html文件"""
    # 加载翻译键
    translations = load_translation_keys('chinese_strings_new.txt')
    print(f"加载了 {len(translations)} 个翻译键")
    
    # 读取原始index.html文件
    with open('index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 替换标题中的乱码
    if 'æºè½ç¤ç®± - ç®¡çåæ§å¶é¢æ¿' in content:
        correct_title = f"{translations.get('智能烤箱', '智能烤箱')} - {translations.get('管理员控制面板', '管理员控制面板')}"
        content = content.replace(
            '<title data-lang="ovenControlPanelTitle">æºè½ç¤ç®± - ç®¡çåæ§å¶é¢æ¿</title>',
            f'<title data-lang="ovenControlPanelTitle">{correct_title}</title>'
        )
        print(f"已修复标题: {correct_title}")
    
    # 替换JavaScript注释中的乱码
    if 'é¡µé¢å è½½æ¶æ£æ¥ç»å½ç¶æ' in content:
        content = content.replace(
            '// é¡µé¢å è½½æ¶æ£æ¥ç»å½ç¶æ',
            '// 页面加载时检查登录状态'
        )
        print("已修复JavaScript注释")
    
    if 'æªç»å½ï¼è·³è½¬å°ç»å½é¡µé¢' in content:
        content = content.replace(
            '// æªç»å½ï¼è·³è½¬å°ç»å½é¡µé¢',
            '// 未登录，跳转到登录页面'
        )
        print("已修复未登录提示注释")
    
    if 'ç»åºåè½' in content:
        content = content.replace(
            '// ç»åºåè½',
            '// 登出功能'
        )
        print("已修复登出功能注释")
    
    if 'ä¿®æ¹å¯ç åè½' in content:
        content = content.replace(
            '// ä¿®æ¹å¯ç åè½',
            '// 修改密码功能'
        )
        print("已修复修改密码功能注释")
    
    # 保存修复后的文件
    with open('index.html', 'w', encoding='utf-8', newline='\n') as f:
        f.write(content)
    
    print("index.html文件重建完成!")

if __name__ == "__main__":
    rebuild_index_html()