#!/usr/bin/env python3
"""
简单的串口测试脚本
"""

import serial
import time

def test_serial():
    port = "COM11"
    baud_rate = 115200

    print("正在测试串口连接...")
    print(f"端口: {port}, 波特率: {baud_rate}")

    try:
        ser = serial.Serial(port, baud_rate, timeout=2)
        time.sleep(1)  # 等待串口初始化

        print("串口已连接")

        # 发送状态查询命令
        print("发送状态查询命令...")
        ser.write(b"status\n")
        ser.flush()

        # 等待响应
        time.sleep(1)

        # 读取响应
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            print("设备响应:")
            print(response)
        else:
            print("设备无响应")

        ser.close()
        print("串口连接已关闭")

    except serial.SerialException as e:
        print(f"串口连接失败: {e}")
    except Exception as e:
        print(f"测试出错: {e}")

if __name__ == "__main__":
    test_serial()