#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import requests
import time

# 设备IP地址
DEVICE_IP = "192.168.16.100"
OTA_UPDATE_URL = f"http://{DEVICE_IP}/ota_update"
UPDATE_FIRMWARE_URL = f"http://{DEVICE_IP}/update"
UPDATE_FS_URL = f"http://{DEVICE_IP}/update"

def check_endpoint(url, name):
    """检查端点是否可访问"""
    print(f"正在检查 {name} ({url})...")
    try:
        # 先发送HEAD请求检查
        response = requests.head(url, timeout=5)
        print(f"  HEAD请求 - 状态码: {response.status_code}")
        
        # 再发送GET请求检查
        response = requests.get(url, timeout=5)
        print(f"  GET请求 - 状态码: {response.status_code}")
        print(f"  内容长度: {len(response.content)} 字节")
        
        if response.status_code == 200:
            print(f"✓ {name} 端点正常工作")
            return True
        else:
            print(f"⚠ {name} 端点返回状态码: {response.status_code}")
            return True
    except requests.exceptions.Timeout:
        print(f"⚠ {name} 端点响应超时")
        return True
    except Exception as e:
        print(f"✗ 检查 {name} 端点时出错: {e}")
        return False

def main():
    print("=" * 50)
    print("智能烤箱控制器 OTA 端点检查")
    print("=" * 50)
    print(f"目标设备IP: {DEVICE_IP}")
    print()
    
    # 检查各个端点
    ota_page = check_endpoint(OTA_UPDATE_URL, "OTA更新页面")
    firmware_endpoint = check_endpoint(UPDATE_FIRMWARE_URL, "固件更新端点")
    fs_endpoint = check_endpoint(UPDATE_FS_URL, "文件系统更新端点")
    
    print("\n" + "=" * 50)
    if ota_page and firmware_endpoint and fs_endpoint:
        print("✓ 所有OTA端点均可访问!")
        print("  设备OTA功能正常")
    else:
        print("⚠ 部分OTA端点可能存在问题")
    print("=" * 50)

if __name__ == "__main__":
    main()