import http.server
import socketserver
import json
import os
from urllib.parse import urlparse, parse_qs

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        # 处理WiFi扫描请求
        if path == '/scanwifi':
            self.handle_scan_wifi()
            return
            
        # 处理设备状态请求
        elif path == '/status':
            self.handle_device_status()
            return
            
        # 其他请求使用默认处理
        else:
            super().do_GET()
    
    def do_POST(self):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        # 处理WiFi保存请求
        if path == '/savewifi':
            self.handle_save_wifi()
            return
            
        # 其他POST请求返回404
        else:
            self.send_error(404, "POST endpoint not found")
    
    def handle_scan_wifi(self):
        # 返回模拟的WiFi网络数据
        wifi_data = {
            "networks": [
                {"ssid": "Home_WiFi_5G", "signal": -45},
                {"ssid": "TP-LINK_2.4G", "signal": -55},
                {"ssid": "ChinaNet-XXXX", "signal": -65},
                {"ssid": "CMCC-XXXX", "signal": -70},
                {"ssid": "Guest_WiFi", "signal": -75}
            ]
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(wifi_data).encode())
    
    def handle_device_status(self):
        print("Handling device status request")
        # 返回模拟的设备状态数据
        status_data = {
            "deviceName": "Smart Oven",  # This should be translated on the client side
            "firmwareVersion": "v0.8.7",
            "currentTemperature": 25,
            "targetTemperature": 180,
            "heatingTime": 0,
            "remainingTime": 0,
            "status": "standby",  # This should be translated on the client side
            "wifiConnected": True,
            "wifiSSID": "Home_WiFi_5G",
            "ipAddress": "192.168.1.100",
            "macAddress": "00:11:22:33:44:55"
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(status_data).encode())
    
    def handle_save_wifi(self):
        # 读取POST数据
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        
        # 解析表单数据
        form_data = parse_qs(post_data.decode('utf-8'))
        ssid = form_data.get('ssid', [''])[0]
        password = form_data.get('password', [''])[0]
        
        # 返回成功响应
        response = {
            "status": "success",
            "message": f"WiFi configuration saved, SSID: {ssid}"  # This should be translated on the client side
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode())

# 创建服务器
PORT = 8083
Handler = CustomHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print(f"Server started on port {PORT}")
    print(f"Access URL: http://localhost:{PORT}")
    httpd.serve_forever()