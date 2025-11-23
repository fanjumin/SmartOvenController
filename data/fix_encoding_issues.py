#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
修复index.html文件中的编码问题
"""

import re

def fix_encoding_issues():
    # 读取文件
    with open('index.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 定义常见的乱码映射
    fixes = {
        'å\x85³é\x97\xadå\x8a\xa0ç\x83\xad': '关闭加热',
        'å¼\x80å\x90¯å\x8a\xa0ç\x83\xad': '开启加热'
    }
    
    # 应用修复
    for broken, fixed in fixes.items():
        content = content.replace(broken, fixed)
    
    # 写回文件
    with open('index.html', 'w', encoding='utf-8') as f:
        f.write(content)
    
    print("编码问题修复完成!")

if __name__ == "__main__":
    fix_encoding_issues()