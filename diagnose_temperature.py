#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
温度传感器诊断脚本
用于诊断MAX6675温度传感器读取问题
"""

import serial
import time
import sys

def diagnose_temperature():
    """诊断温度传感器问题"""
    
    # 尝试连接串口
    try:
        ser = serial.Serial('COM11', 115200, timeout=2)
        print("✅ 串口连接成功")
    except Exception as e:
        print(f"❌ 串口连接失败: {e}")
        return
    
    # 等待设备启动
    time.sleep(2)
    
    # 清空缓冲区
    ser.reset_input_buffer()
    
    print("\n🔍 开始温度传感器诊断...")
    print("=" * 50)
    
    # 1. 获取设备状态
    print("\n1. 获取设备状态...")
    ser.write(b'GET_STATUS\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"状态响应: {response}")
    
    # 2. 获取原始温度数据
    print("\n2. 获取原始温度数据...")
    ser.write(b'GET_RAW_TEMP\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"原始温度响应:\n{response}")
    
    # 3. 检查SPI通信
    print("\n3. 检查SPI通信状态...")
    ser.write(b'GET_PERFORMANCE\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"性能信息:\n{response}")
    
    # 4. 多次读取温度数据以确认问题
    print("\n4. 连续读取温度数据（5次）...")
    for i in range(5):
        ser.write(b'GET_RAW_TEMP\n')
        time.sleep(0.3)
        response = ser.read_all().decode('utf-8', errors='ignore')
        print(f"第{i+1}次读取:\n{response}")
        time.sleep(0.2)
    
    # 5. 检查引脚状态
    print("\n5. 检查引脚配置...")
    print("MAX6675引脚配置:")
    print("  - THERMO_CLK (时钟): GPIO14 (D5)")
    print("  - THERMO_CS (片选): GPIO12 (D6)")
    print("  - THERMO_DO (数据): GPIO13 (D7)")
    print("  - 电源: 3.3V")
    print("  - 接地: GND")
    
    # 6. 诊断建议
    print("\n6. 诊断建议:")
    print("=" * 50)
    
    # 分析响应数据
    if "原始值: 0x0" in response or "温度位: 0" in response:
        print("❌ 问题确认: 温度传感器返回全0数据")
        print("\n🔧 可能的解决方案:")
        print("1. 检查MAX6675模块的电源连接（3.3V和GND）")
        print("2. 验证热电偶是否正确插入MAX6675模块")
        print("3. 检查SPI引脚连接（D5, D6, D7）")
        print("4. 确认MAX6675模块是否正常工作")
        print("5. 尝试更换热电偶或MAX6675模块")
    elif "错误标志位" in response and "1" in response:
        print("❌ 问题确认: 温度传感器通信错误")
        print("\n🔧 可能的解决方案:")
        print("1. 检查热电偶连接是否松动")
        print("2. 验证热电偶类型是否正确（K型）")
        print("3. 检查MAX6675模块是否损坏")
    else:
        print("✅ 温度传感器通信正常")
        print("\n🔧 如果仍有问题，请检查:")
        print("1. 热电偶是否接触良好")
        print("2. 环境温度是否在测量范围内")
        print("3. 校准参数是否正确")
    
    print("\n📋 硬件检查清单:")
    print("  - [ ] MAX6675模块VCC连接到3.3V")
    print("  - [ ] MAX6675模块GND连接到GND")
    print("  - [ ] CLK引脚连接到GPIO14 (D5)")
    print("  - [ ] CS引脚连接到GPIO12 (D6)")
    print("  - [ ] DO引脚连接到GPIO13 (D7)")
    print("  - [ ] 热电偶正确插入MAX6675模块")
    print("  - [ ] 热电偶探头接触良好")
    
    ser.close()
    print("\n✅ 诊断完成")

if __name__ == "__main__":
    diagnose_temperature()