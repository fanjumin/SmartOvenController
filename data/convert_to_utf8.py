import os
import chardet

def convert_to_utf8(file_path):
    # 首先检测文件编码
    with open(file_path, 'rb') as f:
        raw_data = f.read()
        result = chardet.detect(raw_data)
        detected_encoding = result['encoding']
        confidence = result['confidence']
    
    print(f"处理 {file_path}: 检测到编码 {detected_encoding} (置信度: {confidence:.2f})")
    
    # 读取文件内容（使用检测到的编码）
    try:
        with open(file_path, 'r', encoding=detected_encoding, errors='replace') as f:
            content = f.read()
        
        # 重新保存为UTF-8（无BOM）
        with open(file_path, 'w', encoding='utf-8', newline='') as f:
            f.write(content)
        
        print(f"✓ 已转换为 UTF-8")
        return True
    except Exception as e:
        print(f"✗ 转换失败: {str(e)}")
        return False

def main():
    # 检测并转换所有HTML和JS文件
    html_files = []
    js_files = []
    
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith('.html'):
                html_files.append(os.path.join(root, file))
            elif file.endswith('.js'):
                js_files.append(os.path.join(root, file))
    
    print("开始转换HTML文件...")
    print("-" * 60)
    success_count = 0
    fail_count = 0
    
    for file in html_files:
        if convert_to_utf8(file):
            success_count += 1
        else:
            fail_count += 1
    
    print("\n开始转换JS文件...")
    print("-" * 60)
    for file in js_files:
        if convert_to_utf8(file):
            success_count += 1
        else:
            fail_count += 1
    
    print(f"\n转换完成:")
    print(f"成功: {success_count} 个文件")
    print(f"失败: {fail_count} 个文件")

if __name__ == "__main__":
    main()