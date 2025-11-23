import chardet

# 检查index.html文件编码
import sys

import os
import glob

if len(sys.argv) != 2:
    print("Usage:")
    print("  Single file: python check_encoding.py <file_path>")
    print("  Directory:   python check_encoding.py <directory_path>")
    sys.exit(1)

path = sys.argv[1]

if os.path.isdir(path):
    # 查找目录下所有HTML文件
    html_files = glob.glob(os.path.join(path, '**', '*.html'), recursive=True)
    if not html_files:
        print(f"No HTML files found in directory: {path}")
        sys.exit(0)
else:
    html_files = [path]
for file_path in html_files:
    with open(file_path, 'rb') as f:
        raw_data = f.read()
        encoding_info = chardet.detect(raw_data)
        print(f"文件: {file_path}")
        print(f"检测到的编码: {encoding_info['encoding']}")
        print(f"可信度: {encoding_info['confidence']:.2f}")