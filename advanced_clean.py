import shutil
import datetime
import re

def advanced_clean():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.advanced_clean_backup_{timestamp}')
    
    # 移除特殊字符的多种方法
    print("开始清理文件...")
    
    # 方法1: 直接移除已知的特殊字符
    special_chars = ['', '\x03', '\x00']
    for char in special_chars:
        count = content.count(char)
        if count > 0:
            print(f"移除 {count} 个 '{repr(char)}' 字符")
            content = content.replace(char, '')
    
    # 方法2: 移除控制字符 (除了常见的换行符、制表符等)
    cleaned_content = []
    for char in content:
        # 保留正常的字符，移除控制字符
        if ord(char) >= 32 or char in '\n\r\t':
            cleaned_content.append(char)
        elif char in '\x09\x0a\x0d':  # 保留制表符、换行符、回车符
            cleaned_content.append(char)
    
    content = ''.join(cleaned_content)
    
    # 方法3: 清理多余的空白字符
    # 将多个连续的空白字符替换为单个空格
    content = re.sub(r'[ \t]{2,}', ' ', content)
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("文件清理完成!")

if __name__ == "__main__":
    advanced_clean()