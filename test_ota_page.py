#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
OTA页面测试脚本
测试设备IP/ota_update页面的访问情况
"""

import requests
import sys
import time

def test_ota_page(device_ip):
    """
    测试OTA页面的访问
    """
    url = f"http://{device_ip}/ota_update"

    print(f"正在测试OTA页面: {url}")
    print("-" * 50)

    try:
        # 发送GET请求
        response = requests.get(url, timeout=10)

        print(f"HTTP状态码: {response.status_code}")

        if response.status_code == 200:
            print("✅ OTA页面访问成功")
            print(f"响应内容长度: {len(response.text)} 字符")

            # 检查是否包含OTA相关内容
            if "OTA升级" in response.text or "OTA update" in response.text:
                print("✅ 响应包含OTA页面内容")
            else:
                print("⚠️  响应不包含预期的OTA页面内容")

            # 显示响应前200个字符
            print("\n响应内容预览:")
            print("-" * 30)
            print(response.text[:200] + "..." if len(response.text) > 200 else response.text)

        else:
            print(f"❌ OTA页面访问失败，状态码: {response.status_code}")
            print(f"错误信息: {response.text}")

    except requests.exceptions.RequestException as e:
        print(f"❌ 网络请求失败: {e}")
        print("可能的原因:")
        print("1. 设备IP地址不正确")
        print("2. 设备未连接到网络")
        print("3. 设备Web服务器未启动")
        print("4. 防火墙或网络配置问题")

    except Exception as e:
        print(f"❌ 未知错误: {e}")

def main():
    if len(sys.argv) != 2:
        print("用法: python test_ota_page.py <设备IP地址>")
        print("例如: python test_ota_page.py 192.168.1.100")
        sys.exit(1)

    device_ip = sys.argv[1]

    # 验证IP地址格式
    try:
        parts = device_ip.split('.')
        if len(parts) != 4:
            raise ValueError
        for part in parts:
            num = int(part)
            if num < 0 or num > 255:
                raise ValueError
    except ValueError:
        print(f"❌ 无效的IP地址格式: {device_ip}")
        sys.exit(1)

    print("OTA页面测试工具")
    print("=" * 50)

    # 多次测试
    for i in range(3):
        if i > 0:
            print(f"\n等待2秒后重试... ({i+1}/3)")
            time.sleep(2)

        test_ota_page(device_ip)

if __name__ == "__main__":
    main()