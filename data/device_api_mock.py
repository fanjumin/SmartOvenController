from http.server import HTTPServer, SimpleHTTPRequestHandler
import json
import time

class DeviceAPIHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/device-info':
            # 模拟设备信息响应
            device_info = {
                "model": "SmartOven Pro",
                "firmware": "0.8.7",
                "hardware": "v2.1",
                "serial": "SN001",
                "uptime": int(time.time() % 3600),  # 模拟运行时间
                "freeHeap": 128,
                "lastBoot": int(time.time() * 1000 - 3600000),  # 1小时前启动
                "wifiConnected": True
            }
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(device_info).encode())
        else:
            # 处理静态文件
            super().do_GET()

if __name__ == '__main__':
    server = HTTPServer(('localhost', 8001), DeviceAPIHandler)
    print("Device API mock server running on http://localhost:8001")
    print("Press Ctrl+C to stop")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped")