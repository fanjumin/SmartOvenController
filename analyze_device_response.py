#!/usr/bin/env python3
"""
分析设备响应格式，对比APP解析逻辑
"""

import socket
import time

def test_device_discovery_format():
    """测试设备发现响应格式"""
    print("=== 设备发现响应格式分析 ===")
    
    # 创建UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(5)
    
    # 发送发现请求
    message = "DISCOVER_SMARTOVEN"
    sock.sendto(message.encode(), ('255.255.255.255', 8888))
    print(f"发送发现请求: {message}")
    
    try:
        # 接收响应
        data, addr = sock.recvfrom(1024)
        response = data.decode().strip()
        print(f"收到设备响应: {response}")
        print(f"来源IP: {addr[0]}")
        
        # 分析响应格式
        print("\n=== 响应格式分析 ===")
        print(f"响应前缀: {response[:20]}...")
        
        # 检查是否以DEVICE_INFO:开头
        if response.startswith("DEVICE_INFO:"):
            print("✅ 响应以DEVICE_INFO:开头 - 符合APP期望格式")
            
            # 提取数据部分
            data_part = response[12:]
            print(f"数据部分: {data_part}")
            
            # 检查分隔符
            if "," in data_part:
                print("✅ 使用逗号分隔符 - 符合APP期望格式")
                parts = data_part.split(",")
                print(f"字段数量: {len(parts)}")
                
                # 分析每个字段
                for i, part in enumerate(parts):
                    print(f"字段 {i+1}: {part}")
                    if ":" in part:
                        key, value = part.split(":", 1)
                        print(f"  - 键: {key}, 值: {value}")
                    else:
                        print(f"  - 格式异常: 缺少冒号分隔符")
            else:
                print("❌ 未使用逗号分隔符 - 格式不匹配")
                
        else:
            print("❌ 响应不以DEVICE_INFO:开头 - 格式不匹配")
            print("APP期望格式: DEVICE_INFO:NAME=...;MAC=...;PORT=...")
            print("实际格式可能需要调整")
            
    except socket.timeout:
        print("❌ 设备未响应发现请求")
    except Exception as e:
        print(f"❌ 错误: {e}")
    finally:
        sock.close()

def test_tcp_response_format():
    """测试TCP响应格式"""
    print("\n=== TCP响应格式分析 ===")
    
    try:
        # 连接到设备
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(('192.168.16.104', 8888))
        print("✅ TCP连接成功")
        
        # 发送状态查询命令
        sock.send(b"GET_STATUS\n")
        print("发送命令: GET_STATUS")
        
        # 接收响应
        response = sock.recv(1024).decode().strip()
        print(f"设备响应: {response}")
        
        # 分析响应格式
        print("\n=== TCP响应格式分析 ===")
        if "TEMP:" in response and "," in response:
            print("✅ 响应包含TEMP字段和逗号分隔符")
            parts = response.split(",")
            for part in parts:
                if ":" in part:
                    key, value = part.split(":", 1)
                    print(f"  - {key}: {value}")
        else:
            print("❌ TCP响应格式异常")
            
    except Exception as e:
        print(f"❌ TCP测试错误: {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    test_device_discovery_format()
    test_tcp_response_format()
    
    print("\n=== 总结 ===")
    print("1. 检查设备发现响应是否以'DEVICE_INFO:'开头")
    print("2. 检查字段分隔符是逗号还是分号")
    print("3. 检查键值对分隔符是冒号还是等号")
    print("4. 根据实际格式调整APP解析逻辑")