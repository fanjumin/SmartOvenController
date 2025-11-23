import shutil
import datetime

def remove_special_chars():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.special_char_backup_{timestamp}')
    
    # 移除特殊字符
    special_char = ''  # 这是我们要移除的特殊字符
    count = content.count(special_char)
    
    if count > 0:
        print(f"发现 {count} 个特殊字符，正在移除...")
        fixed_content = content.replace(special_char, '')  # 将其替换为空字符串
        
        # Write back to file
        with open('data/index.html', 'w', encoding='utf-8') as f:
            f.write(fixed_content)
        
        print("特殊字符已移除!")
    else:
        print("未发现特殊字符")

if __name__ == "__main__":
    remove_special_chars()