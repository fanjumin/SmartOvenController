import os
import chardet

def check_file_encoding(file_path):
    with open(file_path, 'rb') as f:
        raw_data = f.read()
        result = chardet.detect(raw_data)
        encoding = result['encoding']
        confidence = result['confidence']
        
        # 检查是否有BOM
        has_bom = False
        if raw_data.startswith(b'\xef\xbb\xbf'):
            has_bom = True
            
        return encoding, confidence, has_bom

def main():
    html_files = []
    js_files = []
    
    # 收集所有HTML和JS文件
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith('.html'):
                html_files.append(os.path.join(root, file))
            elif file.endswith('.js'):
                js_files.append(os.path.join(root, file))
    
    print("HTML文件编码检查:")
    print("-" * 60)
    for file in html_files:
        try:
            encoding, confidence, has_bom = check_file_encoding(file)
            print(f"{file}: {encoding} (置信度: {confidence:.2f}) {'[含BOM]' if has_bom else ''}")
        except Exception as e:
            print(f"{file}: 无法检查编码 - {str(e)}")
    
    print("\nJS文件编码检查:")
    print("-" * 60)
    for file in js_files:
        try:
            encoding, confidence, has_bom = check_file_encoding(file)
            print(f"{file}: {encoding} (置信度: {confidence:.2f}) {'[含BOM]' if has_bom else ''}")
        except Exception as e:
            print(f"{file}: 无法检查编码 - {str(e)}")
    
    # 特别检查index.html和lang.js
    print("\n重点文件内容预览:")
    print("-" * 60)
    for file in ['index.html', 'lang.js']:
        if os.path.exists(file):
            print(f"\n{file} 内容预览:")
            with open(file, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read(200)
                print(content)

if __name__ == "__main__":
    main()