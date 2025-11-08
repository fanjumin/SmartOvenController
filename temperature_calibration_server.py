#!/usr/bin/env python3

"""
智能烤箱温度校准服务器
提供Web API接口，支持前端温度校准功能
"""

import serial
import time
import json
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import parse_qs, urlparse
import threading
import logging

# 配置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class TemperatureCalibrationHandler(BaseHTTPRequestHandler):
    """温度校准HTTP请求处理器"""
    
    def __init__(self, *args, **kwargs):
        # 串口配置
        self.serial_port = 'COM11'
        self.baud_rate = 115200
        self.serial_timeout = 2
        self.serial_connection = None
        
        super().__init__(*args, **kwargs)
    
    def connect_serial(self):
        """连接到串口设备"""
        try:
            if self.serial_connection and self.serial_connection.is_open:
                return True
                
            self.serial_connection = serial.Serial(
                port=self.serial_port,
                baudrate=self.baud_rate,
                timeout=self.serial_timeout
            )
            time.sleep(2)  # 等待串口初始化
            logger.info(f"串口连接成功: {self.serial_port}")
            return True
            
        except serial.SerialException as e:
            logger.error(f"串口连接失败: {e}")
            return False
    
    def send_serial_command(self, command):
        """发送串口命令并获取响应"""
        if not self.connect_serial():
            return None
        
        try:
            # 清空输入缓冲区
            self.serial_connection.reset_input_buffer()
            
            # 发送命令
            self.serial_connection.write((command + '\r\n').encode())
            time.sleep(1)  # 等待设备响应
            
            # 读取响应
            response = b''
            start_time = time.time()
            while time.time() - start_time < 5:  # 5秒超时
                if self.serial_connection.in_waiting > 0:
                    response += self.serial_connection.read(self.serial_connection.in_waiting)
                time.sleep(0.1)
            
            if response:
                return response.decode('utf-8', errors='ignore')
            else:
                return "无响应"
                
        except Exception as e:
            logger.error(f"串口通信错误: {e}")
            return None
    
    def do_GET(self):
        """处理GET请求"""
        parsed_path = urlparse(self.path)
        
        if parsed_path.path == '/status':
            # 获取设备状态
            self.handle_get_status()
        elif parsed_path.path == '/calibration_status':
            # 获取校准状态
            self.handle_get_calibration_status()
        else:
            self.send_error(404, "接口不存在")
    
    def do_POST(self):
        """处理POST请求"""
        parsed_path = urlparse(self.path)
        
        if parsed_path.path == '/calibrate':
            # 温度校准请求
            self.handle_calibrate()
        else:
            self.send_error(404, "接口不存在")
    
    def handle_get_status(self):
        """获取设备状态"""
        try:
            response = self.send_serial_command('GET_STATUS')
            if response:
                # 解析设备状态响应
                status_data = self.parse_status_response(response)
                self.send_json_response(200, status_data)
            else:
                self.send_json_response(500, {"error": "无法获取设备状态"})
                
        except Exception as e:
            logger.error(f"获取设备状态失败: {e}")
            self.send_json_response(500, {"error": str(e)})
    
    def handle_get_calibration_status(self):
        """获取校准状态"""
        try:
            # 获取当前温度
            temp_response = self.send_serial_command('GET_TEMP')
            
            status_data = {
                "current_temperature": self.extract_temperature(temp_response),
                "calibration_ready": True,
                "last_calibration": "未知"
            }
            
            self.send_json_response(200, status_data)
            
        except Exception as e:
            logger.error(f"获取校准状态失败: {e}")
            self.send_json_response(500, {"error": str(e)})
    
    def handle_calibrate(self):
        """处理温度校准请求"""
        try:
            # 读取POST数据
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            params = parse_qs(post_data)
            
            # 检查重置请求
            if 'reset' in params and params['reset'][0].lower() == 'true':
                # 重置校准
                response = self.send_serial_command('RESET_CALIBRATION')
                if response and "重置" in response:
                    self.send_json_response(200, {
                        "success": True,
                        "message": "温度校准已重置",
                        "offset": 0.0
                    })
                else:
                    self.send_json_response(200, {
                        "success": False,
                        "message": "重置失败，设备可能不支持此功能"
                    })
                return
            
            # 获取校准参数
            actual_temp = float(params.get('actualTemp', [0])[0])
            displayed_temp = float(params.get('displayedTemp', [0])[0])
            
            if actual_temp <= 0 or displayed_temp <= 0:
                self.send_json_response(400, {
                    "success": False,
                    "message": "无效的温度值"
                })
                return
            
            # 发送校准命令
            calibration_command = f"CALIBRATE_TEMP {actual_temp}"
            response = self.send_serial_command(calibration_command)
            
            if response and ("校准完成" in response or "Calibration" in response):
                # 计算偏移量
                offset = actual_temp - displayed_temp
                
                self.send_json_response(200, {
                    "success": True,
                    "message": "温度校准成功",
                    "offset": round(offset, 2),
                    "actual_temp": actual_temp,
                    "displayed_temp": displayed_temp
                })
                logger.info(f"温度校准成功: 实际温度={actual_temp}°C, 显示温度={displayed_temp}°C, 偏移量={offset:.2f}°C")
                
            else:
                self.send_json_response(200, {
                    "success": False,
                    "message": "校准失败，设备响应异常"
                })
                
        except Exception as e:
            logger.error(f"温度校准失败: {e}")
            self.send_json_response(500, {
                "success": False,
                "message": f"校准失败: {str(e)}"
            })
    
    def parse_status_response(self, response):
        """解析设备状态响应"""
        try:
            # 解析格式: TEMP:16.98,TARGET:180.00,HEAT:0,MODE:1,UPTIME:1518
            status_data = {}
            parts = response.strip().split(',')
            
            for part in parts:
                if ':' in part:
                    key, value = part.split(':', 1)
                    if key == 'TEMP':
                        status_data['currentTemp'] = float(value)
                    elif key == 'TARGET':
                        status_data['targetTemp'] = float(value)
                    elif key == 'HEAT':
                        status_data['heatingEnabled'] = value == '1'
                    elif key == 'MODE':
                        status_data['ovenMode'] = value == '1'
                    elif key == 'UPTIME':
                        status_data['uptime'] = int(value)
            
            return status_data
            
        except Exception as e:
            logger.error(f"解析状态响应失败: {e}")
            return {"currentTemp": 0.0, "targetTemp": 0.0, "heatingEnabled": False}
    
    def extract_temperature(self, response):
        """从响应中提取温度值"""
        try:
            # 解析格式: 当前温度: 15.03°C
            if ':' in response:
                temp_part = response.split(':')[1].strip()
                temp_value = temp_part.split('°')[0]
                return float(temp_value)
            return 0.0
        except:
            return 0.0
    
    def send_json_response(self, status_code, data):
        """发送JSON响应"""
        self.send_response(status_code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
        
        response_json = json.dumps(data, ensure_ascii=False)
        self.wfile.write(response_json.encode('utf-8'))
    
    def do_OPTIONS(self):
        """处理OPTIONS请求（CORS预检）"""
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
    
    def log_message(self, format, *args):
        """自定义日志格式"""
        logger.info(f"{self.client_address[0]} - {format % args}")

def start_calibration_server(host='localhost', port=8080):
    """启动温度校准服务器"""
    server = HTTPServer((host, port), TemperatureCalibrationHandler)
    logger.info(f"温度校准服务器启动在 http://{host}:{port}")
    logger.info("可用接口:")
    logger.info("  GET  /status - 获取设备状态")
    logger.info("  GET  /calibration_status - 获取校准状态")
    logger.info("  POST /calibrate - 执行温度校准")
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        logger.info("服务器正在关闭...")
        server.shutdown()
        server.server_close()
        logger.info("服务器已关闭")

if __name__ == "__main__":
    # 启动服务器
    start_calibration_server()