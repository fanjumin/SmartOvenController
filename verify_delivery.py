#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
智能烤箱控制器交付包验证脚本
用于验证所有必要的文件是否已正确生成
"""

import os
import sys
from pathlib import Path

def check_file_exists(filepath, min_size=0):
    """检查文件是否存在且大小符合要求"""
    path = Path(filepath)
    if not path.exists():
        print(f"❌ 文件不存在: {filepath}")
        return False
    
    size = path.stat().st_size
    if size < min_size:
        print(f"❌ 文件大小异常: {filepath} ({size} bytes)")
        return False
    
    print(f"✅ 文件验证通过: {filepath} ({size} bytes)")
    return True

def main():
    print("=" * 50)
    print("智能烤箱控制器交付包验证")
    print("=" * 50)
    
    # 检查根目录文件
    root_files = [
        ("SmartOvenController_v0.8.1.zip", 374017),
        ("FINAL_PROJECT_SUMMARY.md", 1000),
        ("PROJECT_STATUS_REPORT.md", 1000),
        ("QUICK_START_GUIDE.md", 1000),
        ("DELIVERY_README.md", 1000),
    ]
    
    print("\n[1/3] 验证根目录文件...")
    for filename, min_size in root_files:
        if not check_file_exists(filename, min_size):
            return False
    
    # 检查release目录文件
    release_files = [
        ("release/firmware.bin", 300000),
        ("release/littlefs.bin", 1000000),
        ("platformio.ini", 500),
        ("BUILD_INSTRUCTIONS.md", 1000),
    ]
    
    print("\n[2/3] 验证release目录文件...")
    for filepath, min_size in release_files:
        if not check_file_exists(filepath, min_size):
            return False
    
    # 检查源代码目录
    src_files = [
        ("src/SmartOvenController.ino", 50000),
    ]
    
    print("\n[3/3] 验证源代码文件...")
    for filepath, min_size in src_files:
        if not check_file_exists(filepath, min_size):
            return False
    
    # 检查data目录
    data_files = [
        "data/index.html",
        "data/login.html",
        "data/device_status.html",
    ]
    
    print("\n[4/4] 验证Web界面文件...")
    for filepath in data_files:
        if not check_file_exists(filepath, 1000):
            return False
    
    print("\n" + "=" * 50)
    print("🎉 所有文件验证通过！")
    print("智能烤箱控制器交付包已准备就绪。")
    print("=" * 50)
    return True

if __name__ == "__main__":
    try:
        success = main()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"❌ 验证过程中发生错误: {e}")
        sys.exit(1)