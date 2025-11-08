#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HTML文件OTA升级工具
通过HTTP协议升级ESP8266设备上的HTML文件
"""

import requests
import os
import time
import json

class HTMLOTAUpdater:
    def __init__(self, device_ip='192.168.4.1'):
        self.device_ip = device_ip
        self.base_url = f"http://{device_ip}"
        
    def check_device_status(self):
        """检查设备状态"""
        try:
            response = requests.get(f"{self.base_url}/", timeout=10)
            return response.status_code == 200
        except:
            return False
    
    def upload_html_file(self, file_path, target_path=None):
        """
        上传单个HTML文件到设备
        
        Args:
            file_path: 本地文件路径
            target_path: 设备上的目标路径（如不指定则使用文件名）
        """
        if not os.path.exists(file_path):
            print(f"错误: 文件不存在 {file_path}")
            return False
        
        if target_path is None:
            target_path = os.path.basename(file_path)
        
        # 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 准备上传数据
        upload_data = {
            'filename': target_path,
            'content': content
        }
        
        try:
            # 发送上传请求
            response = requests.post(
                f"{self.base_url}/upload_html",
                data=json.dumps(upload_data),
                headers={'Content-Type': 'application/json'},
                timeout=30
            )
            
            if response.status_code == 200:
                print(f"✓ {file_path} -> {target_path} 上传成功")
                return True
            else:
                print(f"✗ {file_path} 上传失败: {response.status_code}")
                return False
                
        except Exception as e:
            print(f"✗ {file_path} 上传错误: {e}")
            return False
    
    def upload_all_html_files(self, data_dir='data'):
        """上传data目录中的所有HTML文件"""
        if not os.path.exists(data_dir):
            print(f"错误: 目录不存在 {data_dir}")
            return False
        
        # 获取所有HTML相关文件
        html_files = []
        for file in os.listdir(data_dir):
            if file.endswith(('.html', '.js', '.css')):
                html_files.append(file)
        
        if not html_files:
            print("错误: 未找到HTML文件")
            return False
        
        print(f"找到 {len(html_files)} 个文件:")
        for file in html_files:
            print(f"  - {file}")
        
        # 上传每个文件
        success_count = 0
        for file in html_files:
            file_path = os.path.join(data_dir, file)
            if self.upload_html_file(file_path):
                success_count += 1
        
        print(f"\n上传完成: {success_count}/{len(html_files)} 个文件成功")
        return success_count == len(html_files)
    
    def reboot_device(self):
        """重启设备"""
        try:
            response = requests.get(f"{self.base_url}/reboot", timeout=10)
            if response.status_code == 200:
                print("✓ 设备重启命令已发送")
                return True
        except:
            print("✗ 设备重启失败")
            return False

def main():
    """主函数"""
    print("=== HTML文件OTA升级工具 ===")
    print("通过HTTP协议升级设备HTML文件")
    print("=" * 40)
    
    # 创建升级器
    updater = HTMLOTAUpdater('192.168.4.1')  # 默认设备IP
    
    # 检查设备状态
    print("检查设备状态...")
    if not updater.check_device_status():
        print("✗ 无法连接到设备")
        print("请确保:")
        print("1. 设备已启动并创建WiFi热点")
        print("2. 计算机已连接到设备热点 (SSID: SmartOven-XXXX)")
        print("3. 设备IP地址正确 (默认: 192.168.4.1)")
        return
    
    print("✓ 设备连接正常")
    
    # 上传HTML文件
    print("\n开始上传HTML文件...")
    if updater.upload_all_html_files('data'):
        print("\n✓ 所有文件上传成功!")
        
        # 询问是否重启设备
        choice = input("\n是否重启设备以应用更改? (y/n): ").lower()
        if choice == 'y':
            updater.reboot_device()
            print("设备重启中，请等待30秒后重新连接...")
    else:
        print("\n✗ 文件上传失败")

if __name__ == "__main__":
    main()