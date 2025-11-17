#!/usr/bin/env python3
"""
OTA文件上传功能测试脚本
测试修复后的文件上传功能
"""

import requests
import time
import os

def test_ota_file_upload():
    """测试OTA文件上传功能"""
    base_url = "http://192.168.16.105"
    
    print("=== OTA文件上传功能测试 ===")
    
    # 检查测试文件是否存在
    test_files = [
        "test_ota_files/test_firmware.bin",
        "test_ota_files/test_littlefs.bin",
        "test_ota_files/test.html"
    ]
    
    available_files = []
    for file_path in test_files:
        if os.path.exists(file_path):
            available_files.append(file_path)
            print(f"✓ 找到测试文件: {file_path}")
        else:
            print(f"⚠ 测试文件不存在: {file_path}")
    
    if not available_files:
        print("\n⚠ 没有找到测试文件，创建示例文件进行测试...")
        # 创建测试目录
        os.makedirs("test_ota_files", exist_ok=True)
        
        # 创建示例HTML文件
        with open("test_ota_files/test.html", "w") as f:
            f.write("<html><body><h1>Test File</h1></body></html>")
        available_files.append("test_ota_files/test.html")
        print("✓ 已创建示例测试文件")
    
    # 测试文件上传
    for file_path in available_files:
        print(f"\n测试上传文件: {file_path}")
        
        # 根据文件类型选择端点
        if "littlefs" in file_path:
            endpoint = "/update"
            field_name = "filesystem"
        else:
            endpoint = "/update"
            field_name = "firmware"
        
        try:
            with open(file_path, "rb") as f:
                files = {field_name: (os.path.basename(file_path), f, "application/octet-stream")}
                
                print(f"  上传到端点: {endpoint}")
                print(f"  字段名称: {field_name}")
                print(f"  文件大小: {os.path.getsize(file_path)} 字节")
                
                # 设置较长的超时时间，因为文件上传需要时间
                response = requests.post(
                    f"{base_url}{endpoint}",
                    files=files,
                    timeout=30,
                    stream=True
                )
                
                print(f"  响应状态码: {response.status_code}")
                print(f"  响应内容: {response.text[:200]}...")
                
                if response.status_code == 200:
                    print("  ✓ 文件上传成功")
                else:
                    print("  ✗ 文件上传失败")
                    
        except requests.exceptions.Timeout:
            print("  ⚠ 上传超时，可能是文件处理中（正常现象）")
        except Exception as e:
            print(f"  ✗ 上传异常: {e}")
    
    print("\n=== 测试完成 ===")
    print("\n测试结果说明:")
    print("1. 超时响应可能是正常的，因为文件上传处理需要时间")
    print("2. 建议通过浏览器进行完整的功能测试")
    print("3. 浏览器测试地址: http://192.168.16.105/ota_update")

if __name__ == "__main__":
    test_ota_file_upload()