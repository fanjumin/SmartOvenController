#!/usr/bin/env python3
"""
问题API端点详细诊断脚本
"""

import requests
import json
import time

def diagnose_savewifi():
    """诊断/savewifi端点问题"""
    print("诊断 /savewifi 端点...")
    
    # 测试不同的参数组合
    test_cases = [
        {"ssid": "test", "password": "test123"},
        {"ssid": "", "password": ""},  # 空参数
        {},  # 无参数
        {"ssid": "test"},  # 只有SSID
        {"password": "test123"}  # 只有密码
    ]
    
    for i, params in enumerate(test_cases):
        print(f"\n测试用例 {i+1}: {params}")
        try:
            response = requests.post(
                "http://192.168.16.104/savewifi",
                json=params,
                timeout=5
            )
            print(f"  状态码: {response.status_code}")
            print(f"  响应: {response.text[:200]}")
        except Exception as e:
            print(f"  错误: {e}")

def diagnose_factoryreset():
    """诊断/factoryreset端点问题"""
    print("\n诊断 /factoryreset 端点...")
    
    # 测试不同的超时时间
    timeouts = [5, 10, 15, 30]
    
    for timeout in timeouts:
        print(f"\n测试超时时间: {timeout}秒")
        try:
            start_time = time.time()
            response = requests.post(
                "http://192.168.16.104/factoryreset",
                timeout=timeout
            )
            end_time = time.time()
            print(f"  成功! 响应时间: {end_time - start_time:.2f}秒")
            print(f"  状态码: {response.status_code}")
            break
        except requests.exceptions.Timeout:
            print(f"  超时 ({timeout}秒)")
        except Exception as e:
            print(f"  错误: {e}")
            break

def check_device_status_after_reset():
    """检查设备在恢复出厂设置后的状态"""
    print("\n检查设备状态...")
    
    # 等待设备重启
    for i in range(10):
        try:
            response = requests.get("http://192.168.16.104/status", timeout=5)
            if response.status_code == 200:
                print(f"  设备已恢复在线 (尝试 {i+1})")
                data = response.json()
                print(f"  设备状态: {json.dumps(data, indent=2, ensure_ascii=False)}")
                return
        except:
            print(f"  等待设备恢复... (尝试 {i+1})")
            time.sleep(2)
    
    print("  设备未能在预期时间内恢复")

def main():
    print("开始详细诊断问题API端点")
    print("=" * 50)
    
    # 诊断/savewifi端点
    diagnose_savewifi()
    
    # 诊断/factoryreset端点
    diagnose_factoryreset()
    
    print("\n" + "=" * 50)
    print("诊断完成")

if __name__ == "__main__":
    main()