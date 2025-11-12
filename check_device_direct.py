import serial
import time
import json

try:
    # 尝试连接到设备串口
    ser = serial.Serial('COM11', 115200, timeout=2)
    print("串口连接成功")
    time.sleep(2)
    
    # 清空输入缓冲区
    ser.reset_input_buffer()
    
    # 发送状态命令
    ser.write(b'status\n')
    time.sleep(1)
    
    # 读取响应
    response = ser.read_all()
    if response:
        print("设备状态响应:")
        try:
            # 尝试解析JSON响应
            response_str = response.decode('utf-8', errors='ignore')
            print(response_str)
        except:
            print(response.decode('utf-8', errors='ignore'))
    else:
        print("设备无响应")
    
    # 关闭串口
    ser.close()
    
except Exception as e:
    print(f"连接设备时出错: {e}")