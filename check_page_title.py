#!/usr/bin/env python3
"""
页面标题验证脚本
验证设备页面标题是否正确
"""

import requests
import time

def check_page_title():
    """检查页面标题"""
    device_ip = "192.168.16.104"
    
    print("=== 页面标题验证 ===")
    
    # 测试主页访问
    print("\n1. 检查主页标题...")
    try:
        response = requests.get(f"http://{device_ip}/", timeout=10)
        if response.status_code == 200:
            print("   ✅ 主页可访问")
            print(f"   页面长度: {len(response.text)} 字符")
            
            # 检查页面标题
            if "<title>智能烤箱 - 管理员控制面板</title>" in response.text:
                print("   ✅ 页面标题正确")
            elif "智能烤箱" in response.text and "管理员控制面板" in response.text:
                print("   ✅ 页面标题包含正确内容")
            else:
                print("   ⚠️  页面标题可能不正确")
                # 显示页面前500个字符以供检查
                print(f"   页面前500字符: {response.text[:500]}")
        else:
            print(f"   ❌ 主页访问失败: {response.status_code}")
    except Exception as e:
        print(f"   ❌ 主页访问失败: {e}")
    
    # 测试登录页面标题
    print("\n2. 检查登录页面标题...")
    try:
        response = requests.get(f"http://{device_ip}/login.html", timeout=10)
        if response.status_code == 200:
            print("   ✅ 登录页面可访问")
            if "<title>智能烤箱 - 登录</title>" in response.text:
                print("   ✅ 登录页面标题正确")
            elif "智能烤箱" in response.text and "登录" in response.text:
                print("   ✅ 登录页面标题包含正确内容")
            else:
                print("   ⚠️  登录页面标题可能不正确")
        else:
            print(f"   ❌ 登录页面访问失败: {response.status_code}")
    except Exception as e:
        print(f"   ❌ 登录页面访问失败: {e}")
    
    print("\n=== 验证完成 ===")

if __name__ == "__main__":
    check_page_title()