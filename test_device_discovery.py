#!/usr/bin/env python3
"""
测试设备发现协议脚本
验证APP和设备之间的发现协议是否正常工作
"""

import socket
import time
import json

def test_device_discovery():
    """测试设备发现协议"""
    print("=== 设备发现协议测试 ===")
    
    # 创建UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(5)  # 5秒超时
    
    # 发送发现请求
    discovery_message = "DISCOVER_SMARTOVEN"
    broadcast_address = "255.255.255.255"
    discovery_port = 8888
    
    print(f"发送发现请求到 {broadcast_address}:{discovery_port}")
    print(f"消息内容: {discovery_message}")
    
    try:
        sock.sendto(discovery_message.encode(), (broadcast_address, discovery_port))
        print("发现请求发送成功")
        
        # 监听响应
        print("等待设备响应...")
        start_time = time.time()
        
        while time.time() - start_time < 10:  # 监听10秒
            try:
                data, addr = sock.recvfrom(1024)
                response = data.decode('utf-8')
                
                print(f"\n=== 收到设备响应 ===")
                print(f"来源IP: {addr[0]}")
                print(f"来源端口: {addr[1]}")
                print(f"响应内容: {response}")
                
                # 解析设备信息
                if response.startswith("DEVICE_INFO:"):
                    print("\n=== 解析设备信息 ===")
                    info_str = response.replace("DEVICE_INFO:", "")
                    info_parts = info_str.split(",")
                    
                    device_info = {}
                    for part in info_parts:
                        if ":" in part:
                            key, value = part.split(":", 1)
                            device_info[key] = value
                            print(f"{key}: {value}")
                    
                    # 检查实际设备IP
                    actual_ip = addr[0]
                    print(f"\n实际设备IP: {actual_ip}")
                    
                    if actual_ip == "192.168.16.104":
                        print("✅ 设备发现成功！APP应该能正确识别设备IP")
                    else:
                        print(f"⚠️  设备IP不匹配，实际IP: {actual_ip}")
                        
                elif response.startswith("SMARTOVEN:"):
                    print("检测到旧版协议格式")
                    
            except socket.timeout:
                continue
                
    except Exception as e:
        print(f"测试失败: {e}")
    
    finally:
        sock.close()
    
    print("\n=== 测试完成 ===")

def test_tcp_connection(ip="192.168.16.104", port=8888):
    """测试TCP连接"""
    print(f"\n=== TCP连接测试 ===")
    print(f"目标设备: {ip}:{port}")
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        
        print("尝试连接...")
        sock.connect((ip, port))
        print("✅ TCP连接成功")
        
        # 发送状态查询命令
        status_command = "GET_STATUS"
        print(f"发送命令: {status_command}")
        sock.send(status_command.encode())
        
        # 接收响应
        response = sock.recv(1024).decode('utf-8')
        print(f"设备响应: {response}")
        
        sock.close()
        
    except Exception as e:
        print(f"❌ TCP连接失败: {e}")

if __name__ == "__main__":
    # 测试设备发现协议
    test_device_discovery()
    
    # 测试TCP连接
    test_tcp_connection()
    
    print("\n=== 测试总结 ===")
    print("如果设备发现测试成功，APP应该能自动发现设备")
    print("如果TCP连接测试成功，APP应该能正常获取温度数据")
    print("如果两个测试都失败，请检查设备网络连接和配置")