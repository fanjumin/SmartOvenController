"""
温度精确校准到15°C脚本
专门用于将设备温度传感器校准到15°C
"""

import serial
import time
import datetime
import json

def calibrate_to_15c():
    """将温度精确校准到15°C"""
    print("=== 温度精确校准到15°C ===")
    print("目标温度: 15.00°C")
    print("校准命令: CALIBRATE_TEMP 15")
    print()
    
    try:
        # 尝试连接串口
        print("正在连接串口 COM11...")
        ser = serial.Serial('COM11', 115200, timeout=2)
        time.sleep(2)  # 等待串口初始化
        print("串口连接成功")
        
        # 先获取当前温度
        print("\\n获取当前温度...")
        ser.write(b'GET_TEMP\\r\\n')
        time.sleep(1)
        current_temp_response = ser.read(ser.in_waiting)
        print("当前温度响应:")
        print(current_temp_response.decode('utf-8', errors='ignore'))
        
        # 发送校准命令
        print("\\n开始温度校准到15°C...")
        ser.write(b'CALIBRATE_TEMP 15\\r\\n')
        time.sleep(3)  # 等待校准完成
        
        # 读取校准响应
        response = b''
        start_time = time.time()
        while time.time() - start_time < 8:  # 8秒超时，确保完整响应
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print("\\n校准响应:")
            response_text = response.decode('utf-8', errors='ignore')
            print(response_text)
            
            # 检查校准是否成功
            if "校准完成" in response_text or "Calibration" in response_text:
                print("\\n✅ 温度校准成功!")
            else:
                print("\\n⚠️  校准响应异常，请检查设备状态")
        else:
            print("\\n❌ 无校准响应")
        
        # 验证校准结果
        print("\\n验证校准结果...")
        ser.write(b'GET_TEMP\\r\\n')
        time.sleep(1)
        verify_response = ser.read(ser.in_waiting)
        print("校准后温度:")
        print(verify_response.decode('utf-8', errors='ignore'))
        
        # 获取完整状态
        print("\\n获取设备状态...")
        ser.write(b'GET_STATUS\\r\\n')
        time.sleep(1)
        status_response = ser.read(ser.in_waiting)
        print("设备状态:")
        print(status_response.decode('utf-8', errors='ignore'))
        
        ser.close()
        
        # 保存校准记录
        calibration_record = {
            "target_temperature": 15.0,
            "calibration_time": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "device_id": "oven-8591756",
            "firmware_version": "0.6.0",
            "calibration_command": "CALIBRATE_TEMP 15",
            "response": response_text if response else "无响应"
        }
        
        # 保存到文件
        with open('calibration_15c_record.json', 'w', encoding='utf-8') as f:
            json.dump(calibration_record, f, indent=2, ensure_ascii=False)
        
        print(f"\\n✅ 校准记录已保存到 calibration_15c_record.json")
        print(f"📅 校准时间: {calibration_record['calibration_time']}")
        
    except serial.SerialException as e:
        print(f"❌ 串口连接失败: {e}")
        print("\\n可能的原因:")
        print("1. 串口COM11被其他程序占用")
        print("2. 设备未连接或电源未开启")
        print("3. 串口驱动程序问题")
        print("\\n解决方法:")
        print("1. 关闭其他可能占用串口的程序")
        print("2. 检查设备连接和电源")
        print("3. 稍后重试")
        
    except Exception as e:
        print(f"❌ 校准过程中发生错误: {e}")

def check_web_temperature():
    """检查Web页面温度显示"""
    import requests
    
    try:
        print("\\n检查Web页面温度显示...")
        response = requests.get('http://192.168.16.104/status', timeout=5)
        if response.status_code == 200:
            data = response.json()
            print(f"Web页面温度: {data.get('temperature', 'N/A')}°C")
            print(f"目标温度: {data.get('target_temperature', 'N/A')}°C")
            print(f"校准状态: {data.get('calibrated', 'N/A')}")
        else:
            print(f"Web页面访问失败: {response.status_code}")
    except Exception as e:
        print(f"Web页面检查失败: {e}")

if __name__ == "__main__":
    print("温度精确校准脚本 v1.0")
    print("=" * 50)
    
    # 执行校准
    calibrate_to_15c()
    
    # 检查Web页面
    check_web_temperature()
    
    print("\\n" + "=" * 50)
    print("校准脚本执行完成")
    print("\\n后续操作:")
    print("1. 如果串口被占用，请稍后重试")
    print("2. 校准成功后，温度应显示为15°C左右")
    print("3. 校准参数已自动保存到EEPROM")
    print("4. 可通过Web页面 http://192.168.16.104 查看实时温度")