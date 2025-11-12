#!/usr/bin/env python3

import http.server
import socketserver
import json
import os
import urllib.parse
from http import HTTPStatus
import mimetypes

# Store the original directory
original_dir = os.getcwd()
data_dir = os.path.join(original_dir, 'data')

class CustomHTTPRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        print(f"Received GET request for path: {self.path}")
        
        if self.path == '/status':
            print("Handling /status request")
            # 返回设备状态信息
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            # 模拟设备状态数据
            status_data = {
                "device": "SmartOven",
                "status": "idle",
                "temperature": 25,
                "target_temperature": 0,
                "timer": 0,
                "mode": "normal"
            }
            
            self.wfile.write(json.dumps(status_data).encode())
        elif self.path == '/scanwifi':
            print("Handling /scanwifi request")
            # 返回WiFi扫描结果
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            # 模拟WiFi网络数据
            wifi_data = {
                "networks": [
                    {"ssid": "Home_WiFi_5G", "rssi": -45},
                    {"ssid": "TP-LINK_2.4G", "rssi": -55},
                    {"ssid": "ChinaNet-XXXX", "rssi": -65},
                    {"ssid": "CMCC-XXXX", "rssi": -70},
                    {"ssid": "Guest_WiFi", "rssi": -75}
                ]
            }
            
            self.wfile.write(json.dumps(wifi_data).encode())
        else:
            print(f"Handling static file request for path: {self.path}")
            # 处理静态文件请求
            self.serve_static_file()
    
    def serve_static_file(self):
        # 解析请求路径
        parsed_path = urllib.parse.unquote(self.path)
        if parsed_path == '/':
            parsed_path = '/index.html'
        
        # 构建文件路径
        file_path = os.path.join(data_dir, parsed_path.lstrip('/'))
        print(f"Looking for file: {file_path}")
        
        # 检查文件是否存在且在data目录内
        if os.path.exists(file_path) and os.path.isfile(file_path) and file_path.startswith(data_dir):
            try:
                # 确定内容类型
                content_type, _ = mimetypes.guess_type(file_path)
                if content_type is None:
                    content_type = 'application/octet-stream'
                
                # 读取文件内容
                with open(file_path, 'rb') as f:
                    content = f.read()
                
                # 发送响应
                self.send_response(200)
                self.send_header('Content-type', content_type)
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(content)
            except Exception as e:
                print(f"Error reading file: {e}")
                self.send_error(HTTPStatus.INTERNAL_SERVER_ERROR, f"Error reading file: {str(e)}")
        else:
            print(f"File not found or outside data directory: {file_path}")
            self.send_error(HTTPStatus.NOT_FOUND, "File not found")
    
    def do_POST(self):
        print(f"Received POST request for path: {self.path}")
        
        # Handle POST requests for OTA updates
        if self.path == '/ota_update':
            self.handle_ota_update()
        elif self.path == '/control':
            self.handle_control()
        else:
            # For other POST requests, return 404
            self.send_error(HTTPStatus.NOT_FOUND, "POST request not supported for this endpoint")
    
    def handle_control(self):
        """Handle control POST requests"""
        try:
            # Get content length
            content_length = int(self.headers['Content-Length'])
            
            # Read the raw POST data
            post_data = self.rfile.read(content_length)
            
            # Parse JSON data
            try:
                control_data = json.loads(post_data.decode('utf-8'))
                print(f"Control data received: {control_data}")
                
                # Process control commands
                response_data = {
                    "status": "success",
                    "message": "Control command received"
                }
                
                # Here you would normally send the command to the actual device
                # For now, we'll just simulate a successful response
                
                # Send success response
                self.send_response(HTTPStatus.OK)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                
                self.wfile.write(json.dumps(response_data).encode('utf-8'))
            except json.JSONDecodeError:
                # Send error response for invalid JSON
                self.send_response(HTTPStatus.BAD_REQUEST)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                
                response = {
                    'success': False,
                    'message': 'Invalid JSON data'
                }
                
                self.wfile.write(json.dumps(response).encode('utf-8'))
        except Exception as e:
            # Handle any unexpected errors
            self.send_response(HTTPStatus.INTERNAL_SERVER_ERROR)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            response = {
                'success': False,
                'message': f'Error processing control command: {str(e)}'
            }
            
            self.wfile.write(json.dumps(response).encode('utf-8'))
    
    def handle_ota_update(self):
        """Handle OTA update POST requests with improved multipart parsing"""
        try:
            # Get content length
            content_length = int(self.headers['Content-Length'])
            
            # Read the raw POST data
            post_data = self.rfile.read(content_length)
            
            # Extract firmware data from multipart form
            content_type = self.headers['Content-Type']
            if 'boundary=' in content_type:
                boundary = content_type.split('boundary=')[1].encode()
                
                # Parse multipart data
                parts = post_data.split(b'--' + boundary)
                firmware_found = False
                
                for part in parts:
                    # Check if this part contains the firmware file
                    if b'filename="' in part and b'name="firmware"' in part:
                        # Extract file data (everything after the headers)
                        header_end = part.find(b'\r\n\r\n')
                        if header_end != -1:
                            file_data = part[header_end + 4:]
                            
                            # Remove trailing boundary marker if present
                            if file_data.endswith(b'\r\n'):
                                file_data = file_data[:-2]
                            
                            # Save the file in the data directory
                            firmware_path = os.path.join(data_dir, 'firmware.bin')
                            with open(firmware_path, 'wb') as f:
                                f.write(file_data)
                            
                            firmware_found = True
                            
                            # Send success response
                            self.send_response(HTTPStatus.OK)
                            self.send_header('Content-type', 'application/json')
                            self.send_header('Access-Control-Allow-Origin', '*')
                            self.end_headers()
                            
                            # Return success response
                            response = {
                                'success': True,
                                'message': f'OTA update request received successfully. File saved to {firmware_path}',
                                'file_size': len(file_data)
                            }
                            
                            self.wfile.write(json.dumps(response).encode('utf-8'))
                            return
                
                if not firmware_found:
                    # If we get here, no firmware file was found
                    self.send_response(HTTPStatus.BAD_REQUEST)
                    self.send_header('Content-type', 'application/json')
                    self.send_header('Access-Control-Allow-Origin', '*')
                    self.end_headers()
                    
                    # Return error response
                    response = {
                        'success': False,
                        'message': 'No firmware file provided in the request'
                    }
                    
                    self.wfile.write(json.dumps(response).encode('utf-8'))
            else:
                # Send error response for unsupported content type
                self.send_response(HTTPStatus.BAD_REQUEST)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                
                # Return error response
                response = {
                    'success': False,
                    'message': f'Unsupported content type: {content_type}'
                }
                
                self.wfile.write(json.dumps(response).encode('utf-8'))
        except Exception as e:
            # Handle any unexpected errors
            self.send_response(HTTPStatus.INTERNAL_SERVER_ERROR)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            response = {
                'success': False,
                'message': f'Error processing OTA update: {str(e)}'
            }
            
            self.wfile.write(json.dumps(response).encode('utf-8'))
    
    def do_OPTIONS(self):
        print(f"Received OPTIONS request for path: {self.path}")
        
        # Handle CORS preflight requests
        self.send_response(HTTPStatus.OK)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

if __name__ == '__main__':
    PORT = 8000
    
    with socketserver.TCPServer(("", PORT), CustomHTTPRequestHandler) as httpd:
        print(f"Server running at http://localhost:{PORT}/")
        httpd.serve_forever()