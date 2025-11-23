#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
修复HTML文件的编码问题
"""

import os
import chardet
import re

def fix_file_encoding(filename):
    """修复单个文件的编码"""
    try:
        # 检测文件编码
        with open(filename, 'rb') as f:
            raw_data = f.read(10240)
            result = chardet.detect(raw_data)
            encoding = result['encoding']
            confidence = result['confidence']
            
        print(f'检查 {filename}: {encoding} (置信度: {confidence:.2f})')
        
        # 如果编码置信度较低或不是UTF-8，尝试重新编码
        # 降低置信度阈值并扩大检测范围
        if confidence < 0.8 or encoding.lower() not in ['utf-8', 'ascii', 'utf-8-sig']:
            print(f'  修复 {filename} 的编码问题...')
            
            # 以检测到的编码读取文件
            with open(filename, 'r', encoding=encoding, errors='replace') as f:
                content = f.read()
            
            # 确保HTML文件包含正确的meta charset标签
            # 处理HTML和JavaScript文件
            if filename.lower().endswith(('.html', '.htm', '.js')):
                # 检查是否已有meta charset标签
                # 处理两种meta标签格式: <meta charset> 和 <meta http-equiv="Content-Type">
                meta_charset_pattern = re.compile(r'<meta\s+charset=["\']?([^"\'>]+)["\']?', re.IGNORECASE)
                meta_http_equiv_pattern = re.compile(r'<meta\s+http-equiv=["\']?Content-Type["\']?\s+content=["\']?text/html;\s*charset=([^"\'>]+)["\']?', re.IGNORECASE)
                # 确保<head>中包含meta charset标签
                head_pattern = re.compile(r'<head>', re.IGNORECASE)
                if not meta_tag_pattern.search(content):
                    if head_pattern.search(content):
                        content = head_pattern.sub('<head>\n    <meta charset="UTF-8">', content)
                        print(f'  添加meta charset标签到<head>中')
                    else:
                        # 如果没有<head>标签，在<html>后添加
                        html_pattern = re.compile(r'<html>', re.IGNORECASE)
                        if html_pattern.search(content):
                            content = html_pattern.sub('<html>\n<head>\n    <meta charset="UTF-8">\n</head>', content)
                            print(f'  添加<head>和meta charset标签')
                        else:
                            # 在文件开头添加
                            content = '<head>\n    <meta charset="UTF-8">\n</head>\n' + content
                            print(f'  添加<head>和meta charset标签到文件开头')
            
            print(f'  {filename} 编码已修复为 UTF-8')
        else:
            print(f'  {filename} 编码正常')
            
    except Exception as e:
        print(f'处理文件 {filename} 时出错: {e}')

def main():
    # 获取所有HTML文件（包括子目录）
    html_files = []
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.lower().endswith('.html'):
                html_files.append(os.path.join(root, file))
    
    print("修复HTML文件编码问题:")
    print("-" * 50)
    
    for html_file in html_files:
        fix_file_encoding(html_file)
    
    print("-" * 50)
    print("修复完成。")

if __name__ == "__main__":
    main()