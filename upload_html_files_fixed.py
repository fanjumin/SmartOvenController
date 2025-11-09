#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HTML文件上传工具 - 修复版
使用更可靠的方法将data文件夹中的HTML文件上传到ESP8266设备的LittleFS文件系统
"""

import os
import serial
import time
import sys
import base64

def upload_file_to_device(ser, file_path, device_path):
    """
    使用base64编码上传文件到设备
    """
    try:
        # 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 使用base64编码避免特殊字符问题
        encoded_content = base64.b64encode(content.encode('utf-8')).decode('ascii')
        
        print(f"正在上传: {os.path.basename(file_path)} ({len(content)} 字节)")
        
        # 发送base64解码和写入命令
        cmd = f'echo {encoded_content} | base64 -d > {device_path}\r\n'
        ser.write(cmd.encode('utf-8'))
        
        # 等待上传完成
        time.sleep(2)
        
        # 读取响应
        response = b''
        start_time = time.time()
        while time.time() - start_time < 5:  # 最多等待5秒
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print(f"设备响应: {response.decode('utf-8', errors='ignore')}")
        
        # 验证文件是否成功写入
        ser.write(f'cat {device_path} | head -c 50\r\n'.encode('utf-8'))
        time.sleep(1)
        
        verification = b''
        start_time = time.time()
        while time.time() - start_time < 3:
            if ser.in_waiting > 0:
                verification += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if verification:
            print(f"文件验证: {verification.decode('utf-8', errors='ignore')}")
        
        return True
        
    except Exception as e:
        print(f"上传文件 {file_path} 时发生错误: {e}")
        return False

def upload_html_to_device(port='COM11', baudrate=115200, data_dir='data'):
    """
    上传HTML文件到ESP8266设备
    
    Args:
        port: 串口端口
        baudrate: 波特率
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
    
    try:
        # 连接串口
        ser = serial.Serial(port, baudrate, timeout=5)
        print(f"已连接到 {port}")
        
        # 等待设备就绪
        time.sleep(2)
        
        # 清空缓冲区
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        
        # 发送文件上传命令
        ser.write(b'\x03')  # Ctrl+C 中断当前操作
        time.sleep(1)
        
        # 进入文件上传模式
        ser.write(b'\r\n')  # 回车
        time.sleep(0.5)
        
        # 检查设备是否支持base64
        ser.write(b'which base64\r\n')
        time.sleep(1)
        
        base64_check = b''
        start_time = time.time()
        while time.time() - start_time < 3:
            if ser.in_waiting > 0:
                base64_check += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if b'not found' in base64_check.lower():
            print("警告: 设备不支持base64命令，将使用备用方法")
            # 使用备用方法：逐行上传
            for file_name in html_files:
                file_path = os.path.join(data_dir, file_name)
                
                # 读取文件内容
                with open(file_path, 'r', encoding='utf-8') as f:
                    lines = f.readlines()
                
                print(f"\n正在上传: {file_name} ({len(lines)} 行)")
                
                # 创建文件
                ser.write(f'echo "" > /{file_name}\r\n'.encode('utf-8'))
                time.sleep(0.5)
                
                # 逐行追加内容
                for i, line in enumerate(lines):
                    # 转义特殊字符
                    escaped_line = line.replace('"', '\\"').replace('`', '\\`').replace('$', '\\$')
                    cmd = f'echo "{escaped_line.rstrip()}" >> /{file_name}\r\n'
                    ser.write(cmd.encode('utf-8'))
                    
                    # 每10行等待一下
                    if i % 10 == 0:
                        time.sleep(0.2)
                
                time.sleep(1)
                print(f"✓ {file_name} 上传完成")
        else:
            # 使用base64方法上传
            for file_name in html_files:
                file_path = os.path.join(data_dir, file_name)
                success = upload_file_to_device(ser, file_path, f'/{file_name}')
                if success:
                    print(f"✓ {file_name} 上传完成")
                    
                    # 验证文件是否成功写入并刷新文件系统
                    time.sleep(1)
                    ser.write(f'ls -la /{file_name}\r\n'.encode('utf-8'))
                    time.sleep(1)
                    
                    # 读取验证响应
                    verification = b''
                    start_time = time.time()
                    while time.time() - start_time < 3:
                        if ser.in_waiting > 0:
                            verification += ser.read(ser.in_waiting)
                        time.sleep(0.1)
                    
                    if verification:
                        print(f"文件验证: {verification.decode('utf-8', errors='ignore')}")
                else:
                    print(f"✗ {file_name} 上传失败")
        
        # 发送重启命令
        print("\n重启设备...")
        ser.write(b'restart\r\n')
        time.sleep(3)
        
        ser.close()
        print("✓ 所有文件上传完成，设备已重启")
        return True
        
    except serial.SerialException as e:
        print(f"串口连接错误: {e}")
        return False
    except Exception as e:
        print(f"上传过程中发生错误: {e}")
        return False

def main():
    """主函数"""
    print("=== HTML文件上传工具 - 修复版 ===")
    print("使用更可靠的方法将HTML文件上传到ESP8266设备")
    print("=" * 50)
    
    # 配置参数
    port = 'COM11'  # 默认端口
    data_dir = 'data'
    
    # 检查是否有命令行参数
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    print(f"目标端口: {port}")
    print(f"数据目录: {data_dir}")
    
    # 自动开始上传，无需等待用户输入
    print("自动开始上传...")
    
    # 执行上传
    success = upload_html_to_device(port, 115200, data_dir)
    
    if success:
        print("\n✓ 上传成功！")
        print("现在可以通过浏览器访问设备查看新的HTML界面")
    else:
        print("\n✗ 上传失败")
        print("请检查：")
        print(f"1. 设备是否正确连接到 {port}")
        print("2. 设备是否正常运行")
        print("3. 串口是否被其他程序占用")

if __name__ == "__main__":
    main()