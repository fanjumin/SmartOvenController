#!/usr/bin/env python3
"""
检查设备性能统计信息
包括内存占用、温度读取性能等
"""

import socket
import time

def get_performance_stats():
    """获取设备性能统计信息"""
    print("=== 设备性能统计检查 ===")
    
    # 设备IP和端口
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        # 创建TCP连接
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((device_ip, device_port))
        
        # 接收欢迎消息
        welcome = sock.recv(1024).decode('utf-8')
        print(f"设备欢迎消息: {welcome.strip()}")
        
        # 发送GET_PERFORMANCE命令
        command = "GET_PERFORMANCE\n"
        sock.send(command.encode('utf-8'))
        print(f"发送命令: {command.strip()}")
        
        # 接收性能统计信息
        performance_data = ""
        start_time = time.time()
        
        while time.time() - start_time < 5:  # 最多等待5秒
            try:
                data = sock.recv(1024).decode('utf-8')
                if data:
                    performance_data += data
                    # 检查是否收到完整响应
                    if "温度读取性能" in performance_data or "内存警告" in performance_data:
                        break
            except socket.timeout:
                break
        
        print("性能统计信息:")
        print(performance_data)
        
        # 如果性能统计为空，尝试获取设备状态
        if not performance_data.strip():
            print("\n⚠️ 未收到性能统计信息，尝试获取设备状态...")
            sock.send("GET_STATUS\n".encode('utf-8'))
            status_response = sock.recv(1024).decode('utf-8')
            print(f"设备状态: {status_response.strip()}")
        
        sock.close()
        print("✅ 性能检查完成")
        
    except Exception as e:
        print(f"❌ 性能检查失败: {e}")

def test_temperature_response_time():
    """测试温度响应时间"""
    print("\n=== 温度响应时间测试 ===")
    
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, device_port))
        
        # 接收欢迎消息
        welcome = sock.recv(1024).decode('utf-8')
        print(f"设备连接成功")
        
        # 测试多次温度获取的响应时间
        response_times = []
        for i in range(5):
            start_time = time.time()
            sock.send("GET_TEMP\n".encode('utf-8'))
            response = sock.recv(1024).decode('utf-8')
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000  # 转换为毫秒
            response_times.append(response_time)
            
            print(f"第{i+1}次温度获取: {response.strip()} (响应时间: {response_time:.1f}ms)")
            time.sleep(0.5)  # 间隔0.5秒
        
        # 计算平均响应时间
        avg_response_time = sum(response_times) / len(response_times)
        print(f"\n📊 温度获取性能统计:")
        print(f"平均响应时间: {avg_response_time:.1f}ms")
        print(f"最快响应时间: {min(response_times):.1f}ms")
        print(f"最慢响应时间: {max(response_times):.1f}ms")
        
        sock.close()
        
    except Exception as e:
        print(f"❌ 温度响应时间测试失败: {e}")

def check_memory_usage():
    """检查内存使用情况"""
    print("\n=== 内存使用情况检查 ===")
    
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, device_port))
        
        # 接收欢迎消息
        welcome = sock.recv(1024).decode('utf-8')
        
        # 发送GET_STATUS命令获取基本信息
        sock.send("GET_STATUS\n".encode('utf-8'))
        status_response = sock.recv(1024).decode('utf-8')
        
        print("设备基本信息:")
        if "TEMP:" in status_response:
            parts = status_response.strip().split(',')
            for part in parts:
                if ':' in part:
                    key, value = part.split(':', 1)
                    print(f"  {key}: {value}")
        
        # 尝试获取性能信息
        sock.send("GET_PERFORMANCE\n".encode('utf-8'))
        time.sleep(1)  # 给设备时间处理
        
        performance_data = ""
        try:
            performance_data = sock.recv(1024).decode('utf-8')
        except:
            pass
        
        if performance_data:
            print("\n性能信息:")
            print(performance_data)
        else:
            print("\n⚠️ 未收到性能统计信息")
        
        sock.close()
        
    except Exception as e:
        print(f"❌ 内存检查失败: {e}")

if __name__ == "__main__":
    get_performance_stats()
    test_temperature_response_time()
    check_memory_usage()