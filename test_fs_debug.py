#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
文件系统更新调试脚本
用于诊断OTA文件系统更新问题
"""

import requests
import time
import serial
import serial.tools.list_ports
import sys

def find_serial_port():
    """查找ESP8266的串口"""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "CH340" in port.description or "CP210" in port.description or "USB" in port.description:
            return port.device
    return None

def monitor_serial(port, timeout=30):
    """监控串口输出"""
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        print(f"正在监控串口 {port}...")
        start_time = time.time()

        while time.time() - start_time < timeout:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[SERIAL] {line}")
                    # 检查关键信息
                    if "检测到文件系统更新请求" in line:
                        print("✅ 检测到文件系统更新标志！")
                    elif "文件系统更新成功完成" in line:
                        print("✅ 文件系统更新成功！")
                    elif "文件系统更新结束失败" in line:
                        print("❌ 文件系统更新失败！")
            time.sleep(0.1)

        ser.close()
    except Exception as e:
        print(f"串口监控错误: {e}")

def test_clear_cache(device_ip):
    """测试缓存清除功能"""
    print(f"\n=== 测试缓存清除功能 ({device_ip}) ===")
    try:
        url = f"http://{device_ip}/clear_cache"
        print(f"发送POST请求到: {url}")

        response = requests.post(url, timeout=10)
        print(f"响应状态码: {response.status_code}")
        print(f"响应内容: {response.text}")

        if response.status_code == 200:
            try:
                result = response.json()
                if result.get('status') == 'success':
                    print("✅ 缓存清除请求成功！")
                    deleted_count = result.get('deleted_files', 0)
                    print(f"删除了 {deleted_count} 个文件")
                    return True
                else:
                    print(f"❌ 缓存清除失败: {result.get('message', '未知错误')}")
                    return False
            except:
                print("❌ 响应解析失败")
                return False
        else:
            print(f"❌ HTTP错误: {response.status_code}")
            return False

    except requests.exceptions.RequestException as e:
        print(f"❌ 网络错误: {e}")
        return False

def test_filesystem_update(device_ip, fs_file_path):
    """测试文件系统更新功能"""
    print(f"\n=== 测试文件系统更新功能 ({device_ip}) ===")
    try:
        url = f"http://{device_ip}/filesystem_update"
        print(f"上传文件: {fs_file_path}")
        print(f"目标URL: {url}")

        with open(fs_file_path, 'rb') as f:
            files = {'filesystem': ('littlefs.bin', f, 'application/octet-stream')}
            print("开始上传文件系统映像...")

            response = requests.post(url, files=files, stream=True, timeout=60)
            print(f"初始响应状态码: {response.status_code}")

            if response.status_code == 200:
                print("✅ 文件上传成功，开始监控串口...")
                return True
            else:
                print(f"❌ 上传失败，状态码: {response.status_code}")
                print(f"响应内容: {response.text}")
                return False

    except FileNotFoundError:
        print(f"❌ 文件不存在: {fs_file_path}")
        return False
    except requests.exceptions.RequestException as e:
        print(f"❌ 网络错误: {e}")
        return False

def check_device_status(device_ip):
    """检查设备状态"""
    print(f"\n=== 检查设备状态 ({device_ip}) ===")
    try:
        url = f"http://{device_ip}/status"
        response = requests.get(url, timeout=5)
        print(f"状态响应: {response.status_code}")
        if response.status_code == 200:
            print("✅ 设备响应正常")
            try:
                status = response.json()
                print(f"设备ID: {status.get('device_id', 'N/A')}")
                print(f"固件版本: {status.get('firmware_version', 'N/A')}")
                return True
            except:
                print("无法解析状态JSON")
                return False
        else:
            print(f"❌ 设备无响应，状态码: {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print(f"❌ 连接设备失败: {e}")
        return False

def main():
    device_ip = "192.168.16.100"
    fs_file_path = ".pio/build/nodemcuv2/littlefs.bin"

    print("ESP8266 文件系统更新调试工具")
    print("=" * 40)

    # 查找串口
    serial_port = find_serial_port()
    if serial_port:
        print(f"找到串口: {serial_port}")
    else:
        print("⚠️  未找到ESP8266串口，请手动连接串口调试")

    # 检查设备状态
    if not check_device_status(device_ip):
        print("❌ 无法连接到设备，请检查设备IP和网络连接")
        return

    # 测试缓存清除
    if not test_clear_cache(device_ip):
        print("❌ 缓存清除失败")
        return

    print("等待设备处理缓存清除...")
    time.sleep(5)

    # 再次检查设备状态（应该无法访问，因为文件已被清除）
    if check_device_status(device_ip):
        print("⚠️  设备仍然响应，可能缓存清除不完全")
    else:
        print("✅ 缓存清除成功，设备文件系统已清空")

    # 测试文件系统更新
    if test_filesystem_update(device_ip, fs_file_path):
        print("文件上传成功，等待设备重启...")

        # 监控串口（如果找到的话）
        if serial_port:
            monitor_serial(serial_port, 60)
        else:
            print("等待60秒让设备完成更新...")
            time.sleep(60)

        # 最终检查
        print("\n最终状态检查:")
        if check_device_status(device_ip):
            print("✅ 文件系统更新成功！设备已恢复正常")
        else:
            print("❌ 文件系统更新可能失败")
    else:
        print("❌ 文件系统更新测试失败")

if __name__ == "__main__":
    main()