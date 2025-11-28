#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
智能烤箱文件系统OTA更新测试脚本
测试文件系统镜像上传功能
"""

import requests
import time
import json
import os
from pathlib import Path

def test_filesystem_update(device_ip="192.168.16.103", fs_image_path=".pio/build/nodemcuv2/littlefs.bin"):
    """
    测试文件系统OTA更新功能

    Args:
        device_ip: 设备IP地址
        fs_image_path: 文件系统镜像路径
    """

    url = f"http://{device_ip}/filesystem_update"

    # 检查文件是否存在
    if not os.path.exists(fs_image_path):
        print(f"错误：文件系统镜像文件不存在: {fs_image_path}")
        return False

    # 获取文件大小
    file_size = os.path.getsize(fs_image_path)
    print(f"文件系统镜像大小: {file_size} 字节")

    try:
        print(f"开始上传文件系统镜像到 {url}")

        # 打开文件
        with open(fs_image_path, 'rb') as f:
            files = {
                'filesystem': ('littlefs.bin', f, 'application/octet-stream')
            }

            # 发送POST请求
            start_time = time.time()
            response = requests.post(url, files=files, timeout=300)  # 5分钟超时
            end_time = time.time()

            print(f"上传完成，耗时: {end_time - start_time:.2f} 秒")
            print(f"响应状态码: {response.status_code}")
            print(f"响应内容: {response.text}")

            if response.status_code == 200:
                try:
                    response_json = response.json()
                    if response_json.get('status') == 'success':
                        print("✅ 文件系统更新请求成功")
                        print("设备应该在3秒后重启...")
                        return True
                    else:
                        print(f"❌ 更新失败: {response_json.get('message', '未知错误')}")
                        return False
                except json.JSONDecodeError:
                    print("❌ 响应不是有效的JSON格式")
                    return False
            else:
                print(f"❌ HTTP错误: {response.status_code}")
                return False

    except requests.exceptions.RequestException as e:
        print(f"❌ 网络请求错误: {e}")
        return False
    except Exception as e:
        print(f"❌ 未知错误: {e}")
        return False

def check_device_status(device_ip="192.168.16.103"):
    """
    检查设备状态
    """
    try:
        url = f"http://{device_ip}/status"
        response = requests.get(url, timeout=10)

        if response.status_code == 200:
            status = response.json()
            print("📊 设备状态:")
            print(f"  固件版本: {status.get('firmware_version', '未知')}")
            print(f"  温度: {status.get('temperature', '未知')}°C")
            print(f"  WiFi连接: {status.get('wifi_connected', '未知')}")
            return True
        else:
            print(f"❌ 无法获取设备状态: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ 检查设备状态失败: {e}")
        return False

def check_index_page(device_ip="192.168.16.103"):
    """
    检查index.html页面是否可访问
    """
    try:
        url = f"http://{device_ip}/index.html"
        response = requests.get(url, timeout=10)

        if response.status_code == 200:
            print("✅ index.html 页面可正常访问")
            return True
        else:
            print(f"❌ index.html 页面不可访问: {response.status_code}")
            if "Not found" in response.text:
                print("  错误信息: Not found: /index.html")
            return False
    except Exception as e:
        print(f"❌ 检查index.html失败: {e}")
        return False

def main():
    device_ip = "192.168.16.103"
    fs_image_path = ".pio/build/nodemcuv2/littlefs.bin"

    print("🔧 智能烤箱文件系统OTA更新测试")
    print("=" * 50)

    # 检查设备状态
    print("\n1. 检查设备状态...")
    if not check_device_status(device_ip):
        print("设备不可访问，请检查网络连接")
        return

    # 检查当前index.html
    print("\n2. 检查当前index.html...")
    check_index_page(device_ip)

    # 执行文件系统更新
    print("\n3. 执行文件系统更新...")
    if test_filesystem_update(device_ip, fs_image_path):
        print("\n4. 等待设备重启...")
        time.sleep(10)  # 等待10秒重启

        print("\n5. 验证更新结果...")
        # 再次检查设备状态
        if check_device_status(device_ip):
            # 检查index.html是否恢复
            if check_index_page(device_ip):
                print("\n✅ 文件系统更新成功！")
            else:
                print("\n❌ 文件系统更新失败：index.html仍不可访问")
        else:
            print("\n❌ 设备重启后不可访问")
    else:
        print("\n❌ 文件系统更新请求失败")

if __name__ == "__main__":
    main()