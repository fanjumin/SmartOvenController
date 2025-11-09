#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HTML文件上传验证工具
用于验证HTML文件上传后是否能正确生效
"""

import os
import serial
import time
import sys
import requests
import json

def check_file_on_device(ser, filename):
    """检查设备上的文件是否存在"""
    try:
        # 发送文件检查命令
        ser.write(f'ls -la /{filename}\r\n'.encode('utf-8'))
        time.sleep(1)
        
        # 读取响应
        response = b''
        start_time = time.time()
        while time.time() - start_time < 3:
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        response_text = response.decode('utf-8', errors='ignore')
        
        if filename in response_text and 'No such file' not in response_text:
            print(f"✓ 设备上存在文件: {filename}")
            
            # 获取文件大小
            ser.write(f'wc -c /{filename}\r\n'.encode('utf-8'))
            time.sleep(1)
            
            size_response = b''
            start_time = time.time()
            while time.time() - start_time < 3:
                if ser.in_waiting > 0:
                    size_response += ser.read(ser.in_waiting)
                time.sleep(0.1)
            
            print(f"文件大小信息: {size_response.decode('utf-8', errors='ignore')}")
            return True
        else:
            print(f"✗ 设备上不存在文件: {filename}")
            return False
            
    except Exception as e:
        print(f"检查文件时发生错误: {e}")
        return False

def test_web_access(device_ip, filename):
    """测试Web访问"""
    try:
        url = f"http://{device_ip}/{filename}"
        print(f"测试Web访问: {url}")
        
        response = requests.get(url, timeout=10)
        
        if response.status_code == 200:
            print(f"✓ Web访问成功，状态码: {response.status_code}")
            print(f"内容长度: {len(response.text)} 字节")
            
            # 检查是否为有效的HTML
            if '<html' in response.text.lower() or '<!doctype' in response.text.lower():
                print("✓ 返回内容为有效的HTML")
            else:
                print("⚠ 返回内容可能不是有效的HTML")
                
            return True
        else:
            print(f"✗ Web访问失败，状态码: {response.status_code}")
            return False
            
    except requests.exceptions.RequestException as e:
        print(f"Web访问测试失败: {e}")
        return False

def test_api_endpoints(device_ip):
    """测试API端点"""
    endpoints = [
        "/scanwifi",
        "/savewifi", 
        "/status",
        "/wificonfig"
    ]
    
    print("\n测试API端点:")
    
    for endpoint in endpoints:
        try:
            url = f"http://{device_ip}{endpoint}"
            response = requests.get(url, timeout=5)
            
            if response.status_code == 200:
                print(f"✓ {endpoint} - 状态码: {response.status_code}")
            else:
                print(f"⚠ {endpoint} - 状态码: {response.status_code}")
                
        except requests.exceptions.RequestException as e:
            print(f"✗ {endpoint} - 错误: {e}")

def verify_upload_process(port='COM11', device_ip='192.168.4.1', data_dir='data'):
    """验证整个上传过程"""
    print("=== HTML文件上传验证工具 ===")
    print("=" * 50)
    
    # 检查数据目录
    if not os.path.exists(data_dir):
        print(f"错误: 目录 '{data_dir}' 不存在")
        return False
    
    # 获取HTML文件列表
    html_files = []
    for file in os.listdir(data_dir):
        if file.endswith('.html'):
            html_files.append(file)
    
    if not html_files:
        print("错误: 在data目录中未找到HTML文件")
        return False
    
    print(f"找到 {len(html_files)} 个HTML文件:")
    for file in html_files:
        print(f"  - {file}")
    
    try:
        # 连接串口
        ser = serial.Serial(port, 115200, timeout=5)
        print(f"\n已连接到串口: {port}")
        
        # 等待设备就绪
        time.sleep(2)
        
        # 清空缓冲区
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        
        print("\n1. 检查设备当前文件状态:")
        for filename in html_files:
            check_file_on_device(ser, filename)
        
        print("\n2. 测试Web访问（上传前）:")
        for filename in html_files:
            test_web_access(device_ip, filename)
        
        print("\n3. 测试API端点:")
        test_api_endpoints(device_ip)
        
        print("\n4. 执行文件上传...")
        print("请先运行 upload_html_files_fixed.py 进行文件上传")
        print("上传完成后按回车键继续验证...")
        input()
        
        print("\n5. 验证上传后的文件状态:")
        for filename in html_files:
            check_file_on_device(ser, filename)
        
        print("\n6. 测试Web访问（上传后）:")
        for filename in html_files:
            test_web_access(device_ip, filename)
        
        ser.close()
        
        print("\n✓ 验证完成")
        print("\n建议:")
        print("1. 如果文件上传后Web访问仍然失败，请检查设备IP地址")
        print("2. 确保设备已正确重启")
        print("3. 检查防火墙或网络设置")
        
        return True
        
    except serial.SerialException as e:
        print(f"串口连接错误: {e}")
        return False
    except Exception as e:
        print(f"验证过程中发生错误: {e}")
        return False

def main():
    """主函数"""
    print("HTML文件上传验证工具")
    print("用于验证HTML文件上传后是否能正确生效")
    print("=" * 50)
    
    # 配置参数
    port = 'COM11'  # 默认串口
    device_ip = '192.168.4.1'  # 默认设备IP
    data_dir = 'data'
    
    # 检查命令行参数
    if len(sys.argv) > 1:
        port = sys.argv[1]
    if len(sys.argv) > 2:
        device_ip = sys.argv[2]
    
    print(f"目标串口: {port}")
    print(f"设备IP: {device_ip}")
    print(f"数据目录: {data_dir}")
    
    # 确认操作
    input("\n按回车键开始验证，或按Ctrl+C取消...")
    
    # 执行验证
    success = verify_upload_process(port, device_ip, data_dir)
    
    if success:
        print("\n✓ 验证过程完成")
    else:
        print("\n✗ 验证过程失败")

if __name__ == "__main__":
    main()