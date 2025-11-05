#!/usr/bin/env python3
import serial
import time
import sys

def check_device_status():
    try:
        # 打开串口
        ser = serial.Serial('COM11', 115200, timeout=5)
        print("成功连接到COM11端口")
        print("正在读取设备状态...")
        print("-" * 50)
        
        # 读取设备输出
        start_time = time.time()
        reboot_count = 0
        last_reboot_time = start_time
        temperature_readings = []
        
        while time.time() - start_time < 30:  # 监控30秒
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"设备输出: {line}")
                    
                    # 检查是否重启
                    if "智能电烤箱控制器" in line or "MAX6675" in line or "WiFi" in line:
                        current_time = time.time()
                        if current_time - last_reboot_time < 10:  # 10秒内再次看到启动信息
                            reboot_count += 1
                            print(f"⚠️  检测到设备重启 (第{reboot_count}次)")
                        last_reboot_time = current_time
                    
                    # 检查温度数据
                    if "°C" in line or "温度" in line:
                        temperature_readings.append(line)
                        print(f"🌡️  温度数据: {line}")
            
            time.sleep(0.1)
        
        ser.close()
        
        print("-" * 50)
        print("\n📊 设备状态分析报告:")
        print(f"1. 重启次数: {reboot_count} 次")
        print(f"2. 温度读数数量: {len(temperature_readings)} 个")
        
        if reboot_count > 2:
            print("❌ 设备频繁重启 - 可能存在硬件或软件问题")
        else:
            print("✅ 设备启动稳定")
            
        if len(temperature_readings) > 0:
            print("✅ 温度传感器工作正常")
            print(f"   最近温度读数: {temperature_readings[-1] if temperature_readings else '无'}")
        else:
            print("❌ 未检测到温度数据 - 传感器可能有问题")
            
    except serial.SerialException as e:
        print(f"❌ 无法打开串口: {e}")
        print("请检查:")
        print("1. COM11端口是否被其他程序占用")
        print("2. 设备是否正确连接")
        print("3. 串口驱动是否正常安装")
    except Exception as e:
        print(f"❌ 发生错误: {e}")

if __name__ == "__main__":
    check_device_status()