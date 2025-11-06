#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
将项目文件转换为UTF-8编码
"""

import os
import codecs
import chardet

def convert_file_to_utf8(file_path):
    """将单个文件转换为UTF-8编码"""
    try:
        # 检测文件编码
        with open(file_path, 'rb') as f:
            raw_data = f.read()
            
        # 检测编码
        detected = chardet.detect(raw_data)
        encoding = detected.get('encoding', 'utf-8')
        confidence = detected.get('confidence', 0)
        
        print(f"检测文件: {file_path}")
        print(f"  编码: {encoding} (置信度: {confidence:.2f})")
        
        # 如果已经是UTF-8且置信度高，则跳过
        if encoding.lower() in ['utf-8', 'ascii'] and confidence > 0.8:
            print(f"  已经是UTF-8编码，跳过")
            return
            
        # 读取文件内容
        try:
            with open(file_path, 'r', encoding=encoding) as f:
                content = f.read()
        except UnicodeDecodeError:
            # 如果检测的编码无法读取，尝试常见编码
            for enc in ['gbk', 'gb2312', 'latin-1', 'iso-8859-1']:
                try:
                    with open(file_path, 'r', encoding=enc) as f:
                        content = f.read()
                    encoding = enc
                    break
                except UnicodeDecodeError:
                    continue
            else:
                print(f"  无法解码文件，跳过")
                return
        
        # 写入UTF-8编码
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
            
        print(f"  已转换为UTF-8编码")
        
    except Exception as e:
        print(f"  转换失败: {e}")

def main():
    """主函数"""
    project_dir = os.path.dirname(os.path.abspath(__file__))
    
    # 需要转换的文件扩展名
    extensions = [
        '.html', '.md', '.py', '.ino', '.txt', '.css', '.js',
        '.bat', '.ps1', '.gradle', '.properties', '.xml', '.json'
    ]
    
    converted_count = 0
    
    for root, dirs, files in os.walk(project_dir):
        # 跳过一些不需要的目录
        skip_dirs = ['.git', '__pycache__', 'node_modules', '.gradle', 'build']
        dirs[:] = [d for d in dirs if d not in skip_dirs]
        
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                file_path = os.path.join(root, file)
                convert_file_to_utf8(file_path)
                converted_count += 1
    
    print(f"\n转换完成！共处理了 {converted_count} 个文件")

if __name__ == "__main__":
    main()