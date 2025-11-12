#!/usr/bin/env python3
"""
设备主页内容检查脚本
检查设备主页的内容和结构
"""

import requests
import time

def check_homepage_content():
    """检查主页内容"""
    device_ip = "192.168.16.104"
    
    print("=== 设备主页内容检查 ===")
    
    # 测试主页访问
    print("\n1. 获取主页内容...")
    try:
        response = requests.get(f"http://{device_ip}/", timeout=10)
        response.encoding = 'utf-8'  # 设置正确的编码
        if response.status_code == 200:
            print("   ✅ 主页可访问")
            print(f"   页面长度: {len(response.text)} 字符")
            
            # 显示前几行内容
            lines = response.text.split('\n')
            print("   主页前15行内容:")
            for i, line in enumerate(lines[:15]):
                print(f"   {i+1:2d}: {line}")
            
            # 检查关键元素
            if '<title>智能烤箱 - 管理员控制面板</title>' in response.text:
                print("   ✅ 页面标题正确")
            else:
                print("   ❌ 页面标题不正确")
                
            if '智能烤箱' in response.text:
                print("   ✅ 包含'智能烤箱'标识")
            else:
                print("   ❌ 缺少'智能烤箱'标识")
                
            if 'logout()' in response.text:
                print("   ✅ 包含登出功能")
            else:
                print("   ❌ 缺少登出功能")
                
        else:
            print(f"   ❌ 主页访问失败: {response.status_code}")
    except Exception as e:
        print(f"   ❌ 主页访问失败: {e}")
    
    print("\n=== 检查完成 ===")

if __name__ == "__main__":
    check_homepage_content()