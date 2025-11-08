#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HTML文件上传工具
用于将data文件夹中的HTML文件上传到ESP8266设备的LittleFS文件系统
"""

import os
import serial
import time
import sys

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
        
        # 上传每个文件
        for file_name in html_files:
            file_path = os.path.join(data_dir, file_name)
            
            # 读取文件内容
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            print(f"\n正在上传: {file_name} ({len(content)} 字节)")
            
            # 发送文件上传命令
            upload_cmd = f'echo "{content}" > /{file_name}\r\n'
            ser.write(upload_cmd.encode('utf-8'))
            
            # 等待上传完成
            time.sleep(2)
            
            # 读取响应
            response = b''
            while ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
                time.sleep(0.1)
            
            if response:
                print(f"设备响应: {response.decode('utf-8', errors='ignore')}")
            
            print(f"✓ {file_name} 上传完成")
        
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
    print("=== HTML文件上传工具 ===")
    print("将data文件夹中的HTML文件上传到ESP8266设备")
    print("=" * 40)
    
    # 配置参数
    port = 'COM11'  # 默认端口
    data_dir = 'data'
    
    # 检查是否有命令行参数
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    print(f"目标端口: {port}")
    print(f"数据目录: {data_dir}")
    
    # 确认操作
    input("按回车键开始上传，或按Ctrl+C取消...")
    
    # 执行上传
    success = upload_html_to_device(port, 115200, data_dir)
    
    if success:
        print("\n✓ 上传成功！")
        print("现在可以通过浏览器访问设备查看新的HTML界面")
    else:
        print("\n✗ 上传失败")
        print("请检查：")
        print("1. 设备是否正确连接到 {port}")
        print("2. 设备是否正常运行")
        print("3. 串口是否被其他程序占用")

if __name__ == "__main__":
    main()