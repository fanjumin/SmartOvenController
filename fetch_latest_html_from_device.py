#!/usr/bin/env python

"""
从设备中获取最新的HTML文件
该脚本通过HTTP请求从智能烤箱控制器设备获取最新的HTML文件内容
并保存到本地data目录，以确保使用最新版本
"""

import os
import requests
import time
from datetime import datetime

def ensure_directory_exists(directory):
    """确保目录存在，如果不存在则创建"""
    if not os.path.exists(directory):
        os.makedirs(directory)
        print(f"已创建目录: {directory}")

def fetch_file_from_device(device_ip, file_path, output_dir):
    """从设备获取指定文件并保存到本地"""
    url = f"http://{device_ip}/{file_path}"
    output_path = os.path.join(output_dir, os.path.basename(file_path))
    
    try:
        print(f"正在从设备获取: {file_path}")
        print(f"请求URL: {url}")
        
        # 发送请求
        response = requests.get(url, timeout=10)
        response.raise_for_status()  # 如果状态码不是200，抛出异常
        
        # 检查响应内容是否为空
        if not response.content:
            print(f"警告: 从设备获取的 {file_path} 内容为空")
            return False
        
        # 尝试不同路径前缀
        for prefix in prefixes:
            file_url = f"http://{device_ip}/{prefix}{filename.lstrip('/')}"
            try:
                response = requests.get(file_url, timeout=10)
                response.raise_for_status()
                # 保存文件
                with open(output_path, 'wb') as f:
                    f.write(response.content)
                print(f"✓ 成功保存到: {output_path}")
                print(f"文件大小: {len(response.content)} 字节\n")
                success = True
                break
            except requests.exceptions.RequestException as e:
                print(f"✗ HTTP错误: {str(e)}")
                continue
        return True
    
    except requests.exceptions.ConnectionError:
        print(f"✗ 连接错误: 无法连接到设备 {device_ip}")
        print("  请检查设备是否在线，IP地址是否正确")
        return False
    except requests.exceptions.Timeout:
        print(f"✗ 请求超时: 连接到设备 {device_ip} 超时")
        return False
    except requests.exceptions.HTTPError as e:
        print(f"✗ HTTP错误: {e.response.status_code} - {e.response.reason}")
        return False
    except Exception as e:
        print(f"✗ 获取文件时出错: {str(e)}")
        return False

def main():
    """主函数"""
    print("========== 从智能烤箱控制器设备获取最新HTML文件 ==========")
    print("该工具将从设备中获取最新的HTML文件并保存到本地data目录")
    print()
    
    # 默认设备IP地址
    default_ip = "192.168.16.104"  # 用户提供的设备IP
    
    # 获取用户输入的设备IP
    device_ip = input(f"请输入设备IP地址 [默认: {default_ip}]: ")
    if not device_ip.strip():
        device_ip = default_ip
    
    print(f"使用设备IP: {device_ip}")
    print()
    
    # 要获取的HTML文件列表
    html_files = [
        ("index.html", ["", "data/"]),
        ("device_status.html", ["", "data/"]),
        ("login.html", ["", "data/"]),
        ("settings_help.html", ["", "data/"]),
        ("wifi_config.html", ["", "data/"]),
        ("smart_oven_controller.html", ["", "data/"]),
        ("temperature_control.html", ["", "data/"])
    ]
    
    # 创建输出目录
    output_dir = "data_extracted"
    ensure_directory_exists(output_dir)
    
    # 创建备份目录
    backup_dir = f"data_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    if os.path.exists("data") and os.listdir("data"):
        ensure_directory_exists(backup_dir)
        
        # 备份当前data目录中的文件
        import shutil
        for file in os.listdir("data"):
            if file.lower().endswith('.html'):
                src = os.path.join("data", file)
                dst = os.path.join(backup_dir, file)
                shutil.copy2(src, dst)
                print(f"已备份: {file} -> {backup_dir}/{file}")
    
    print(f"\n开始从设备获取HTML文件...")
    print("="*60)
    
    # 获取文件
    success_count = 0
    for html_file in html_files:
        # 有些设备可能将文件放在根目录，有些可能在data目录下
        # 先尝试直接路径
        if fetch_file_from_device(device_ip, html_file, output_dir):
            success_count += 1
        else:
            # 再尝试data路径
            print(f"尝试从data路径获取: {html_file}")
            if fetch_file_from_device(device_ip, f"data/{html_file}", output_dir):
                success_count += 1
        print()
        time.sleep(1)  # 避免请求过快
    
    # 总结
    print("="*60)
    print(f"获取完成！成功: {success_count}/{len(html_files)} 个文件")
    print(f"\n最新文件已保存到: {output_dir}")
    
    if backup_dir:
        print(f"原有文件已备份到: {backup_dir}")
    
    # 询问是否要替换现有的data目录
    if success_count > 0:
        replace = input("\n是否要将获取的文件复制到主data目录？(y/n): ")
        if replace.lower() == 'y':
            ensure_directory_exists("data")
            for file in os.listdir(output_dir):
                src = os.path.join(output_dir, file)
                dst = os.path.join("data", file)
                shutil.copy2(src, dst)
                print(f"已复制: {file} -> data/{file}")
            print("\n✓ 已成功更新data目录中的文件")
    
    print("\n提示:")
    print("1. 如果无法连接到设备，请确保设备已开机并处于正常工作状态")
    print("2. 检查设备IP地址是否正确")
    print("3. 确认设备已连接到WiFi网络(如果不在AP模式)")

if __name__ == '__main__':
    main()