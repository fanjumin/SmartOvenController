import socket
import struct
import time

def listen_for_device():
    # 创建UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # 允许接收广播信息
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    # 绑定到所有接口的特定端口
    sock.bind(('', 8888))  # 假设设备使用8888端口进行广播
    
    print("监听设备广播信息...")
    
    try:
        while True:
            data, addr = sock.recvfrom(1024)
            message = data.decode('utf-8')
            print(f"收到来自 {addr} 的消息: {message}")
            
            # 检查是否是我们的设备
            if "SMARTOVEN_BROADCAST" in message:
                print(f"找到设备! IP地址: {addr[0]}")
                # 解析设备信息
                parts = message.split(":")
                if len(parts) >= 4:
                    device_type = parts[1]
                    device_id = parts[2]
                    version = parts[3]
                    print(f"设备类型: {device_type}")
                    print(f"设备ID: {device_id}")
                    print(f"版本: {version}")
                return addr[0]
                
    except KeyboardInterrupt:
        print("监听已停止")
    finally:
        sock.close()

if __name__ == "__main__":
    listen_for_device()