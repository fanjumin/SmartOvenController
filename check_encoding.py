#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
检查文件编码
"""

import os
import chardet

def check_file_encoding(file_path):
    """检查单个文件的编码"""
    try:
        with open(file_path, 'rb') as f:
            raw_data = f.read()
        detected = chardet.detect(raw_data)
        encoding = detected.get('encoding', 'unknown')
        confidence = detected.get('confidence', 0)
        return encoding, confidence
    except Exception as e:
        return f"error: {e}", 0

def main():
    """主函数"""
    files_to_check = [
        'index.html',
        'login.html', 
        'mobile_home.html',
        'src/SmartOvenController.ino',
        'VERSION.md',
        'README.md'
    ]
    
    print("检查文件编码:")
    print("=" * 50)
    
    for file in files_to_check:
        if os.path.exists(file):
            encoding, confidence = check_file_encoding(file)
            print(f"{file}: {encoding} (置信度: {confidence:.2f})")
        else:
            print(f"{file}: 文件不存在")

if __name__ == "__main__":
    main()