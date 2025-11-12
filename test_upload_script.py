#!/usr/bin/env python
# -*- coding: utf-8 -*-

import requests
import os
import sys

# 测试系统文件上传功能
def upload_system_file(file_path, device_ip="192.168.4.1"):
    """
    上传系统文件到设备
    file_path: 要上传的文件路径
    device_ip: 设备IP地址
    """
    try:
        # 确保文件存在
        if not os.path.exists(file_path):
            print(f"错误: 文件 {file_path} 不存在")
            return False
        
        # 从文件路径获取文件名
        filename = os.path.basename(file_path)
        
        # 准备上传URL
        url = f"http://{device_ip}/upload"
        
        print(f"正在上传文件: {filename} 到设备 {device_ip}...")
        
        # 打开文件并发送POST请求
        with open(file_path, 'rb') as f:
            files = {'update': (filename, f)}
            response = requests.post(url, files=files, timeout=30)
            
        # 检查响应
        if response.status_code == 200:
            print(f"上传成功! 服务器响应: {response.text}")
            return True
        else:
            print(f"上传失败! 状态码: {response.status_code}")
            print(f"服务器响应: {response.text}")
            return False
            
    except requests.exceptions.ConnectionError:
        print(f"连接错误: 无法连接到设备 {device_ip}")
        print("请确保设备已连接且IP地址正确")
        return False
    except Exception as e:
        print(f"上传过程中发生错误: {str(e)}")
        return False

if __name__ == "__main__":
    # 默认上传我们创建的测试文件
    test_file = "test_system_file.html"
    
    # 如果命令行提供了文件路径，则使用提供的文件
    if len(sys.argv) > 1:
        test_file = sys.argv[1]
    
    # 上传文件
    upload_system_file(test_file)
    
    print("\n注意: 如果设备在配网模式下，IP地址为192.168.4.1")
    print("      如果设备已连接到WiFi，请修改脚本中的device_ip参数")