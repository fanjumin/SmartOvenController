import shutil
import datetime
import re

def advanced_garbled_fix():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.advanced_fix_backup_{timestamp}')
    
    # 使用正则表达式查找和移除乱码字符模式
    # 移除常见的乱码字符序列
    fixed_content = content
    
    # 移除以特定模式出现的乱码
    # 基于我们之前的发现，移除特定的乱码字符
    fixed_content = fixed_content.replace('', '')
    
    # 使用正则表达式移除其他可能的乱码模式
    # 移除连续的非打印字符
    fixed_content = re.sub(r'[\x00-\x08\x0B\x0C\x0E-\x1F\x7F-\x9F]+', '', fixed_content)
    
    # 再次检查并移除特定的乱码字符
    garbled_patterns = [
        r'Ã[^ ]{2,}',
        r'Â[^ ]',
        r'[¤§°¯½¼¾£¢¥©®µ²³¹]+'
    ]
    
    for pattern in garbled_patterns:
        fixed_content = re.sub(pattern, '', fixed_content)
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(fixed_content)
    
    print("Advanced garbled fix applied!")

if __name__ == "__main__":
    advanced_garbled_fix()