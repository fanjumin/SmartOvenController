#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
检查所有HTML文件的编码问题
"""

import os
import chardet

def check_html_files():
    # 获取所有HTML文件
    html_files = [f for f in os.listdir('.') if f.endswith('.html')]
    
    print("检查所有HTML文件的编码:")
    print("-" * 50)
    
    for html_file in html_files:
        try:
            # 读取文件前10KB内容来检测编码
            with open(html_file, 'rb') as f:
                raw_data = f.read(10240)
                result = chardet.detect(raw_data)
                encoding = result['encoding']
                confidence = result['confidence']
                
                print(f"{html_file}: {encoding} (置信度: {confidence:.2f})")
                
                # 如果置信度较低，进一步检查内容
                if confidence < 0.9:
                    print(f"  警告: {html_file} 编码检测置信度较低")
                    
        except Exception as e:
            print(f"{html_file}: 检查失败 - {str(e)}")
    
    print("-" * 50)

if __name__ == "__main__":
    check_html_files()