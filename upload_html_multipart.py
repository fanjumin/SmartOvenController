#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HTML文件上传工具 - 使用multipart/form-data格式
通过HTTP协议上传HTML文件到ESP8266设备
"""

import requests
import os
import time

class HTMLUploader:
    def __init__(self, device_ip='192.168.16.104'):
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
        
        # 确保目标路径以/开头
        if not target_path.startswith('/'):
            target_path = '/' + target_path
        
        try:
            # 使用multipart/form-data格式上传文件
            with open(file_path, 'rb') as f:
                files = {'file': (target_path, f, 'text/html')}
                response = requests.post(
                    f"{self.base_url}/upload",
                    files=files,
                    timeout=30
                )
            
            if response.status_code == 200:
                print(f"✓ {file_path} -> {target_path} 上传成功")
                return True
            else:
                print(f"✗ {file_path} 上传失败: {response.status_code}")
                print(f"响应内容: {response.text}")
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
            response = requests.get(f"{self.base_url}/restart", timeout=10)
            if response.status_code == 200:
                print("✓ 设备重启命令已发送")
                return True
        except:
            print("✗ 设备重启失败")
            return False

def main():
    """主函数"""
    print("=== HTML文件上传工具 ===")
    print("使用multipart/form-data格式上传文件")
    print("=" * 40)
    
    # 获取设备IP地址
    device_ip = input("请输入设备IP地址 (默认: 192.168.16.104): ").strip()
    if not device_ip:
        device_ip = '192.168.16.104'
    
    # 创建上传器
    uploader = HTMLUploader(device_ip)
    
    # 检查设备状态
    print(f"检查设备状态 (IP: {device_ip})...")
    if not uploader.check_device_status():
        print("✗ 无法连接到设备")
        print("请确保:")
        print("1. 设备已启动并连接到网络")
        print("2. 设备IP地址正确")
        print("3. 设备和电脑在同一网络")
        return
    
    print("✓ 设备连接正常")
    
    # 上传HTML文件
    print("\n开始上传HTML文件...")
    if uploader.upload_all_html_files('data'):
        print("\n✓ 所有文件上传成功!")
        
        # 询问是否重启设备
        choice = input("\n是否重启设备以应用更改? (y/n): ").lower()
        if choice == 'y':
            uploader.reboot_device()
            print("设备重启中，请等待30秒后重新连接...")
    else:
        print("\n✗ 文件上传失败")

if __name__ == "__main__":
    main()