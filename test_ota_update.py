#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import requests
import time
import os

# 设备IP地址
DEVICE_IP = "192.168.16.100"
OTA_UPDATE_URL = f"http://{DEVICE_IP}/ota_update"
UPDATE_FIRMWARE_URL = f"http://{DEVICE_IP}/update"
UPDATE_FS_URL = f"http://{DEVICE_IP}/update"

def test_ota_page_access():
    """测试OTA更新页面是否可以访问"""
    print("正在测试OTA更新页面访问...")
    try:
        response = requests.get(OTA_UPDATE_URL, timeout=10)
        if response.status_code == 200:
            print(f"✓ OTA更新页面可以正常访问")
            print(f"  状态码: {response.status_code}")
            print(f"  内容长度: {len(response.content)} 字节")
            return True
        else:
            print(f"✗ 无法访问OTA更新页面，状态码: {response.status_code}")
            return False
    except Exception as e:
        print(f"✗ 访问OTA更新页面时出错: {e}")
        return False

def test_firmware_update_endpoint():
    """测试固件更新端点"""
    print("\n正在测试固件更新端点...")
    try:
        # 发送OPTIONS请求检查支持的方法
        response = requests.options(UPDATE_FIRMWARE_URL, timeout=10)
        print(f"✓ 固件更新端点可访问")
        print(f"  支持的HTTP方法: {response.headers.get('Allow', '未知')}")
        return True
    except Exception as e:
        print(f"✗ 测试固件更新端点时出错: {e}")
        return False

def test_filesystem_update_endpoint():
    """测试文件系统更新端点"""
    print("\n正在测试文件系统更新端点...")
    try:
        # 发送OPTIONS请求检查支持的方法
        response = requests.options(UPDATE_FS_URL, timeout=10)
        print(f"✓ 文件系统更新端点可访问")
        print(f"  支持的HTTP方法: {response.headers.get('Allow', '未知')}")
        return True
    except Exception as e:
        print(f"✗ 测试文件系统更新端点时出错: {e}")
        return False

def main():
    print("=" * 50)
    print("智能烤箱控制器 OTA 功能测试")
    print("=" * 50)
    
    # 测试OTA页面访问
    page_access = test_ota_page_access()
    
    if page_access:
        # 测试更新端点
        firmware_endpoint = test_firmware_update_endpoint()
        fs_endpoint = test_filesystem_update_endpoint()
        
        print("\n" + "=" * 50)
        if firmware_endpoint and fs_endpoint:
            print("✓ 所有OTA功能测试通过!")
            print("  设备已准备好接收OTA更新")
        else:
            print("⚠ 部分OTA功能可能存在问题")
        print("=" * 50)
    else:
        print("\n" + "=" * 50)
        print("✗ OTA功能测试失败!")
        print("  请检查设备连接和配置")
        print("=" * 50)

if __name__ == "__main__":
    main()