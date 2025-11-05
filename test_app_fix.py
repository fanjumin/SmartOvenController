#!/usr/bin/env python3
"""
测试APP端口修复后的连接情况
验证设备在8888端口是否正确响应GET_STATUS命令
"""

import socket
import time

def test_device_connection(device_ip="192.168.16.104", device_port=8888):
    """测试设备连接和GET_STATUS命令响应"""
    print(f"测试设备连接: {device_ip}:{device_port}")
    
    try:
        # 创建TCP连接
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)  # 5秒超时
        
        print(f"正在连接到设备 {device_ip}:{device_port}...")
        sock.connect((device_ip, device_port))
        print("✓ 连接成功")
        
        # 发送GET_STATUS命令
        command = "GET_STATUS\n"
        print(f"发送命令: {command.strip()}")
        sock.send(command.encode())
        
        # 接收响应
        response = sock.recv(1024).decode().strip()
        print(f"✓ 收到响应: {response}")
        
        # 解析温度数据
        if "TEMP:" in response:
            # 解析响应格式: TEMP:16.98,TARGET:180.00,HEAT:0,MODE:1,UPTIME:1518
            parts = response.split(",")
            for part in parts:
                if part.startswith("TEMP:"):
                    temp = part.split(":")[1]
                    print(f"✓ 当前温度: {temp}°C")
                    break
        
        sock.close()
        return True
        
    except socket.timeout:
        print("✗ 连接超时")
        return False
    except ConnectionRefusedError:
        print("✗ 连接被拒绝 - 端口可能不正确")
        return False
    except Exception as e:
        print(f"✗ 连接失败: {e}")
        return False

def test_both_ports():
    """测试两个端口，验证哪个端口可用"""
    print("=" * 50)
    print("测试端口8888 (正确端口):")
    print("-" * 30)
    result_8888 = test_device_connection(device_port=8888)
    
    print("\n" + "=" * 50)
    print("测试端口8080 (错误端口):")
    print("-" * 30)
    result_8080 = test_device_connection(device_port=8080)
    
    print("\n" + "=" * 50)
    print("测试结果总结:")
    print(f"端口8888: {'✓ 可用' if result_8888 else '✗ 不可用'}")
    print(f"端口8080: {'✓ 可用' if result_8080 else '✗ 不可用'}")
    
    if result_8888 and not result_8080:
        print("\n✓ 确认: 设备确实在8888端口监听，8080端口不可用")
        print("✓ APP端口修复应该能解决问题")
    else:
        print("\n⚠ 注意: 端口测试结果与预期不符，可能需要进一步检查")

if __name__ == "__main__":
    print("智能烤箱APP端口修复验证测试")
    print("=" * 50)
    test_both_ports()