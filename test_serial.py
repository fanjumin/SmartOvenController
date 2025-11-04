#!/usr/bin/env python3
import serial
import time

# 串口配置
SERIAL_PORT = 'COM11'
BAUD_RATE = 115200

def send_serial_command(command):
    try:
        # 创建串口连接
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(0.5)  # 等待串口初始化
        
        # 发送命令
        print(f"发送命令: {command}")
        ser.write((command + '\n').encode('utf-8'))
        
        # 读取响应
        time.sleep(0.5)
        response = ser.read_all().decode('utf-8', errors='ignore')
        if response:
            print("设备响应:")
            print(response)
        else:
            print("无响应")
            
        ser.close()
        
    except Exception as e:
        print(f"错误: {e}")

# 测试各种命令
commands = [
    "GET_STATUS",
    "BEEP",
    "LED_BLINK",
    "GET_TEMP"
]

for cmd in commands:
    print("\n" + "="*50)
    send_serial_command(cmd)
    time.sleep(2)  # 等待设备处理