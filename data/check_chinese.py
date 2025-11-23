import re
import glob

# 查找所有.js和.html文件
files = glob.glob('*.js') + glob.glob('*.html')

print("Checking for hardcoded Chinese characters...")
print("=" * 50)

total_matches = 0

for file in files:
    try:
        with open(file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找中文字符
        matches = re.findall(r'[\u4e00-\u9fff]+', content)
        
        if matches:
            print(f"{file}: {len(matches)} Chinese characters found")
            total_matches += len(matches)
        else:
            print(f"{file}: No Chinese characters found")
            
    except Exception as e:
        print(f"{file}: Error reading file - {e}")

print("=" * 50)
print(f"Total Chinese characters found: {total_matches}")