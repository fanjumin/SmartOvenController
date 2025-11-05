#!/usr/bin/env python3
"""
测试设备响应脚本 - 验证设备是否发送温度数据
"""

import socket
import time
import threading

def test_device_response():
    """测试设备是否发送温度数据"""
    print("=== 设备响应测试 ===")
    
    # 设备IP和端口
    device_ip = "192.168.16.104"  # 实际连接的设备IP
    device_port = 8888
    
    try:
        # 创建Socket连接
        print(f"连接到设备 {device_ip}:{device_port}...")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)  # 10秒超时
        sock.connect((device_ip, device_port))
        print("连接成功!")
        
        # 发送GET_STATUS命令
        print("发送GET_STATUS命令...")
        sock.send(b"GET_STATUS\n")
        
        # 接收响应
        print("等待设备响应...")
        response = sock.recv(1024).decode('utf-8').strip()
        print(f"设备响应: {response}")
        
        # 解析响应
        if "TEMP:" in response:
            print("✅ 设备返回了温度数据")
            # 解析温度值
            parts = response.split(',')
            for part in parts:
                if part.startswith("TEMP:"):
                    temp_value = part.split(':')[1]
                    print(f"✅ 当前温度: {temp_value}°C")
        else:
            print("❌ 设备响应中未包含温度数据")
            
        # 测试持续接收数据
        print("\n=== 持续接收测试 ===")
        print("监听设备发送的数据（持续10秒）...")
        
        start_time = time.time()
        received_count = 0
        
        sock.settimeout(1)  # 设置1秒超时，便于循环
        
        while time.time() - start_time < 10:
            try:
                data = sock.recv(1024).decode('utf-8').strip()
                if data:
                    received_count += 1
                    print(f"接收 #{received_count}: {data}")
                    
                    # 检查是否包含温度数据
                    if "TEMP:" in data:
                        parts = data.split(',')
                        for part in parts:
                            if part.startswith("TEMP:"):
                                temp_value = part.split(':')[1]
                                print(f"✅ 温度数据: {temp_value}°C")
                                
            except socket.timeout:
                continue  # 超时继续等待
            except Exception as e:
                print(f"接收错误: {e}")
                break
        
        print(f"\n测试结果: 共收到 {received_count} 条数据")
        
        if received_count > 0:
            print("✅ 设备正在发送数据")
        else:
            print("❌ 设备未发送任何数据")
            
    except Exception as e:
        print(f"❌ 连接失败: {e}")
        print("可能的原因:")
        print("1. 设备未开机或不在同一网络")
        print("2. 设备IP地址不正确")
        print("3. 设备固件未运行")
        print("4. 防火墙或网络设置问题")
    
    finally:
        try:
            sock.close()
        except:
            pass

def test_app_parsing():
    """测试APP解析逻辑"""
    print("\n=== APP解析逻辑测试 ===")
    
    # 模拟设备响应数据
    test_responses = [
        "TEMP:16.98,TARGET:0.00,PID:0.00,HEAT:0,MODE:0,UPTIME:123",
        "OK:Command executed",
        "ERROR:Invalid command",
        "TEMP:25.50,TARGET:180.00,PID:45.67,HEAT:1,MODE:1,UPTIME:456"
    ]
    
    for response in test_responses:
        print(f"\n测试响应: {response}")
        
        # 模拟APP的parseDeviceStatus逻辑
        try:
            if "TEMP:" in response:
                parts = response.split(',')
                current_temp = 0
                target_temp = 0
                pid_output = 0
                is_heating = False
                use_pid = False
                uptime = 0
                
                for part in parts:
                    key_value = part.split(':')
                    if len(key_value) == 2:
                        key = key_value[0].strip()
                        value = key_value[1].strip()
                        
                        if key == "TEMP":
                            current_temp = float(value)
                        elif key == "TARGET":
                            target_temp = float(value)
                        elif key == "PID":
                            pid_output = float(value)
                        elif key == "HEAT":
                            is_heating = value == "1"
                        elif key == "MODE":
                            use_pid = value == "1"
                        elif key == "UPTIME":
                            uptime = int(value)
                
                print(f"✅ 解析成功: 温度={current_temp}°C, 目标={target_temp}°C, 加热={is_heating}, PID={use_pid}")
            elif response.startswith("OK:"):
                print("✅ 命令执行成功")
            elif response.startswith("ERROR:"):
                print("❌ 命令执行错误")
            else:
                print("❌ 无法识别的响应格式")
                
        except Exception as e:
            print(f"❌ 解析错误: {e}")

if __name__ == "__main__":
    print("智能烤箱设备响应测试")
    print("=" * 50)
    
    # 测试设备响应
    test_device_response()
    
    # 测试APP解析逻辑
    test_app_parsing()
    
    print("\n" + "=" * 50)
    print("测试完成")