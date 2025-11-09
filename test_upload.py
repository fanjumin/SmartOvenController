#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
测试HTML文件上传脚本的基本功能
"""

import os
import sys

def test_basic():
    print("=== 测试HTML文件上传脚本 ===")
    
    # 检查data目录是否存在
    data_dir = 'data'
    if os.path.exists(data_dir):
        print(f"✓ 数据目录 '{data_dir}' 存在")
        
        # 检查HTML文件
        html_files = []
        for file in os.listdir(data_dir):
            if file.endswith(('.html', '.js', '.css')):
                html_files.append(file)
        
        if html_files:
            print(f"✓ 找到 {len(html_files)} 个HTML文件:")
            for file in html_files:
                print(f"  - {file}")
        else:
            print("✗ 未找到HTML文件")
            return False
    else:
        print(f"✗ 数据目录 '{data_dir}' 不存在")
        return False
    
    # 检查串口模块
    try:
        import serial
        print("✓ serial模块可用")
    except ImportError:
        print("✗ serial模块不可用")
        return False
    
    # 检查base64模块
    try:
        import base64
        print("✓ base64模块可用")
    except ImportError:
        print("✗ base64模块不可用")
        return False
    
    print("✓ 所有基本检查通过")
    return True

if __name__ == "__main__":
    test_basic()