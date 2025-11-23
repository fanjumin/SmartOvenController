import subprocess
import re

def scan_wifi_networks():
    try:
        # 执行netsh命令扫描WiFi网络
        result = subprocess.run(['netsh', 'wlan', 'show', 'networks', 'mode=Bssid'], 
                              capture_output=True, text=True, check=True)
        
        print("完整的命令输出:")
        print(result.stdout)
        print("=" * 50)
        
        # 解析输出寻找SmartOven热点
        networks = []
        lines = result.stdout.split('\n')
        
        for line in lines:
            line = line.strip()
            
            if line.startswith('SSID'):
                ssid_match = re.search(r'SSID\s+\d+\s*:\s*(.*)', line)
                if ssid_match:
                    ssid = ssid_match.group(1)
                    networks.append({'ssid': ssid})
        
        return networks
    
    except subprocess.CalledProcessError as e:
        print(f"执行命令时出错: {e}")
        return []

def find_smartoven_network():
    networks = scan_wifi_networks()
    
    for network in networks:
        if 'ssid' in network and 'SmartOven' in network['ssid']:
            return network
    
    return None

if __name__ == "__main__":
    print("正在扫描附近的WiFi网络...")
    smartoven_network = find_smartoven_network()
    
    if smartoven_network:
        print(f"找到SmartOven热点:")
        print(f"  SSID: {smartoven_network['ssid']}")
    else:
        print("未找到SmartOven热点，请确保设备已启动并创建了热点")
        
        print("\n附近的所有网络:")
        networks = scan_wifi_networks()
        for i, network in enumerate(networks[:15]):  # 显示前15个网络
            if 'ssid' in network:
                print(f"  {i+1}. {network['ssid']}")