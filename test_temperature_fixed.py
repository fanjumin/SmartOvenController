#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
修复后的温度读取测试脚本
验证SPI时序优化和重试机制的效果
"""

import serial
import time
import sys

def test_temperature_fixed():
    """测试修复后的温度读取功能"""
    
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
    
    print("\n🔧 测试修复后的温度读取功能...")
    print("=" * 60)
    
    # 测试连续读取稳定性
    success_count = 0
    total_tests = 10
    
    for i in range(total_tests):
        print(f"\n📊 第{i+1}次测试...")
        
        # 发送GET_RAW_TEMP命令
        ser.write(b'GET_RAW_TEMP\n')
        time.sleep(0.5)
        
        # 读取响应
        response = ser.read_all().decode('utf-8', errors='ignore')
        
        # 分析响应
        if "原始值: 0x0" in response or "温度位: 0" in response:
            print("❌ 读取失败: 返回0值")
        elif "错误标志位" in response and "1" in response:
            print("❌ 读取失败: 传感器错误")
        elif "原始值: 0x" in response and "温度位:" in response:
            print("✅ 读取成功")
            
            # 提取温度数据
            lines = response.split('\n')
            for line in lines:
                if "原始值:" in line:
                    print(f"   {line.strip()}")
                elif "温度位:" in line:
                    print(f"   {line.strip()}")
                elif "未校准温度:" in line:
                    print(f"   {line.strip()}")
                elif "校准后温度:" in line:
                    print(f"   {line.strip()}")
            
            success_count += 1
        else:
            print("❓ 未知响应格式")
            print(f"响应内容: {response}")
        
        time.sleep(0.5)  # 测试间隔
    
    # 统计结果
    print("\n" + "=" * 60)
    print("📈 测试结果统计:")
    print(f"总测试次数: {total_tests}")
    print(f"成功次数: {success_count}")
    print(f"成功率: {success_count/total_tests*100:.1f}%")
    
    if success_count / total_tests >= 0.8:
        print("✅ 温度读取稳定性良好")
    elif success_count / total_tests >= 0.5:
        print("⚠️ 温度读取稳定性一般，建议检查硬件连接")
    else:
        print("❌ 温度读取稳定性差，需要进一步调试")
    
    # 测试GET_TEMP命令
    print("\n🌡️ 测试GET_TEMP命令...")
    ser.write(b'GET_TEMP\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"温度响应: {response.strip()}")
    
    # 测试GET_STATUS命令
    print("\n📊 测试GET_STATUS命令...")
    ser.write(b'GET_STATUS\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"状态响应: {response.strip()}")
    
    ser.close()
    print("\n✅ 测试完成")

if __name__ == "__main__":
    test_temperature_fixed()