import shutil
import datetime

def nbsp_fix():
    # Read the HTML file
    with open('data/index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    shutil.copy2('data/index.html', f'data/index.html.nbsp_fix_backup_{timestamp}')
    
    # 修复不间断空格问题
    # 根据上下文分析，这些不间断空格应该替换为正确的中文字符
    fixed_content = content
    
    # 替换特定上下文中的不间断空格
    # 根据我们之前的分析，这些\xa0应该替换为正确的中文字符
    replacements = {
        'data-lang="\xa0备': 'data-lang="设备"',
        '// \xa0备': '// 设备',
        '// \xa0': '// ',
        "s.heatingEnabled ? '\xa0' : '": "s.heatingEnabled ? ' ' : '",
        "tatus.textContent = '\xa0'": "tatus.textContent = ' '",  # 注意这里可能是在"status"中间有截断
    }
    
    for old, new in replacements.items():
        fixed_content = fixed_content.replace(old, new)
    
    # 一般性替换：将剩余的不间断空格替换为普通空格
    fixed_content = fixed_content.replace('\xa0', ' ')
    
    # Write back to file
    with open('data/index.html', 'w', encoding='utf-8') as f:
        f.write(fixed_content)
    
    print("NBSP fix applied!")
    print("修复了不间断空格导致的乱码问题")

if __name__ == "__main__":
    nbsp_fix()