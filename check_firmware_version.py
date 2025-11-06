#!/usr/bin/env python3
"""
检查设备固件版本和功能支持情况
"""

import socket
import time

def check_firmware_features():
    """检查固件功能支持"""
    print("=== 固件功能检查 ===")
    
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, device_port))
        
        # 接收欢迎消息
        welcome = sock.recv(1024).decode('utf-8')
        print(f"设备信息: {welcome.strip()}")
        
        # 测试各种命令支持
        commands_to_test = [
            "GET_PERFORMANCE",
            "RESET_PERFORMANCE", 
            "GET_STATUS",
            "GET_TEMP",
            "PING"
        ]
        
        for command in commands_to_test:
            print(f"\n测试命令: {command}")
            sock.send(f"{command}\n".encode('utf-8'))
            
            try:
                response = sock.recv(1024).decode('utf-8')
                print(f"响应: {response.strip()}")
                
                # 分析响应内容
                if "未知命令" in response:
                    print("❌ 命令不支持")
                elif "温度读取性能" in response or "内存警告" in response:
                    print("✅ 性能监控功能已启用")
                elif "PONG" in response:
                    print("✅ PING命令正常")
                elif "TEMP:" in response:
                    print("✅ 温度数据正常")
                    
            except socket.timeout:
                print("❌ 命令超时")
            
            time.sleep(0.5)
        
        sock.close()
        
    except Exception as e:
        print(f"❌ 固件检查失败: {e}")

def test_web_server():
    """测试Web服务器响应"""
    print("\n=== Web服务器测试 ===")
    
    import urllib.request
    import urllib.error
    
    device_ip = "192.168.16.104"
    
    # 测试状态页面
    try:
        response = urllib.request.urlopen(f"http://{device_ip}/status", timeout=5)
        data = response.read().decode('utf-8')
        print(f"状态页面响应: {data.strip()}")
    except Exception as e:
        print(f"状态页面错误: {e}")
    
    # 测试主页
    try:
        response = urllib.request.urlopen(f"http://{device_ip}/", timeout=5)
        print("主页访问正常")
    except Exception as e:
        print(f"主页访问错误: {e}")

if __name__ == "__main__":
    check_firmware_features()
    test_web_server()