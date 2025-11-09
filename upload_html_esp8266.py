#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP8266 HTML文件上传工具
专门针对ESP8266设备的文件系统上传HTML文件
"""

import os
import serial
import time
import sys

def upload_file_to_esp8266(ser, file_path, file_name):
    """
    使用ESP8266 Web服务器文件上传功能上传文件
    """
    try:
        # 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        print(f"正在上传: {file_name} ({len(content)} 字节)")
        
        # 等待设备就绪
        time.sleep(1)
        
        # 发送文件内容到设备
        # 使用简单的文本传输方式
        ser.write(b'\x03')  # Ctrl+C 中断当前操作
        time.sleep(0.5)
        
        # 发送文件上传开始标记
        ser.write(f'FILE_START:{file_name}\r\n'.encode('utf-8'))
        time.sleep(0.5)
        
        # 发送文件内容（分块发送）
        chunk_size = 128
        for i in range(0, len(content), chunk_size):
            chunk = content[i:i+chunk_size]
            # 转义特殊字符
            escaped_chunk = chunk.replace('\n', '\\n').replace('\r', '\\r')
            ser.write(f'{escaped_chunk}\r\n'.encode('utf-8'))
            time.sleep(0.1)
        
        # 发送文件结束标记
        ser.write(b'FILE_END\r\n')
        time.sleep(0.5)
        
        print(f"✓ {file_name} 上传完成")
        return True
        
    except Exception as e:
        print(f"上传文件 {file_name} 时发生错误: {e}")
        return False

def upload_html_to_esp8266(port='COM11', baudrate=115200, data_dir='data'):
    """
    上传HTML文件到ESP8266设备
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
        
        # 上传所有文件
        for file_name in html_files:
            file_path = os.path.join(data_dir, file_name)
            success = upload_file_to_esp8266(ser, file_path, file_name)
            if not success:
                print(f"✗ {file_name} 上传失败")
        
        # 发送重启命令
        print("\n重启设备...")
        ser.write(b'restart\r\n')
        time.sleep(3)
        
        ser.close()
        print("✓ 文件上传完成，设备已重启")
        return True
        
    except serial.SerialException as e:
        print(f"串口连接错误: {e}")
        print("请检查：")
        print("1. 设备是否正确连接到 COM11")
        print("2. 设备是否正常运行")
        print("3. 串口是否被其他程序占用")
        return False
    except Exception as e:
        print(f"上传过程中发生错误: {e}")
        return False

def main():
    """主函数"""
    print("=== ESP8266 HTML文件上传工具 ===")
    print("专门针对ESP8266设备的文件系统上传HTML文件")
    print("==================================================")
    
    # 自动开始上传
    success = upload_html_to_esp8266()
    
    if success:
        print("\n✓ 上传成功！")
        print("现在您可以连接设备的WiFi热点并访问 http://192.168.4.1")
    else:
        print("\n✗ 上传失败")

if __name__ == "__main__":
    main()