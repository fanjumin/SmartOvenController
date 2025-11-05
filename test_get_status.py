#!/usr/bin/env python3
"""
测试设备GET_STATUS命令响应
验证设备是否正确返回温度数据
"""

import socket
import time

def test_get_status():
    """测试GET_STATUS命令"""
    print("=== 测试GET_STATUS命令 ===")
    
    # 设备IP和端口
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        # 创建TCP连接
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, device_port))
        
        # 接收欢迎消息
        welcome = sock.recv(1024).decode('utf-8')
        print(f"设备欢迎消息: {welcome.strip()}")
        
        # 发送GET_STATUS命令
        command = "GET_STATUS\n"
        sock.send(command.encode('utf-8'))
        print(f"发送命令: {command.strip()}")
        
        # 接收响应
        response = sock.recv(1024).decode('utf-8')
        print(f"设备响应: {response.strip()}")
        
        # 解析响应
        if "TEMP:" in response:
            # 解析温度数据
            parts = response.strip().split(',')
            for part in parts:
                if ':' in part:
                    key, value = part.split(':', 1)
                    print(f"  {key}: {value}")
        
        sock.close()
        print("✅ GET_STATUS测试完成")
        
    except Exception as e:
        print(f"❌ 测试失败: {e}")

def test_multiple_commands():
    """测试多个命令以验证设备响应"""
    print("\n=== 测试多个命令 ===")
    
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, device_port))
        
        # 接收欢迎消息
        welcome = sock.recv(1024).decode('utf-8')
        print(f"设备欢迎消息: {welcome.strip()}")
        
        # 测试GET_STATUS
        sock.send("GET_STATUS\n".encode('utf-8'))
        response1 = sock.recv(1024).decode('utf-8')
        print(f"GET_STATUS响应: {response1.strip()}")
        
        time.sleep(1)
        
        # 测试GET_TEMP
        sock.send("GET_TEMP\n".encode('utf-8'))
        response2 = sock.recv(1024).decode('utf-8')
        print(f"GET_TEMP响应: {response2.strip()}")
        
        time.sleep(1)
        
        # 再次测试GET_STATUS
        sock.send("GET_STATUS\n".encode('utf-8'))
        response3 = sock.recv(1024).decode('utf-8')
        print(f"第二次GET_STATUS响应: {response3.strip()}")
        
        sock.close()
        print("✅ 多命令测试完成")
        
    except Exception as e:
        print(f"❌ 多命令测试失败: {e}")

if __name__ == "__main__":
    test_get_status()
    test_multiple_commands()