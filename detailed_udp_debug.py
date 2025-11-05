#!/usr/bin/env python3
"""
详细的UDP通信调试脚本
用于诊断设备发现协议的具体问题
"""

import socket
import time
import sys

def detailed_udp_debug():
    """详细的UDP通信调试"""
    print("=== 详细UDP通信调试 ===")
    
    device_ip = "192.168.16.104"
    discovery_port = 8888
    
    # 测试1: 检查设备UDP端口是否真的开放
    print("\n=== 测试1: UDP端口连接测试 ===")
    try:
        test_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        test_sock.settimeout(3)
        test_sock.connect((device_ip, discovery_port))
        print("✅ UDP端口8888连接成功")
        test_sock.close()
    except Exception as e:
        print(f"❌ UDP端口8888连接失败: {e}")
    
    # 测试2: 发送UDP数据包并检查是否被接收
    print("\n=== 测试2: UDP数据包发送测试 ===")
    
    # 创建发送socket
    send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    send_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    # 创建接收socket（绑定到特定端口）
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.settimeout(5)
    recv_port = 8889  # 使用设备期望的响应端口
    recv_sock.bind(('', recv_port))
    
    print(f"接收socket绑定到端口: {recv_port}")
    
    # 发送发现请求
    discovery_message = "DISCOVER_SMARTOVEN"
    
    # 发送到设备IP
    print(f"发送发现请求到 {device_ip}:{discovery_port}")
    send_sock.sendto(discovery_message.encode(), (device_ip, discovery_port))
    print("✅ 直接发送成功")
    
    # 发送广播
    print(f"发送发现请求到 255.255.255.255:{discovery_port}")
    send_sock.sendto(discovery_message.encode(), ("255.255.255.255", discovery_port))
    print("✅ 广播发送成功")
    
    # 监听响应
    print(f"等待设备响应（端口{recv_port}）...")
    start_time = time.time()
    
    while time.time() - start_time < 10:
        try:
            data, addr = recv_sock.recvfrom(1024)
            response = data.decode('utf-8')
            
            print(f"\n✅ 收到设备响应！")
            print(f"来源IP: {addr[0]}")
            print(f"来源端口: {addr[1]}")
            print(f"响应内容: {response}")
            
            # 检查响应格式
            if "DEVICE_INFO" in response:
                print("✅ 响应格式正确（包含DEVICE_INFO）")
            else:
                print("⚠️ 响应格式可能不正确")
            
            send_sock.close()
            recv_sock.close()
            return True
            
        except socket.timeout:
            print("❌ 接收超时，未收到响应")
            break
        except Exception as e:
            print(f"❌ 接收错误: {e}")
            break
    
    send_sock.close()
    recv_sock.close()
    
    # 测试3: 尝试不同的接收端口
    print("\n=== 测试3: 多端口监听测试 ===")
    
    ports_to_try = [8888, 8889, 8890, 8891, 8892]
    
    for port in ports_to_try:
        print(f"\n尝试监听端口 {port}...")
        
        recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        recv_sock.settimeout(3)
        
        try:
            recv_sock.bind(('', port))
            
            # 发送请求
            send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            send_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            send_sock.sendto(discovery_message.encode(), ("255.255.255.255", discovery_port))
            
            # 监听响应
            try:
                data, addr = recv_sock.recvfrom(1024)
                response = data.decode('utf-8')
                
                print(f"✅ 在端口 {port} 收到响应！")
                print(f"响应内容: {response}")
                
                send_sock.close()
                recv_sock.close()
                return True
                
            except socket.timeout:
                print(f"❌ 端口 {port} 未收到响应")
            except Exception as e:
                print(f"❌ 端口 {port} 接收错误: {e}")
            
            send_sock.close()
            recv_sock.close()
            
        except Exception as e:
            print(f"❌ 无法绑定到端口 {port}: {e}")
    
    return False

def test_tcp_connection():
    """测试TCP连接"""
    print("\n=== TCP连接测试 ===")
    
    device_ip = "192.168.16.104"
    tcp_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, tcp_port))
        print("✅ TCP连接成功")
        
        # 发送状态查询命令
        command = "GET_STATUS\n"
        sock.send(command.encode())
        
        # 接收响应
        response = sock.recv(1024).decode('utf-8')
        print(f"设备响应: {response}")
        
        sock.close()
        return True
        
    except Exception as e:
        print(f"❌ TCP连接失败: {e}")
        return False

if __name__ == "__main__":
    print("智能电烤箱UDP通信详细调试工具")
    print("=" * 50)
    
    # 运行详细调试
    udp_success = detailed_udp_debug()
    tcp_success = test_tcp_connection()
    
    print("\n=== 调试总结 ===")
    print(f"UDP通信: {'✅ 正常' if udp_success else '❌ 异常'}")
    print(f"TCP连接: {'✅ 正常' if tcp_success else '❌ 异常'}")
    
    if not udp_success and tcp_success:
        print("\n🔍 问题分析:")
        print("设备TCP连接正常，但UDP通信失败")
        print("可能的原因:")
        print("1. 设备UDP端口监听配置问题")
        print("2. 设备没有正确处理UDP发现请求")
        print("3. 网络防火墙或路由器阻止了UDP通信")
        print("4. 设备固件中的UDP处理逻辑有问题")
    
    print("\n调试完成")