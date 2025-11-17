#!/usr/bin/env python3
"""
OTA功能修复验证脚本
测试修复后的OTA上传功能
"""

import requests
import time
import json

def test_ota_endpoints():
    """测试OTA相关端点"""
    base_url = "http://192.168.16.105"
    
    print("=== OTA功能修复验证测试 ===")
    
    # 1. 测试OTA页面访问
    print("\n1. 测试OTA页面访问...")
    try:
        response = requests.get(f"{base_url}/ota_update", timeout=10)
        print(f"   OTA页面状态码: {response.status_code}")
        if response.status_code == 200:
            print("   ✓ OTA页面可正常访问")
        else:
            print("   ✗ OTA页面访问失败")
    except Exception as e:
        print(f"   ✗ OTA页面访问异常: {e}")
    
    # 2. 测试设备状态端点
    print("\n2. 测试设备状态端点...")
    try:
        response = requests.get(f"{base_url}/status", timeout=10)
        print(f"   状态端点状态码: {response.status_code}")
        if response.status_code == 200:
            status_data = response.json()
            print(f"   设备ID: {status_data.get('device_id', 'N/A')}")
            print(f"   固件版本: {status_data.get('firmware_version', 'N/A')}")
            print("   ✓ 设备状态端点正常")
        else:
            print("   ✗ 设备状态端点异常")
    except Exception as e:
        print(f"   ✗ 设备状态端点异常: {e}")
    
    # 3. 测试重启端点
    print("\n3. 测试重启端点...")
    try:
        response = requests.post(f"{base_url}/restart", timeout=10)
        print(f"   重启端点状态码: {response.status_code}")
        if response.status_code == 200:
            print("   ✓ 重启端点正常")
        else:
            print("   ✗ 重启端点异常")
    except Exception as e:
        print(f"   ✗ 重启端点异常: {e}")
    
    # 4. 测试OTA上传端点配置
    print("\n4. 测试OTA上传端点配置...")
    endpoints = ["/update"]
    for endpoint in endpoints:
        try:
            # 发送空的POST请求测试端点是否存在
            response = requests.post(f"{base_url}{endpoint}", timeout=10)
            print(f"   {endpoint} 状态码: {response.status_code}")
            if response.status_code != 404:
                print(f"   ✓ {endpoint} 端点配置正常")
            else:
                print(f"   ✗ {endpoint} 端点返回404")
        except Exception as e:
            print(f"   ✗ {endpoint} 端点测试异常: {e}")
    
    print("\n=== 测试完成 ===")
    print("\n修复说明:")
    print("1. 修复了端点配置冲突问题")
    print("2. 修复了SSE响应格式问题")
    print("3. 修复了文件系统更新逻辑错误")
    print("4. 优化了文件上传处理流程")
    print("\n建议通过浏览器访问 http://192.168.16.105/ota_update 进行实际文件上传测试")

if __name__ == "__main__":
    test_ota_endpoints()