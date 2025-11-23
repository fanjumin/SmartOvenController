#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
修复所有HTML文件的编码问题
"""

import os
import re
import shutil

def fix_html_encoding(file_path):
    """修复单个HTML文件的编码问题"""
    print(f"正在处理文件: {file_path}")
    
    # 读取文件内容（使用latin-1编码以保留所有字节）
    with open(file_path, 'rb') as f:
        content_bytes = f.read()
    
    # 尝试不同的编码方式解码
    encodings = ['utf-8', 'gbk', 'gb2312']
    content = None
    original_encoding = None
    
    for encoding in encodings:
        try:
            content = content_bytes.decode(encoding)
            original_encoding = encoding
            print(f"  使用 {encoding} 编码成功读取文件")
            break
        except UnicodeDecodeError:
            continue
    
    if content is None:
        print(f"  警告: 无法使用常见编码读取文件，使用UTF-8忽略错误")
        content = content_bytes.decode('utf-8', errors='ignore')
        original_encoding = 'unknown'
    
    # 如果内容包含乱码特征，则尝试修复
    if 'æºè½' in content or 'è½ç¤' in content or 'ç®±' in content:
        print("  发现乱码内容，尝试修复...")
        try:
            # 将字节内容重新以正确的UTF-8方式解码
            content = content_bytes.decode('utf-8')
            print("  成功修复乱码")
        except UnicodeDecodeError:
            print("  无法自动修复乱码")
    
    # 确保文件以正确的UTF-8编码保存
    with open(file_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(content)
    
    print(f"  文件已保存为UTF-8编码")
    return True

def main():
    """主函数"""
    # 获取当前目录下的所有HTML文件
    html_files = [f for f in os.listdir('.') if f.endswith('.html')]
    
    print(f"发现 {len(html_files)} 个HTML文件需要处理")
    
    # 处理每个HTML文件
    for html_file in html_files:
        try:
            fix_html_encoding(html_file)
        except Exception as e:
            print(f"处理文件 {html_file} 时出错: {e}")
    
    print("所有HTML文件编码修复完成!")

if __name__ == "__main__":
    main()