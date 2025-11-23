import re
import shutil
from datetime import datetime

def final_cleanup():
    """最终清理脚本"""
    print("Starting final cleanup...")
    
    # 备份原文件
    backup_name = f"data/index.html.final_cleanup_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    shutil.copy2('data/index.html', backup_name)
    print(f"Backup created: {backup_name}")
    
    # 读取文件
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 修复剩余的乱码字符
    # 替换一些特定的乱码模式
    fixes = {
        '误¤Â误Â¶误Â': '备',
        '误Â误Â误Â': '置',
        '误Â误Â': '设',
        '误Â': '备',
        '¤Â': '置',
        '¶误': '备',
        'Ã': '',
        'Â': '',
        '¤': '',
        '¶': '',
        '§': '',
        '¨': '',
        '©': '',
        'ª': '',
        '«': '',
        '¬': '',
        '­': '',
        '®': '',
        '¯': '',
        '°': '',
        '±': '',
        '²': '',
        '³': '',
        '´': '',
        'µ': '',
        '¶': '',
        '·': '',
        '¸': '',
        '¹': '',
        'º': '',
        '»': '',
        '¼': '',
        '½': '',
        '¾': '',
        '¿': '',
        'ƒ': '',
        '„': '',
        '…': '',
        '†': '',
        '‡': '',
        'ˆ': '',
        '‰': '',
        'Š': '',
        '‹': '',
        'Œ': '',
        '': '',
        'Ž': '',
        '': '',
        '': '',
        '‘': '',
        '’': '',
        '“': '',
        '”': '',
        '•': '',
        '–': '',
        '—': '',
        '˜': '',
        '™': '',
        'š': '',
        '›': '',
        'œ': '',
        '': '',
        'ž': '',
        'Ÿ': ''
    }
    
    # 应用修复
    fixed_count = 0
    for garbled, fixed in fixes.items():
        count = content.count(garbled)
        if count > 0:
            if fixed == '':
                # 如果替换为空字符串，直接移除
                content = content.replace(garbled, fixed)
                print(f"Removed '{garbled}' ({count} occurrences)")
            else:
                # 替换为正确的字符
                content = content.replace(garbled, fixed)
                print(f"Replaced '{garbled}' with '{fixed}' ({count} occurrences)")
            fixed_count += count
    
    # 移除连续的特殊字符模式
    patterns = [
        r'[ÃÂ¤Â¶Â§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ]{3,}'
    ]
    
    for pattern in patterns:
        matches = re.findall(pattern, content)
        for match in matches:
            content = content.replace(match, '')
            fixed_count += 1
            print(f"Removed complex pattern: {match}")
    
    # 写回文件
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Final cleanup completed. Fixed {fixed_count} issues.")
    return fixed_count

if __name__ == "__main__":
    count = final_cleanup()
    print(f"Process completed. Fixed {count} issues in total.")