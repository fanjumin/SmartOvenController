import re
import glob

# 查找所有.js文件
js_files = glob.glob('*.js')

print("Checking for hardcoded Chinese alerts in JavaScript files...")
print("=" * 60)

total_matches = 0

for file in js_files:
    try:
        with open(file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找包含中文的alert调用
        matches = re.findall(r'alert\([^)]*[\u4e00-\u9fff]+[^)]*\)', content)
        
        if matches:
            print(f"{file}: {len(matches)} hardcoded Chinese alerts found")
            for match in matches:
                print(f"  - {match}")
            total_matches += len(matches)
        else:
            print(f"{file}: No hardcoded Chinese alerts found")
            
    except Exception as e:
        print(f"{file}: Error reading file - {e}")

print("=" * 60)
print(f"Total hardcoded Chinese alerts found: {total_matches}")