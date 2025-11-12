#!/usr/bin/env python3
"""
登录页面内容检查脚本
检查设备登录页面的内容和结构
"""

import requests
import time

def check_loginpage_content():
    """检查登录页面内容"""
    device_ip = "192.168.16.104"
    
    print("=== 登录页面内容检查 ===")
    
    # 测试登录页面访问
    print("\n1. 获取登录页面内容...")
    try:
        response = requests.get(f"http://{device_ip}/login.html", timeout=10)
        response.encoding = 'utf-8'  # 设置正确的编码
        if response.status_code == 200:
            print("   ✅ 登录页面可访问")
            print(f"   页面长度: {len(response.text)} 字符")
            
            # 显示前几行内容
            lines = response.text.split('\n')
            print("   登录页面前15行内容:")
            for i, line in enumerate(lines[:15]):
                print(f"   {i+1:2d}: {line}")
            
            # 检查关键元素
            if '<title>智能电烤箱 - 管理员登录</title>' in response.text:
                print("   ✅ 页面标题正确")
            else:
                print("   ⚠️  页面标题与预期略有不同")
                # 查找实际标题
                start = response.text.find("<title>")
                end = response.text.find("</title>")
                if start != -1 and end != -1:
                    title = response.text[start+7:end]
                    print(f"   实际标题: {title}")
                
            if '管理员密码' in response.text:
                print("   ✅ 包含密码输入字段")
            else:
                print("   ❌ 缺少密码输入字段")
                
            if 'loginForm' in response.text:
                print("   ✅ 包含登录表单")
            else:
                print("   ❌ 缺少登录表单")
                
        else:
            print(f"   ❌ 登录页面访问失败: {response.status_code}")
    except Exception as e:
        print(f"   ❌ 登录页面访问失败: {e}")
    
    print("\n=== 检查完成 ===")

if __name__ == "__main__":
    check_loginpage_content()