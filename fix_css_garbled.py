import re
import shutil
from datetime import datetime

# CSS中的乱码字符映射
css_garbled_map = {
    'åŠŸèƒ½åŒºåŸŸå“åº”å¼ä¼˜åŒ–': '功能区域响应式优化',
    'è®¾å¤‡çŠ¶æ€': '设备状态',
    'è®¾ç½®': '设置',
    'ç½‘ç»œè®¾ç½®': '网络设置',
    'ç³»ç»Ÿä¿¡æ¯': '系统信息',
    'å¸®åŠ©ä¸Žæ”¯æŒ': '帮助与支持',
    'å…³äºŽæˆ‘ä»¬': '关于我们'
}

def fix_css_garbled():
    """修复CSS中的乱码字符"""
    # 备份原文件
    backup_name = f"data/index.html.css_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # 读取文件
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 计算修复前的乱码数量
    initial_garbled_count = 0
    for pattern in css_garbled_map:
        initial_garbled_count += content.count(pattern)
    
    print(f"Initial CSS garbled patterns found: {initial_garbled_count}")
    
    # 修复乱码
    fixed_count = 0
    for garbled, fixed in css_garbled_map.items():
        count = content.count(garbled)
        if count > 0:
            content = content.replace(garbled, fixed)
            fixed_count += count
            print(f"Replaced '{garbled}' with '{fixed}' ({count} occurrences)")
    
    # 写回文件
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Fixed {fixed_count} CSS garbled patterns")
    return fixed_count

if __name__ == "__main__":
    count = fix_css_garbled()
    print(f"Process completed. Fixed {count} CSS garbled patterns.")