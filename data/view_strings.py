import os

# 读取提取的中文字符串文件
file_path = os.path.join(os.path.dirname(__file__), 'chinese_strings.txt')

# 读取并显示前50行
with open(file_path, 'r', encoding='utf-8') as f:
    lines = f.readlines()
    
print("First 50 lines of chinese_strings.txt:")
print("=" * 50)
for i, line in enumerate(lines[:50]):
    print(f"{i+1:2d}: {line.rstrip()}")