import shutil
import datetime
import re

def enhanced_clean():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.enhanced_clean_backup_{timestamp}')
    
    # 移除特殊字符的增强方法
    print("开始增强清理文件...")
    
    # 方法1: 直接移除已知的特殊字符
    special_chars = ['', '\x03', '\x00', '\x01', '\x02', '\x04', '\x05', '\x06', '\x07', '\x08']
    for char in special_chars:
        count = content.count(char)
        if count > 0:
            print(f"移除 {count} 个 '{repr(char)}' 字符")
            content = content.replace(char, '')
    
    # 方法2: 使用正则表达式移除控制字符
    # 保留正常的空白字符和可打印字符
    content = re.sub(r'[\x00-\x08\x0B\x0C\x0E-\x1F\x7F]', '', content)
    
    # 方法3: 清理多余的空白字符
    # 将多个连续的空白字符替换为单个空格
    content = re.sub(r'[ \t]{2,}', ' ', content)
    
    # 方法4: 移除文件末尾的多余字符
    content = content.rstrip()
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("增强文件清理完成!")

if __name__ == "__main__":
    enhanced_clean()