#!/usr/bin/env python3
import serial
import time
import sys

def check_device_status():
    try:
        print("正在尝试连接到COM11端口...")
        # 尝试连接到COM11端口
        ser = serial.Serial('COM11', 115200, timeout=1)
        print("成功连接到COM11端口")
        print("正在读取设备输出...")
        print("-" * 50)
        
        # 读取设备输出，持续10秒
        start_time = time.time()
        while time.time() - start_time < 10:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"设备输出: {line}")
            time.sleep(0.1)
            
        ser.close()
        print("-" * 50)
        print("设备状态检查完成")
        
    except serial.SerialException as e:
        print(f"无法打开串口: {e}")
        print("请检查:")
        print("1. 设备是否正确连接")
        print("2. COM11端口是否被其他程序占用")
        print("3. 串口驱动是否正常安装")
    except Exception as e:
        print(f"发生错误: {e}")

if __name__ == "__main__":
    check_device_status()