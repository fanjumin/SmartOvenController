#!/usr/bin/env python3
"""
设备启动状态监控脚本
"""

import requests
import time
import sys

def monitor_device_startup(device_ip="192.168.16.105", max_wait_time=120):
    """监控设备启动状态"""

    print(f"=== 监控设备启动状态 (IP: {device_ip}) ===")
    print(f"最多等待 {max_wait_time} 秒...")

    start_time = time.time()

    while time.time() - start_time < max_wait_time:
        try:
            print(f"\n[{int(time.time() - start_time)}s] 尝试连接设备...")
            response = requests.get(f"http://{device_ip}/status", timeout=5)

            if response.status_code == 200:
                status_data = response.json()
                firmware_version = status_data.get('firmware_version', '未知')
                device_id = status_data.get('device_id', '未知')
                ip_address = status_data.get('ip_address', '未知')

                print("[SUCCESS] 设备已启动成功！")
                print(f"   设备ID: {device_id}")
                print(f"   固件版本: {firmware_version}")
                print(f"   IP地址: {ip_address}")
                print(f"   启动耗时: {int(time.time() - start_time)} 秒")

                return True
            else:
                print(f"   设备响应异常: HTTP {response.status_code}")

        except requests.exceptions.ConnectionError:
            print("   设备未响应，等待中...")
            time.sleep(5)
        except requests.exceptions.Timeout:
            print("   连接超时，等待中...")
            time.sleep(5)
        except Exception as e:
            print(f"   连接错误: {e}")
            time.sleep(5)

    print(f"\n[FAILED] 等待超时 ({max_wait_time}秒)")
    print("设备可能启动失败或需要手动重启")
    return False

def check_device_basic_connectivity(device_ip="192.168.16.105"):
    """检查基本的网络连接性"""

    print(f"\n=== 检查网络连接性 (IP: {device_ip}) ===")

    import subprocess
    import platform

    try:
        # 使用ping命令检查网络连接
        if platform.system().lower() == "windows":
            result = subprocess.run(["ping", "-n", "1", device_ip],
                                  capture_output=True, text=True, timeout=5)
        else:
            result = subprocess.run(["ping", "-c", "1", device_ip],
                                  capture_output=True, text=True, timeout=5)

        if result.returncode == 0:
            print("[OK] 网络连接正常")
            return True
        else:
            print("[ERROR] 网络连接失败")
            print(f"   Ping结果: {result.stdout.strip()}")
            return False

    except Exception as e:
        print(f"[ERROR] Ping测试失败: {e}")
        return False

if __name__ == "__main__":
    device_ip = "192.168.16.105"

    # 首先检查网络连接性
    if check_device_basic_connectivity(device_ip):
        # 如果网络连接正常，监控设备启动
        monitor_device_startup(device_ip)
    else:
        print("\n建议检查:")
        print("1. 设备是否已上电")
        print("2. 网络连接是否正常")
        print("3. 设备IP地址是否正确")
        print("4. 防火墙设置是否阻止连接")