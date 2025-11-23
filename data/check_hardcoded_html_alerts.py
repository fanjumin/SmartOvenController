import re
import glob

# 查找所有.html文件
html_files = glob.glob('*.html')

print("Checking for hardcoded Chinese alerts in HTML files...")
print("=" * 60)

total_matches = 0

for file in html_files:
    try:
        with open(file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找包含中文且不使用getTranslation的alert调用
        matches = re.findall(r'alert\([\'"][^\'"]*[\u4e00-\u9fff]+[^\'"]*[\'"]\)', content)
        # 过滤掉那些正确使用getTranslation的调用
        filtered_matches = []
        for match in matches:
            if 'getTranslation' not in match:
                filtered_matches.append(match)
        
        if filtered_matches:
            print(f"{file}: {len(filtered_matches)} hardcoded Chinese alerts found")
            for match in filtered_matches:
                print(f"  - {match}")
            total_matches += len(filtered_matches)
        else:
            print(f"{file}: No hardcoded Chinese alerts found")
            
    except Exception as e:
        print(f"{file}: Error reading file - {e}")

print("=" * 60)
print(f"Total hardcoded Chinese alerts found: {total_matches}")