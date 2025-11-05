#!/usr/bin/env python3
"""
设备发现协议调试脚本
用于诊断设备发现协议的问题
"""

import socket
import time
import sys

def debug_device_discovery():
    """调试设备发现协议"""
    print("=== 设备发现协议调试 ===")
    
    # 创建UDP socket（绑定到任意端口，让系统分配随机端口）
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(5)
    sock.bind(('', 0))  # 绑定到任意可用端口
    
    # 测试1: 直接发送到设备IP
    print("\n=== 测试1: 直接发送到设备IP ===")
    device_ip = "192.168.16.104"
    discovery_port = 8888
    
    discovery_message = "DISCOVER_SMARTOVEN"
    
    print(f"发送发现请求到 {device_ip}:{discovery_port}")
    print(f"消息内容: {discovery_message}")
    
    try:
        sock.sendto(discovery_message.encode(), (device_ip, discovery_port))
        print("✅ 直接发送成功")
        
        # 监听响应
        print("等待设备响应...")
        start_time = time.time()
        
        while time.time() - start_time < 10:
            try:
                data, addr = sock.recvfrom(1024)
                response = data.decode('utf-8')
                
                print(f"✅ 收到设备响应")
                print(f"来源IP: {addr[0]}")
                print(f"来源端口: {addr[1]}")
                print(f"响应内容: {response}")
                
                return True
                
            except socket.timeout:
                print("❌ 直接发送测试超时，未收到响应")
                break
            except Exception as e:
                print(f"❌ 接收错误: {e}")
                break
                
    except Exception as e:
        print(f"❌ 直接发送失败: {e}")
    
    # 测试2: 广播发送
    print("\n=== 测试2: 广播发送 ===")
    broadcast_address = "255.255.255.255"
    
    print(f"发送发现请求到 {broadcast_address}:{discovery_port}")
    print(f"消息内容: {discovery_message}")
    
    try:
        sock.sendto(discovery_message.encode(), (broadcast_address, discovery_port))
        print("✅ 广播发送成功")
        
        # 监听响应
        print("等待设备响应...")
        start_time = time.time()
        
        while time.time() - start_time < 10:
            try:
                data, addr = sock.recvfrom(1024)
                response = data.decode('utf-8')
                
                print(f"✅ 收到设备响应")
                print(f"来源IP: {addr[0]}")
                print(f"来源端口: {addr[1]}")
                print(f"响应内容: {response}")
                
                return True
                
            except socket.timeout:
                print("❌ 广播发送测试超时，未收到响应")
                break
            except Exception as e:
                print(f"❌ 接收错误: {e}")
                break
                
    except Exception as e:
        print(f"❌ 广播发送失败: {e}")
    
    # 测试3: 检查UDP端口状态
    print("\n=== 测试3: UDP端口状态检查 ===")
    
    # 检查设备UDP端口是否开放
    test_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    test_sock.settimeout(3)
    
    try:
        test_sock.connect((device_ip, discovery_port))
        print("✅ UDP端口8888已开放")
    except:
        print("❌ UDP端口8888未开放")
    
    test_sock.close()
    
    # 测试4: 网络连通性检查
    print("\n=== 测试4: 网络连通性检查 ===")
    
    # 检查设备是否可达
    try:
        # 使用ping命令检查连通性
        import subprocess
        result = subprocess.run(['ping', '-n', '1', device_ip], 
                              capture_output=True, text=True, timeout=5)
        if result.returncode == 0:
            print("✅ 设备网络可达")
        else:
            print("❌ 设备网络不可达")
            print(f"Ping输出: {result.stdout}")
    except Exception as e:
        print(f"❌ 网络连通性检查失败: {e}")
    
    sock.close()
    return False

def test_tcp_connection():
    """测试TCP连接"""
    print("\n=== TCP连接测试 ===")
    
    device_ip = "192.168.16.104"
    tcp_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        
        print(f"尝试连接到 {device_ip}:{tcp_port}")
        sock.connect((device_ip, tcp_port))
        print("✅ TCP连接成功")
        
        # 发送测试命令
        command = "GET_STATUS\n"
        sock.send(command.encode())
        print(f"发送命令: {command.strip()}")
        
        # 接收响应
        response = sock.recv(1024).decode('utf-8')
        print(f"设备响应: {response}")
        
        sock.close()
        return True
        
    except Exception as e:
        print(f"❌ TCP连接失败: {e}")
        return False

if __name__ == "__main__":
    print("智能电烤箱设备发现协议调试工具")
    print("=" * 50)
    
    # 运行调试测试
    discovery_success = debug_device_discovery()
    tcp_success = test_tcp_connection()
    
    print("\n=== 调试总结 ===")
    print(f"设备发现协议: {'✅ 正常' if discovery_success else '❌ 异常'}")
    print(f"TCP连接: {'✅ 正常' if tcp_success else '❌ 异常'}")
    
    if not discovery_success and tcp_success:
        print("\n🔍 问题分析:")
        print("设备TCP连接正常，但设备发现协议失败")
        print("可能的原因:")
        print("1. 设备UDP端口监听问题")
        print("2. 设备没有正确处理发现请求")
        print("3. 网络防火墙或路由器设置阻止了UDP广播")
        print("4. 设备固件中的设备发现功能未启用")
    elif discovery_success and tcp_success:
        print("\n✅ 所有测试通过，设备发现协议工作正常")
    else:
        print("\n❌ 设备连接存在严重问题")

    print("\n调试完成")