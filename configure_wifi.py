import serial
import time

def configure_wifi():
    try:
        # 连接到设备串口
        ser = serial.Serial('COM11', 115200, timeout=2)
        print("串口连接成功")
        time.sleep(2)
        
        # 清空输入缓冲区
        ser.reset_input_buffer()
        
        # 发送WiFi配置命令 (请根据实际情况修改WiFi名称和密码)
        wifi_ssid = "SmartOvenWiFi"  # 替换为您的WiFi名称
        wifi_password = "12345678"  # 替换为您的WiFi密码
        
        print(f"正在配置WiFi: {wifi_ssid}")
        
        # 发送WiFi配置命令
        command = f"setwifi {wifi_ssid} {wifi_password}\n"
        ser.write(command.encode())
        time.sleep(1)
        
        # 读取响应
        response = ser.read_all()
        if response:
            print("设备响应:")
            print(response.decode('utf-8', errors='ignore'))
        else:
            print("设备无响应")
            
        # 发送保存配置命令
        ser.write(b"save\n")
        time.sleep(1)
        
        # 读取响应
        response = ser.read_all()
        if response:
            print("保存配置响应:")
            print(response.decode('utf-8', errors='ignore'))
        else:
            print("保存配置无响应")
        
        # 发送重启命令
        ser.write(b"reset\n")
        time.sleep(1)
        
        # 读取响应
        response = ser.read_all()
        if response:
            print("重启响应:")
            print(response.decode('utf-8', errors='ignore'))
        else:
            print("重启无响应")
        
        # 关闭串口
        ser.close()
        print("WiFi配置完成，请等待设备重启并连接到WiFi网络")
        
    except Exception as e:
        print(f"配置WiFi时出错: {e}")

if __name__ == "__main__":
    configure_wifi()