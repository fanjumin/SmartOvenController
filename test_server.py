from http.server import HTTPServer, BaseHTTPRequestHandler
import json

class TestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/status':
            # 模拟设备状态响应
            response_data = {
                "temperature": 25.5,
                "target_temperature": 180.0,
                "heating_enabled": False,
                "wifi_connected": True,
                "wifiSSID": "TestNetwork",
                "ip_address": "192.168.1.100",
                "device_id": "test-device-001",
                "firmware_version": "0.8.0"
            }
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(response_data).encode())
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'Not Found')

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

if __name__ == '__main__':
    server = HTTPServer(('localhost', 8080), TestHandler)
    print('Test server running on http://localhost:8080')
    server.serve_forever()