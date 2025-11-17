#!/usr/bin/env python3
"""
设备状态测试脚本
测试设备是否正常工作
"""

import requests
import time

def test_device_status():
    """测试设备状态"""
    device_ip = "192.168.16.100"
    
    print("=== 设备状态测试 ===")
    
    # 测试设备是否在线
    print("\n1. 测试设备是否在线...")
    try:
        response = requests.get(f"http://{device_ip}/status", timeout=10)
        if response.status_code == 200:
            print("   ✅ 设备在线")
            print(f"   状态信息: {response.text}")
        else:
            print(f"   ❌ 设备响应错误: {response.status_code}")
    except Exception as e:
        print(f"   ❌ 设备离线: {e}")
    
    # 测试主页访问
    print("\n2. 测试主页访问...")
    try:
        response = requests.get(f"http://{device_ip}/", timeout=10)
        if response.status_code == 200:
            print("   ✅ 主页可访问")
            if "智能烤箱" in response.text:
                print("   ✅ 页面标题正确")
            else:
                print("   ⚠️  页面标题可能不正确")
        else:
            print(f"   ❌ 主页访问失败: {response.status_code}")
    except Exception as e:
        print(f"   ❌ 主页访问失败: {e}")
    
    # 测试OTA升级页面
    print("\n3. 测试OTA升级页面...")
    try:
        response = requests.get(f"http://{device_ip}/ota_update", timeout=10)
        if response.status_code == 200:
            print("   ✅ OTA升级页面可访问")
        else:
            print(f"   ❌ OTA升级页面访问失败: {response.status_code}")
    except Exception as e:
        print(f"   ❌ OTA升级页面访问失败: {e}")
    
    print("\n=== 测试完成 ===")

if __name__ == "__main__":
    test_device_status()