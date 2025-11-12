#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HTML文件串口上传工具 - 通过HTTP协议上传
使用串口连接设备，然后通过HTTP协议上传HTML文件
"""

import os
import serial
import time
import sys
import requests

def upload_html_via_http(device_ip='192.168.16.104', data_dir='data'):
    """
    通过HTTP协议上传HTML文件到设备
    
    Args:
        device_ip: 设备IP地址
        data_dir: 包含HTML文件的目录
    """
    
    # 检查data目录是否存在
    if not os.path.exists(data_dir):
        print(f"错误: 目录 '{data_dir}' 不存在")
        return False
    
    # 获取所有HTML文件
    html_files = []
    for file in os.listdir(data_dir):
        if file.endswith(('.html', '.js', '.css')):
            html_files.append(file)
    
    if not html_files:
        print("错误: 在data目录中未找到HTML文件")
        return False
    
    print(f"找到 {len(html_files)} 个HTML文件:")
    for file in html_files:
        print(f"  - {file}")
    
    # 检查设备是否在线
    try:
        response = requests.get(f"http://{device_ip}/", timeout=10)
        if response.status_code != 200:
            print(f"设备连接失败，状态码: {response.status_code}")
            return False
    except Exception as e:
        print(f"无法连接到设备: {e}")
        return False
    
    print(f"✓ 设备连接正常 (IP: {device_ip})")
    
    # 上传每个文件
    success_count = 0
    for file_name in html_files:
        file_path = os.path.join(data_dir, file_name)
        
        try:
            # 使用multipart/form-data格式上传文件
            with open(file_path, 'rb') as f:
                files = {'file': (file_name, f, 'text/html')}
                response = requests.post(
                    f"http://{device_ip}/upload",
                    files=files,
                    timeout=30
                )
            
            if response.status_code == 200:
                print(f"✓ {file_name} 上传成功")
                success_count += 1
            else:
                print(f"✗ {file_name} 上传失败: {response.status_code}")
                print(f"响应内容: {response.text}")
                
        except Exception as e:
            print(f"✗ {file_name} 上传错误: {e}")
    
    print(f"\n上传完成: {success_count}/{len(html_files)} 个文件成功")
    
    if success_count > 0:
        # 重启设备以应用更改
        try:
            print("\n重启设备以应用更改...")
            response = requests.post(f"http://{device_ip}/restart", timeout=10)
            print("✓ 重启命令已发送")
            print("请等待30秒后重新连接设备")
        except:
            print("⚠ 重启命令发送失败，请手动重启设备")
    
    return success_count == len(html_files)

def get_device_ip_from_serial(port='COM11', baudrate=115200):
    """
    通过串口获取设备IP地址
    """
    try:
        ser = serial.Serial(port, baudrate, timeout=5)
        print(f"已连接到串口 {port}")
        
        # 等待设备启动完成
        time.sleep(3)
        
        # 清空缓冲区
        ser.reset_input_buffer()
        
        # 发送获取IP地址的命令
        ser.write(b'\n')
        time.sleep(1)
        
        # 读取设备输出，查找IP地址
        ip_address = None
        start_time = time.time()
        
        while time.time() - start_time < 10:  # 最多等待10秒
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                print(f"设备输出: {line}")
                
                # 查找IP地址模式
                if 'IP Address:' in line or 'IP:' in line or '192.168.' in line:
                    # 提取IP地址
                    import re
                    ip_match = re.search(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', line)
                    if ip_match:
                        ip_address = ip_match.group(1)
                        print(f"✓ 找到设备IP地址: {ip_address}")
                        break
            
            time.sleep(0.5)
        
        ser.close()
        return ip_address
        
    except Exception as e:
        print(f"串口连接错误: {e}")
        return None

def main():
    """主函数"""
    print("=== HTML文件串口上传工具 ===")
    print("通过串口连接设备，然后通过HTTP协议上传HTML文件")
    print("=" * 50)
    
    # 配置参数
    port = 'COM11'  # 默认端口
    data_dir = 'data'
    
    # 检查是否有命令行参数
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    print(f"串口端口: {port}")
    print(f"数据目录: {data_dir}")
    
    # 第一步：通过串口获取设备IP地址
    print("\n1. 正在通过串口获取设备IP地址...")
    device_ip = get_device_ip_from_serial(port)
    
    if not device_ip:
        # 如果无法获取IP，使用默认IP
        device_ip = '192.168.16.104'
        print(f"⚠ 无法获取设备IP，使用默认IP: {device_ip}")
    
    # 第二步：通过HTTP协议上传文件
    print(f"\n2. 通过HTTP协议上传文件到设备 (IP: {device_ip})...")
    
    # 确认操作
    input("按回车键开始上传，或按Ctrl+C取消...")
    
    # 执行上传
    success = upload_html_via_http(device_ip, data_dir)
    
    if success:
        print("\n✓ 所有文件上传成功！")
        print("现在可以通过浏览器访问设备查看新的HTML界面")
        print(f"访问地址: http://{device_ip}")
    else:
        print("\n✗ 上传失败")
        print("请检查：")
        print("1. 设备是否正确连接到网络")
        print("2. 设备IP地址是否正确")
        print("3. 设备和电脑是否在同一网络")

if __name__ == "__main__":
    main()