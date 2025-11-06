#!/usr/bin/env python3
"""
测试温度读取的Python脚本
通过串口发送命令并读取温度数据
"""

import serial
import time
import sys

def test_temperature_reading():
    """测试温度读取功能"""
    try:
        # 配置串口参数
        ser = serial.Serial(
            port='COM11',
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=2
        )
        
        print("连接到COM11端口...")
        time.sleep(2)  # 等待串口连接稳定
        
        # 先读取一下当前串口缓冲区的内容
        print("清空串口缓冲区...")
        while ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            print("缓冲区内容:", data.decode('utf-8', errors='ignore'))
        
        # 发送GET_TEMP命令
        print("\n发送GET_TEMP命令...")
        ser.write(b'GET_TEMP\r\n')
        time.sleep(1)
        
        # 读取响应
        response = b''
        start_time = time.time()
        while time.time() - start_time < 5:  # 5秒超时
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
                time.sleep(0.1)
        
        if response:
            print("温度响应:")
            print(response.decode('utf-8', errors='ignore'))
        else:
            print("未收到响应")
        
        # 发送GET_STATUS命令获取更多信息
        print("\n发送GET_STATUS命令...")
        ser.write(b'GET_STATUS\r\n')
        time.sleep(1)
        
        response = b''
        start_time = time.time()
        while time.time() - start_time < 5:
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
                time.sleep(0.1)
        
        if response:
            print("状态响应:")
            print(response.decode('utf-8', errors='ignore'))
        
        # 关闭串口
        ser.close()
        print("\n测试完成！")
        
    except serial.SerialException as e:
        print(f"串口错误: {e}")
        print("请确保:")
        print("1. 串口监视器已关闭")
        print("2. COM11端口存在且可用")
        print("3. 设备已正确连接")
    except Exception as e:
        print(f"错误: {e}")

if __name__ == "__main__":
    test_temperature_reading()