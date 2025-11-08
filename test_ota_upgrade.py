#!/usr/bin/env python3
"""
OTA升级功能测试脚本
测试固件和HTML文件的一键升级功能
"""

import requests
import json
import time
import os

def test_ota_upgrade():
    """测试OTA升级功能"""
    
    # 设备IP地址（需要根据实际情况修改）
    device_ip = "192.168.4.1"  # 默认AP模式IP
    
    print("=== OTA升级功能测试 ===")
    
    # 1. 测试设备状态
    print("\n1. 测试设备状态...")
    try:
        response = requests.get(f"http://{device_ip}/status", timeout=10)
        if response.status_code == 200:
            status_data = response.json()
            print(f"   设备状态: {status_data}")
            print(f"   固件版本: {status_data.get('firmware_version', '未知')}")
        else:
            print(f"   状态请求失败: {response.status_code}")
    except Exception as e:
        print(f"   设备连接失败: {e}")
        print("   请确保设备已启动并连接到正确网络")
        return
    
    # 2. 测试OTA升级页面
    print("\n2. 测试OTA升级页面...")
    try:
        response = requests.get(f"http://{device_ip}/ota_update", timeout=10)
        if response.status_code == 200:
            print("   OTA升级页面可访问")
            # 检查页面内容
            if "固件升级" in response.text and "HTML文件升级" in response.text:
                print("   升级界面功能完整")
            else:
                print("   警告: 升级界面可能不完整")
        else:
            print(f"   OTA页面请求失败: {response.status_code}")
    except Exception as e:
        print(f"   OTA页面访问失败: {e}")
    
    # 3. 测试固件升级端点
    print("\n3. 测试固件升级端点...")
    try:
        # 检查固件文件是否存在
        firmware_path = ".pio/build/nodemcuv2/firmware.bin"
        if os.path.exists(firmware_path):
            print(f"   固件文件存在: {firmware_path}")
            print(f"   文件大小: {os.path.getsize(firmware_path)} 字节")
            
            # 测试升级端点
            response = requests.get(f"http://{device_ip}/update", timeout=10)
            if response.status_code == 200:
                print("   固件升级端点可访问")
            else:
                print(f"   固件升级端点请求失败: {response.status_code}")
        else:
            print("   警告: 固件文件不存在，请先编译项目")
    except Exception as e:
        print(f"   固件升级测试失败: {e}")
    
    # 4. 测试文件上传端点
    print("\n4. 测试文件上传端点...")
    try:
        # 测试一个简单的HTML文件上传
        test_file = "data/index.html"
        if os.path.exists(test_file):
            print(f"   测试文件存在: {test_file}")
            
            with open(test_file, 'rb') as f:
                files = {'file': ('index.html', f, 'text/html')}
                response = requests.post(f"http://{device_ip}/upload", files=files, timeout=30)
                
                if response.status_code == 200:
                    print("   文件上传端点可访问")
                    print(f"   上传响应: {response.text}")
                else:
                    print(f"   文件上传请求失败: {response.status_code}")
        else:
            print("   警告: 测试文件不存在")
    except Exception as e:
        print(f"   文件上传测试失败: {e}")
    
    # 5. 测试统一升级功能
    print("\n5. 测试统一升级功能...")
    print("   统一升级功能需要在实际设备上测试")
    print("   功能包括:")
    print("   - 固件和HTML文件同时上传")
    print("   - 进度显示")
    print("   - 升级结果反馈")
    
    print("\n=== 测试完成 ===")
    print("\n使用说明:")
    print("1. 将设备连接到WiFi或使用AP模式")
    print("2. 访问 http://设备IP/ota_update")
    print("3. 选择固件文件(.bin)和HTML文件进行升级")
    print("4. 点击上传按钮开始升级")
    print("5. 等待升级完成并重启设备")

if __name__ == "__main__":
    test_ota_upgrade()