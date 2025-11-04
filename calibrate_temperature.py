#!/usr/bin/env python3
# 温度校准脚本
import serial
import time
import sys

def calibrate_temperature():
    try:
        # 配置串口参数
        ser = serial.Serial(
            port='COM11',
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )
        
        print("连接到COM11端口...")
        time.sleep(2)  # 等待串口连接稳定
        
        # 发送GET_STATUS命令检查当前状态
        print("检查设备状态...")
        ser.write(b'GET_STATUS\r\n')
        time.sleep(1)
        
        # 读取响应
        response = b''
        while ser.in_waiting > 0:
            response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print("设备响应:")
            print(response.decode('utf-8', errors='ignore'))
        
        # 发送GET_RAW_TEMP命令获取原始温度
        print("\n获取原始温度数据...")
        ser.write(b'GET_RAW_TEMP\r\n')
        time.sleep(1)
        
        response = b''
        while ser.in_waiting > 0:
            response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print("原始温度数据:")
            print(response.decode('utf-8', errors='ignore'))
        
        # 发送温度校准命令
        print("\n执行温度校准...")
        ser.write(b'CALIBRATE_TEMP 16.0\r\n')
        time.sleep(2)  # 给设备足够时间处理校准
        
        # 读取校准响应
        response = b''
        while ser.in_waiting > 0:
            response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print("校准响应:")
            print(response.decode('utf-8', errors='ignore'))
        
        # 验证校准效果
        print("\n验证校准效果...")
        ser.write(b'GET_STATUS\r\n')
        time.sleep(1)
        
        response = b''
        while ser.in_waiting > 0:
            response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print("校准后状态:")
            print(response.decode('utf-8', errors='ignore'))
        
        # 关闭串口
        ser.close()
        print("\n温度校准完成！")
        
    except serial.SerialException as e:
        print(f"串口错误: {e}")
        print("请确保:")
        print("1. 串口监视器已关闭")
        print("2. COM11端口存在且可用")
        print("3. 设备已正确连接")
    except Exception as e:
        print(f"错误: {e}")

if __name__ == "__main__":
    calibrate_temperature()