#!/usr/bin/env python3
"""
简化的OTA升级功能测试脚本
"""

import requests
import json
import time
import os

def test_ota_simple():
    """简化的OTA升级测试"""

    device_ip = "192.168.16.105"

    print("=== 简化的OTA升级测试 ===")

    # 1. 检查设备状态
    print("\n1. 检查设备状态...")
    try:
        response = requests.get(f"http://{device_ip}/status", timeout=10)
        if response.status_code == 200:
            status_data = response.json()
            print(f"   设备在线，固件版本: {status_data.get('firmware_version', '未知')}")
        else:
            print(f"   设备状态检查失败: {response.status_code}")
            return
    except Exception as e:
        print(f"   设备连接失败: {e}")
        return

    # 2. 检查OTA页面
    print("\n2. 检查OTA升级页面...")
    try:
        response = requests.get(f"http://{device_ip}/ota_update", timeout=10)
        if response.status_code == 200:
            print("   OTA页面可访问")
        else:
            print(f"   OTA页面访问失败: {response.status_code}")
    except Exception as e:
        print(f"   OTA页面访问失败: {e}")

    # 3. 测试固件文件上传
    print("\n3. 测试固件文件上传...")
    firmware_path = ".pio/build/nodemcuv2/firmware.bin"
    if os.path.exists(firmware_path):
        file_size = os.path.getsize(firmware_path)
        print(f"   固件文件存在，大小: {file_size} 字节")

        try:
            with open(firmware_path, 'rb') as f:
                files = {'firmware': ('firmware.bin', f, 'application/octet-stream')}
                print("   开始上传固件文件...")

                # 使用更长的超时时间
                response = requests.post(f"http://{device_ip}/update", files=files, timeout=120)

                print(f"   上传响应状态码: {response.status_code}")
                print(f"   响应内容: {response.text[:200]}...")

                if response.status_code == 200:
                    print("   固件上传成功，设备应该会重启")
                    print("   请等待设备重启完成后重新检查状态")
                else:
                    print(f"   固件上传失败: {response.status_code}")

        except requests.exceptions.Timeout:
            print("   上传超时 - 这可能是正常的，因为设备在升级后会重启")
            print("   请等待30秒后检查设备是否重新上线")
        except Exception as e:
            print(f"   上传过程中出错: {e}")
    else:
        print("   固件文件不存在，请先编译项目")

    # 4. 等待设备重启并检查状态
    print("\n4. 等待设备重启...")
    time.sleep(30)  # 等待30秒

    print("   检查设备是否重新上线...")
    try:
        response = requests.get(f"http://{device_ip}/status", timeout=10)
        if response.status_code == 200:
            status_data = response.json()
            print(f"   设备重新上线，固件版本: {status_data.get('firmware_version', '未知')}")
            print("   OTA升级测试完成")
        else:
            print(f"   设备重启后状态检查失败: {response.status_code}")
    except Exception as e:
        print(f"   设备重启后连接失败: {e}")
        print("   可能需要手动检查设备状态")

if __name__ == "__main__":
    test_ota_simple()