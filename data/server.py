import http.server
import socketserver
import json
import os
from urllib.parse import urlparse, parse_qs

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def send_head(self):
        path = self.translate_path(self.path)
        f = None
        if os.path.isdir(path):
            if not self.path.endswith('/'):
                # Redirect browser - doing basically what apache does
                self.send_response(301)
                self.send_header("Location", self.path + "/")
                self.end_headers()
                return None
            for index in "index.html", "index.htm":
                index = os.path.join(path, index)
                if os.path.exists(index):
                    path = index
                    break
            else:
                return self.list_directory(path)
        ctype = self.guess_type(path)
        try:
            f = open(path, 'rb')
        except OSError:
            self.send_error(404, "File not found")
            return None
        try:
            self.send_response(200)
            # 为HTML和JS文件添加UTF-8字符集声明
            ext = os.path.splitext(path)[1].lower()
            if ext in ('.html', '.htm', '.js') or ctype.startswith('text/html') or ctype in ['application/javascript', 'text/javascript']:
                ctype += '; charset=utf-8'
            self.send_header("Content-type", ctype)
            fs = os.fstat(f.fileno())
            self.send_header("Content-Length", str(fs[6]))
            self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
            self.end_headers()
            return f
        except:
            f.close()
            raise
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
        # 返回模拟的设备状态数据
        status_data = {
            "currentTemp": 25,
            "targetTemp": 30,
            "heatingEnabled": True,
            "wifiConnected": True,
            "wifiSSID": "TestWiFi",
            "ipAddress": "192.168.1.100",
            "rssi": -65,
            "deviceId": "oven123",
            "firmwareVersion": "v0.8.6",
            "uptime": 3600,
            "freeHeap": 45000
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