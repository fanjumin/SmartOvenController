#!/usr/bin/env python3
"""
OTA固件更新脚本
通过Web界面更新设备固件
"""

import requests
import time
import os

def ota_update_firmware():
    """通过OTA更新固件"""
    print("=== OTA固件更新 ===")
    
    device_ip = "192.168.16.104"
    firmware_file = ".pio\\build\\nodemcuv2\\firmware.bin"
    
    # 检查固件文件是否存在
    if not os.path.exists(firmware_file):
        print(f"❌ 固件文件不存在: {firmware_file}")
        return False
    
    print(f"固件文件大小: {os.path.getsize(firmware_file)} 字节")
    
    # 尝试OTA更新
    try:
        # 访问OTA页面
        ota_url = f"http://{device_ip}/update"
        print(f"访问OTA页面: {ota_url}")
        
        response = requests.get(ota_url, timeout=10)
        print(f"OTA页面状态码: {response.status_code}")
        
        if response.status_code == 200:
            print("✅ OTA页面可访问")
            
            # 上传固件文件
            print("开始上传固件...")
            with open(firmware_file, 'rb') as f:
                files = {'firmware': ('firmware.bin', f, 'application/octet-stream')}
                upload_response = requests.post(ota_url, files=files, timeout=60)
                
                print(f"上传响应状态码: {upload_response.status_code}")
                print(f"上传响应内容: {upload_response.text}")
                
                if upload_response.status_code == 200:
                    print("✅ 固件上传成功，设备正在重启...")
                    
                    # 等待设备重启
                    print("等待设备重启（约30秒）...")
                    time.sleep(30)
                    
                    # 检查设备是否重新上线
                    print("检查设备状态...")
                    try:
                        status_response = requests.get(f"http://{device_ip}/status", timeout=10)
                        print(f"设备状态: {status_response.text}")
                        return True
                    except:
                        print("⚠️ 设备重启中，请稍后手动检查")
                        return True
                else:
                    print("❌ 固件上传失败")
                    return False
        else:
            print("❌ OTA页面不可访问")
            return False
            
    except Exception as e:
        print(f"❌ OTA更新失败: {e}")
        return False

def check_device_after_update():
    """更新后检查设备状态"""
    print("\n=== 更新后设备检查 ===")
    
    device_ip = "192.168.16.104"
    
    # 等待设备稳定
    time.sleep(10)
    
    # 检查设备是否在线
    try:
        response = requests.get(f"http://{device_ip}/status", timeout=10)
        print(f"设备状态: {response.text}")
        
        # 检查固件版本
        if "firmware_version" in response.text:
            import json
            data = json.loads(response.text)
            print(f"固件版本: {data.get('firmware_version', '未知')}")
            
        return True
    except Exception as e:
        print(f"❌ 设备检查失败: {e}")
        return False

if __name__ == "__main__":
    if ota_update_firmware():
        check_device_after_update()
    else:
        print("\n⚠️ 建议使用串口方式更新固件")