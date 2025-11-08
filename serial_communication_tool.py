#!/usr/bin/env python3

"""
智能烤箱控制器串口通信工具

这个工具允许用户通过串口与智能烤箱控制器设备进行交互，
发送命令并接收设备响应。
"""

import serial
import serial.tools.list_ports
import time
import os

def list_available_ports():
    """列出所有可用的串口端口"""
    ports = serial.tools.list_ports.comports()
    return [(port.device, port.description) for port in ports]

def select_serial_port():
    """让用户选择串口端口"""
    ports = list_available_ports()
    
    if not ports:
        print("错误: 未发现可用的串口设备")
        return None
    
    print("可用的串口端口:")
    for i, (port, desc) in enumerate(ports, 1):
        print(f"{i}. {port} - {desc}")
    
    while True:
        try:
            choice = input(f"请选择端口 (1-{len(ports)}): ")
            if choice.lower() in ['q', 'quit', 'exit']:
                return None
            index = int(choice) - 1
            if 0 <= index < len(ports):
                return ports[index][0]
            else:
                print("无效的选择，请重新输入")
        except ValueError:
            print("请输入有效的数字")

def connect_serial(port, baud_rate=115200):
    """连接到串口"""
    try:
        ser = serial.Serial(port, baud_rate, timeout=2)
        time.sleep(1)  # 等待串口初始化
        print(f"已连接到 {port}，波特率 {baud_rate}")
        return ser
    except serial.SerialException as e:
        print(f"连接失败: {e}")
        return None

def send_command(ser, command):
    """发送命令到设备并返回响应"""
    try:
        # 清空输入缓冲区
        ser.reset_input_buffer()
        
        # 发送命令
        print(f"\n发送命令: {command}")
        ser.write((command + '\n').encode('utf-8'))
        
        # 等待设备响应
        time.sleep(0.5)
        
        # 读取响应
        response = []
        start_time = time.time()
        while time.time() - start_time < 2:  # 最多等待2秒
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    response.append(line)
            else:
                time.sleep(0.1)  # 短暂休眠避免CPU占用过高
        
        if response:
            print("设备响应:")
            for line in response:
                print(f"  {line}")
        else:
            print("设备无响应")
            
        return response
    except Exception as e:
        print(f"发送命令时出错: {e}")
        return []

def show_command_help():
    """显示命令帮助"""
    print("\n可用命令:")
    print("  GET_STATUS      - 获取设备状态信息")
    print("  GET_TEMP        - 获取当前温度")
    print("  GET_RAW_TEMP    - 获取原始温度数据")
    print("  GET_PERFORMANCE - 获取性能统计信息")
    print("  LED_ON          - 打开LED指示灯")
    print("  LED_OFF         - 关闭LED指示灯")
    print("  LED_BLINK       - LED指示灯闪烁")
    print("  BEEP            - 播放提示音")
    print("  BEEP_LONG       - 播放长提示音")
    print("  BEEP_SHORT      - 播放短提示音")
    print("  RESET_CALIBRATION - 重置温度校准")
    print("  CALIBRATE_TEMP <value> - 设置温度校准值 (例如: CALIBRATE_TEMP 15.5)")
    print("  help            - 显示此帮助信息")
    print("  clear           - 清屏")
    print("  exit/quit/q     - 退出程序")

def interactive_mode(ser):
    """交互式命令模式"""
    print("\n已进入交互式命令模式")
    print("输入 'help' 查看可用命令，输入 'exit' 退出")
    
    while True:
        try:
            command = input("\n> ").strip()
            
            if command.lower() in ['exit', 'quit', 'q']:
                break
            elif command.lower() == 'help':
                show_command_help()
            elif command.lower() == 'clear':
                os.system('cls' if os.name == 'nt' else 'clear')
            elif command:
                send_command(ser, command)
        except KeyboardInterrupt:
            print("\n\n用户中断操作")
            break

def main():
    """主函数"""
    print("="*60)
    print("智能烤箱控制器串口通信工具")
    print("="*60)
    print("此工具允许您通过串口与设备进行通信")
    print()
    
    # 获取串口端口
    port = select_serial_port()
    if not port:
        print("程序已退出")
        return
    
    # 连接到串口
    ser = connect_serial(port)
    if not ser:
        print("无法建立串口连接，程序已退出")
        return
    
    try:
        # 发送初始命令确认连接
        print("\n正在测试连接...")
        send_command(ser, "GET_STATUS")
        
        # 进入交互式模式
        interactive_mode(ser)
        
    except Exception as e:
        print(f"程序运行出错: {e}")
    finally:
        # 关闭串口连接
        if ser and ser.is_open:
            ser.close()
            print("\n已关闭串口连接")
    
    print("\n程序已退出")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n用户中断程序")
    except Exception as e:
        print(f"程序错误: {e}")
    finally:
        input("按Enter键退出...")