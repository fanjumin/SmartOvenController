import requests
import time

# 测试/reset_calibration路由功能
def test_reset_calibration():
    # 使用用户提供的设备IP地址
    urls = [
        "http://192.168.16.104/reset_calibration",  # 用户提供的设备IP
        "http://192.168.4.1/reset_calibration",   # 设备作为热点时的默认IP
        "http://192.168.1.100/reset_calibration"  # 常见的本地IP
    ]
    
    for url in urls:
        try:
            print(f"正在测试URL: {url}")
            response = requests.post(url, timeout=10)  # 增加超时时间
            print(f"成功连接到 {url}")
            print(f"状态码: {response.status_code}")
            print(f"响应内容: {response.text}")
            return True
        except requests.exceptions.RequestException as e:
            print(f"连接到 {url} 失败: {e}")
            continue
    
    print("无法连接到任何预设的IP地址")
    return False

if __name__ == "__main__":
    print("开始测试/reset_calibration路由功能...")
    test_reset_calibration()