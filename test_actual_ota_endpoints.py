import requests
import json
import time

def test_ota_endpoints(device_ip="192.168.16.105"):
    """测试实际的OTA端点"""
    
    base_url = f"http://{device_ip}"
    
    print("🚀 测试SmartOvenController实际OTA端点")
    print("=" * 60)
    
    # 测试固件更新端点 /update
    print("\n🔧 测试固件更新端点 /update")
    print("-" * 40)
    
    # 准备一个小的测试固件文件（模拟.bin文件）
    test_firmware_content = b"\x00" * 1024  # 1KB的测试数据
    
    try:
        # 测试POST上传到/update端点
        files = {'firmware': ('test_firmware.bin', test_firmware_content, 'application/octet-stream')}
        response = requests.post(f"{base_url}/update", files=files, timeout=10)
        
        print(f"   POST /update 状态码: {response.status_code}")
        if response.status_code == 200:
            print(f"   ✅ 固件上传成功!")
            print(f"   响应: {response.text[:200]}")
        else:
            print(f"   ❌ 固件上传失败")
            print(f"   响应头: {dict(response.headers)}")
            
    except Exception as e:
        print(f"   ❌ 固件上传测试失败: {e}")
    
    # 测试文件系统更新端点 /update
print("\n📁 测试文件系统更新端点 /update")
    print("-" * 40)
    
    try:
        # 测试POST上传到/update端点 (文件系统更新)
        files = {'filesystem': ('test_filesystem.bin', test_firmware_content, 'application/octet-stream')}
        response = requests.post(f"{base_url}/update", files=files, timeout=10)
        
        print(f"   POST /update 状态码: {response.status_code}")
        if response.status_code == 200:
            print(f"   ✅ 文件系统上传成功!")
            print(f"   响应: {response.text[:200]}")
        else:
            print(f"   ❌ 文件系统上传失败")
            print(f"   响应头: {dict(response.headers)}")
            
    except Exception as e:
        print(f"   ❌ 文件系统上传测试失败: {e}")
    
    # 测试GET请求到这些端点
    print("\n🔍 测试端点GET请求")
    print("-" * 40)
    
    endpoints = ['/update']
    
    for endpoint in endpoints:
        try:
            response = requests.get(f"{base_url}{endpoint}", timeout=5)
            print(f"   GET {endpoint} 状态码: {response.status_code}")
            if response.status_code == 200:
                print(f"   ✅ 端点可访问")
            elif response.status_code == 405:
                print(f"   ⚠️ 端点存在但不支持GET方法")
            else:
                print(f"   ❌ 端点访问失败")
        except Exception as e:
            print(f"   ❌ {endpoint} GET测试失败: {e}")

def test_ota_page_functionality(device_ip="192.168.16.105"):
    """测试OTA页面的实际功能"""
    
    base_url = f"http://{device_ip}"
    
    print("\n🌐 测试OTA页面功能")
    print("=" * 60)
    
    # 获取OTA页面内容
    try:
        response = requests.get(f"{base_url}/ota_update", timeout=10)
        
        if response.status_code == 200:
            print("✅ OTA页面可正常访问")
            
            # 检查页面内容
            content = response.text
            
            # 检查是否存在上传表单
            if 'uploadFirmware' in content:
                print("✅ 发现固件上传功能")
            else:
                print("❌ 未发现固件上传功能")
                
            if 'uploadFilesystem' in content:
                print("✅ 发现文件系统上传功能")
            else:
                print("❌ 未发现文件系统上传功能")
                
            # 检查JavaScript代码
            if '/update' in content:
                print("✅ 发现固件更新端点 /update")
            else:
                print("❌ 未发现固件更新端点 /update")
                
            if '/update' in content:
                print("✅ 发现文件系统更新端点 /update")
            else:
                print("❌ 未发现文件系统更新端点 /update")
                
            # 检查表单字段名称
            if 'name="firmware"' in content:
                print("✅ 发现固件字段名称: firmware")
            else:
                print("❌ 未发现固件字段名称")
                
            if 'name="filesystem"' in content:
                print("✅ 发现文件系统字段名称: filesystem")
            else:
                print("❌ 未发现文件系统字段名称: filesystem")
                
        else:
            print(f"❌ OTA页面访问失败: {response.status_code}")
            
    except Exception as e:
        print(f"❌ OTA页面测试失败: {e}")

def test_device_restart_after_ota(device_ip="192.168.16.105"):
    """测试OTA升级后的设备重启功能"""
    
    base_url = f"{device_ip}"
    
    print("\n🔄 测试设备重启功能")
    print("-" * 40)
    
    # 首先获取设备当前状态
    try:
        response = requests.get(f"http://{base_url}/status", timeout=5)
        if response.status_code == 200:
            print("✅ 设备当前在线")
            
            # 测试重启端点
            response = requests.post(f"http://{base_url}/reset", timeout=5)
            print(f"   重启请求状态: {response.status_code}")
            
            if response.status_code == 200:
                print("✅ 重启命令发送成功")
                
                # 等待设备重启
                print("⏳ 等待设备重启...")
                time.sleep(10)
                
                # 检查设备是否重新上线
                try:
                    response = requests.get(f"http://{base_url}/status", timeout=10)
                    if response.status_code == 200:
                        print("✅ 设备重启后重新上线")
                    else:
                        print("❌ 设备重启后未重新上线")
                except:
                    print("❌ 设备重启后无法连接")
            else:
                print("❌ 重启命令发送失败")
                
        else:
            print("❌ 设备当前不在线")
            
    except Exception as e:
        print(f"❌ 重启测试失败: {e}")

if __name__ == "__main__":
    # 测试实际OTA端点
    test_ota_endpoints()
    
    # 测试OTA页面功能
    test_ota_page_functionality()
    
    # 测试设备重启功能
    test_device_restart_after_ota()
    
    print("\n" + "=" * 60)
    print("📊 OTA功能测试总结")
    print("=" * 60)
    print("💡 根据代码分析，SmartOvenController的OTA功能通过以下端点实现:")
    print("   - 固件更新: POST /update (字段名: firmware)")
    print("   - 文件系统更新: POST /update (字段名: filesystem)")
    print("   - 设备重启: POST /reset")
    print("\n🔧 建议使用浏览器访问 http://192.168.16.105/ota_update")
    print("   查看实际的OTA升级界面")