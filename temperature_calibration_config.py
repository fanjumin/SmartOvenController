"""
温度校准配置参数
用于保存和恢复温度传感器的校准参数
"""

# 温度校准配置
CALIBRATION_CONFIG = {
    "target_temperature": 15.0,  # 目标校准温度：15°C
    "calibration_offset": 0.0,   # 校准偏移量
    "calibration_scale": 1.0,     # 校准系数
    "calibration_method": "CALIBRATE_TEMP",  # 校准方法
    "calibration_command": "CALIBRATE_TEMP 15",  # 校准命令
    "last_calibration_time": "2024-01-01 00:00:00",  # 最后校准时间
    "device_id": "oven-8591756",  # 设备ID
    "firmware_version": "0.5.0"   # 固件版本
}

def get_calibration_script():
    """生成温度校准脚本"""
    script = """
import serial
import time
import datetime

# 温度校准脚本
def calibrate_temperature():
    try:
        ser = serial.Serial('COM11', 115200, timeout=2)
        time.sleep(2)  # 等待串口初始化
        
        # 发送校准命令
        print("开始温度校准...")
        ser.write(b'CALIBRATE_TEMP 15\\r\\n')
        time.sleep(3)
        
        # 读取响应
        response = b''
        start_time = time.time()
        while time.time() - start_time < 5:  # 5秒超时
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print("校准响应:")
            print(response.decode('utf-8', errors='ignore'))
            
            # 验证校准结果
            print("\\n验证校准结果:")
            ser.write(b'GET_TEMP\\r\\n')
            time.sleep(1)
            verify_response = ser.read(ser.in_waiting)
            print("当前温度:")
            print(verify_response.decode('utf-8', errors='ignore'))
            
        else:
            print("无响应")
        
        ser.close()
        
        # 更新校准时间
        CALIBRATION_CONFIG["last_calibration_time"] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"\\n校准完成时间: {CALIBRATION_CONFIG['last_calibration_time']}")
        
    except Exception as e:
        print(f"校准失败: {e}")

if __name__ == "__main__":
    calibrate_temperature()
"""
    return script

def save_calibration_config():
    """保存校准配置到文件"""
    import json
    
    config_data = {
        "calibration_config": CALIBRATION_CONFIG,
        "calibration_script": get_calibration_script()
    }
    
    with open('temperature_calibration_backup.json', 'w', encoding='utf-8') as f:
        json.dump(config_data, f, indent=2, ensure_ascii=False)
    
    print("校准配置已保存到 temperature_calibration_backup.json")

def load_calibration_config():
    """从文件加载校准配置"""
    import json
    
    try:
        with open('temperature_calibration_backup.json', 'r', encoding='utf-8') as f:
            config_data = json.load(f)
        
        global CALIBRATION_CONFIG
        CALIBRATION_CONFIG.update(config_data.get("calibration_config", {}))
        print("校准配置已加载")
        return config_data.get("calibration_script", "")
        
    except FileNotFoundError:
        print("校准配置文件不存在")
        return ""

if __name__ == "__main__":
    # 保存当前配置
    save_calibration_config()
    
    # 显示校准信息
    print("\\n=== 温度校准配置 ===")
    for key, value in CALIBRATION_CONFIG.items():
        print(f"{key}: {value}")
    
    print("\\n使用方法:")
    print("1. 确保串口COM11可用")
    print("2. 运行: python temperature_calibration_config.py")
    print("3. 或运行生成的校准脚本")
    print("\\n校准命令: CALIBRATE_TEMP 15")