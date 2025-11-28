#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
智能烤箱文件系统OTA更新手动测试脚本
生成测试命令供手动执行
"""

import os
from pathlib import Path

def generate_test_commands(device_ip="192.168.16.103", fs_image_path=".pio/build/nodemcuv2/littlefs.bin"):
    """
    生成手动测试命令
    """

    print("🔧 智能烤箱文件系统OTA更新手动测试")
    print("=" * 50)

    # 检查文件是否存在
    if not os.path.exists(fs_image_path):
        print(f"❌ 错误：文件系统镜像文件不存在: {fs_image_path}")
        print("请先运行: pio run -e nodemcuv2 -t buildfs")
        return

    # 获取文件大小
    file_size = os.path.getsize(fs_image_path)
    print(f"✅ 文件系统镜像大小: {file_size} 字节")

    print("\n📋 请按以下步骤手动测试:")
    print()

    print("1. 检查设备状态:")
    print(f"   curl -X GET 'http://{device_ip}/status'")
    print()

    print("2. 检查当前index.html:")
    print(f"   curl -I 'http://{device_ip}/index.html'")
    print()

    print("3. 上传文件系统镜像:")
    print(f"   curl -X POST 'http://{device_ip}/filesystem_update' \\")
    print(f"        -F 'filesystem=@{fs_image_path}' \\")
    print("        -v")
    print()

    print("4. 等待10秒让设备重启...")
    print()

    print("5. 验证更新结果:")
    print(f"   curl -X GET 'http://{device_ip}/status'")
    print(f"   curl -I 'http://{device_ip}/index.html'")
    print()

    print("💡 提示:")
    print("- 如果上传成功，设备会返回JSON响应包含 'action': 'restart'")
    print("- 设备会在3秒后重启")
    print("- 重启后index.html应该可以正常访问")
    print("- 如果失败，请检查ESP8266的串口输出日志")

def main():
    device_ip = "192.168.16.103"
    fs_image_path = ".pio/build/nodemcuv2/littlefs.bin"

    generate_test_commands(device_ip, fs_image_path)

if __name__ == "__main__":
    main()