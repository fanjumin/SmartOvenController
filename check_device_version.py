import requests
import json
import time

def check_device_info(device_ip):
    """检查设备信息和版本"""
    
    print(f"🎯 目标设备IP: {device_ip}")
    print("-" * 50)
    
    # 测试端点列表
    endpoints = [
        ("/", "主页面"),
        ("/status", "状态页面"),
        ("/version", "版本信息"),
        ("/ota_update", "OTA升级页面"),
        ("/info", "设备信息"),
        ("/config", "配置信息")
    ]
    
    device_info = {}
    
    for endpoint, description in endpoints:
        try:
            url = f"http://{device_ip}{endpoint}"
            print(f"🔍 测试 {description}: {url}")
            
            response = requests.get(url, timeout=10)
            if response.status_code == 200:
                print(f"✅ {description}可访问")
                
                # 保存响应内容用于分析
                if endpoint == "/version":
                    device_info["version"] = response.text.strip()
                    print(f"   版本: {response.text.strip()}")
                elif endpoint == "/status":
                    try:
                        status_data = response.json()
                        device_info["status"] = status_data
                        print(f"   状态: {json.dumps(status_data, indent=2)}")
                    except:
                        device_info["status"] = response.text
                        print(f"   状态: {response.text}")
                
            else:
                print(f"⚠️ {description}不可访问: {response.status_code}")
                
        except requests.exceptions.RequestException as e:
            print(f"❌ {description}连接失败: {e}")
    
    return device_info

if __name__ == "__main__":
    device_ip = "192.168.16.105"
    print("🚀 开始检查设备状态...")
    print()
    
    device_info = check_device_info(device_ip)
    
    print("\n" + "="*50)
    print("📊 设备信息汇总:")
    print(f"IP地址: {device_ip}")
    print(f"管理页面: http://{device_ip}")
    print(f"OTA页面: http://{device_ip}/ota_update")
    
    if "version" in device_info:
        print(f"当前版本: {device_info['version']}")
    
    print("\n🎯 下一步: 准备OTA测试文件")