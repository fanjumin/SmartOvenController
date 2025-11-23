import re
import shutil
import json
from datetime import datetime

def final_comprehensive_fix():
    """最终综合修复脚本"""
    print("Starting final comprehensive fix...")
    
    # 备份原文件
    backup_name = f"data/index.html.final_comprehensive_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # 读取文件
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 第一步：修复已知的乱码模式
    known_garbled_patterns = {
        'è®¾å¤‡': '设备',
        'çŠ¶æ€': '状态',
        'è®¾ç½®': '设置',
        'åˆ†é’Ÿ': '分钟',
        'åŠ çƒ­': '加热',
        'æ¸©åº¦': '温度',
        'æ¨¡å¼': '模式',
        'è‡ªåŠ¨': '自动',
        'æ‰‹åŠ¨': '手动',
        'è‡ªå®šä¹‰': '自定义',
        'åˆ‡æ¢': '切换',
        'æç¤º': '提示',
        'ç¡®è®¤': '确认',
        'å–æ¶ˆ': '取消',
        'ä¿å­˜': '保存',
        'å¤±è´¥': '失败',
        'æˆåŠŸ': '成功',
        'é”™è¯¯': '错误',
        'è­¦å‘Š': '警告',
        'ä¿¡æ¯': '信息',
        'ç½‘ç»œ': '网络',
        'è¿žæŽ¥': '连接',
        'æ–­å¼€': '断开',
        'å¯†ç ': '密码',
        'ç™»å½•': '登录',
        'ç™»å‡º': '登出',
        'ç³»ç»Ÿ': '系统',
        'ç›‘æŽ§': '监控',
        'æ­£å¸¸': '正常',
        'å¼€å§‹': '开始',
        'åœæ­¢': '停止',
        'å…³é—­': '关闭',
        'å¼€å¯': '开启',
        'å·¥ä½œ': '工作',
        'è¯·è¾“å…¥': '请输入',
        'é€‰æ‹©': '选择',
        'æ›´æ–°': '更新',
        'å›ºä»¶': '固件',
        'é‡å¯': '重启',
        'æ¢å¤': '恢复',
        'å‡ºåŽ‚': '出厂',
        'æ¸…é™¤': '清除',
        'å°†è¢«': '将被',
        'è¢«æ¸…é™¤': '被清除',
        'æ˜¾ç¤º': '显示',
        'åŠŸèƒ½åŒºåŸŸå“åº”å¼ä¼˜åŒ–': '功能区域响应式优化'
    }
    
    fixed_count = 0
    for garbled, fixed in known_garbled_patterns.items():
        count = content.count(garbled)
        if count > 0:
            content = content.replace(garbled, fixed)
            fixed_count += count
            print(f"Replaced '{garbled}' with '{fixed}' ({count} occurrences)")
    
    print(f"Fixed {fixed_count} known garbled patterns")
    
    # 第二步：查找并处理HTML标签中的中文文本
    # 移除script和style标签内容以便只处理可见文本
    content_for_analysis = re.sub(r'<(script|style)[^>]*>.*?</\\1>', '', content, flags=re.DOTALL)
    
    # 查找标签之间的中文文本
    tag_contents = re.findall(r'>[^<]*[\\u4e00-\\u9fff][^<]*<', content_for_analysis)
    chinese_tag_texts = []
    for match in tag_contents:
        # 清理匹配结果
        cleaned = match.strip('><').strip()
        # 只保留包含中文且不是纯空白的文本
        if cleaned and re.search(r'[\\u4e00-\\u9fff]', cleaned):
            chinese_tag_texts.append(cleaned)
    
    print(f"Found {len(chinese_tag_texts)} Chinese texts in HTML tags that may need internationalization")
    
    # 第三步：查找JavaScript中的中文文本
    js_strings = re.findall(r'[\'\"]([^\'\"]*[\\u4e00-\\u9fff][^\'\"]*)[\'\"]', content)
    chinese_js_texts = []
    for match in js_strings:
        if re.search(r'[\\u4e00-\\u9fff]', match):
            chinese_js_texts.append(match)
    
    print(f"Found {len(chinese_js_texts)} Chinese texts in JavaScript that may need internationalization")
    
    # 第四步：更新lang.js文件以包含新的翻译键
    try:
        with open('data/lang.js', 'r', encoding='utf-8') as f:
            lang_content = f.read()
        
        # 解析现有的翻译对象
        cn_translations_match = re.search(r'const CN = \\{[^}]+\\}', lang_content, re.DOTALL)
        if cn_translations_match:
            cn_section = cn_translations_match.group(0)
            # 提取所有现有键
            existing_keys = re.findall(r'"([^"]+)"\\s*:', cn_section)
            
            # 为新发现的中文文本创建翻译键
            new_translations = {}
            key_counter = len(existing_keys)
            
            # 处理HTML标签中的中文文本
            for text in chinese_tag_texts[:20]:  # 限制处理数量以避免过度处理
                if text not in existing_keys and len(text.strip()) > 1:
                    key = f"autoKey{key_counter}"
                    new_translations[key] = text
                    key_counter += 1
            
            # 处理JavaScript中的中文文本
            for text in chinese_js_texts[:20]:  # 限制处理数量以避免过度处理
                if text not in existing_keys and len(text.strip()) > 1:
                    key = f"autoKey{key_counter}"
                    new_translations[key] = text
                    key_counter += 1
            
            # 如果有新翻译，更新lang.js
            if new_translations:
                # 在CN对象中添加新翻译
                for key, value in new_translations.items():
                    new_entry = f'    "{key}": "{value}",\\n'
                    # 找到CN对象的结束位置并插入新条目
                    insert_pos = cn_section.rfind('}')
                    if insert_pos != -1:
                        cn_section = cn_section[:insert_pos] + new_entry + cn_section[insert_pos:]
                
                # 更新整个lang.js内容
                lang_content = lang_content.replace(cn_translations_match.group(0), cn_section)
                
                # 写回文件
                with open('data/lang.js', 'w', encoding='utf-8') as f:
                    f.write(lang_content)
                
                print(f"Added {len(new_translations)} new translations to lang.js")
        
    except Exception as e:
        print(f"Warning: Could not update lang.js - {e}")
    
    # 第五步：写回修复后的内容
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("Final comprehensive fix completed!")
    return fixed_count

if __name__ == "__main__":
    count = final_comprehensive_fix()
    print(f"Process completed. Fixed {count} issues in total.")