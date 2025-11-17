import requests
import time

def scan_endpoints():
    # 设备IP地址
    device_ip = "192.168.16.105"
    
    # 常见的端点列表
    endpoints = [
        "/",
        "/index.html",
        "/upload.html",
        "/update.html",
        
        "/api",
        "/api/status",
        "/api/config",
        "/settings",
        "/device_status.html",
        "/settings_help.html",
        "/temperature_calibration.html",
        "/wifi_config.html"
    ]
    
    print(f"扫描设备 {device_ip} 上的端点...")
    
    for endpoint in endpoints:
        url = f"http://{device_ip}{endpoint}"
        try:
            response = requests.get(url, timeout=5)
            print(f"状态码 {response.status_code}: {url}")
        except requests.exceptions.RequestException as e:
            print(f"错误 {type(e).__name__}: {url}")
        time.sleep(0.5)  # 避免过于频繁的请求

if __name__ == "__main__":
    scan_endpoints()