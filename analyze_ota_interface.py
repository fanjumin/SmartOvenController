import requests
from bs4 import BeautifulSoup
import re

def analyze_ota_interface(device_ip="192.168.16.105"):
    """分析OTA页面的实际接口配置"""
    
    base_url = f"http://{device_ip}"
    
    print("🔍 分析OTA页面接口配置...")
    print("-" * 50)
    
    # 获取OTA页面HTML内容
    try:
        response = requests.get(f"{base_url}/ota_update", timeout=10)
        
        if response.status_code == 200:
            print("✅ 成功获取OTA页面HTML")
            
            # 解析HTML
            soup = BeautifulSoup(response.text, 'html.parser')
            
            # 查找表单元素
            forms = soup.find_all('form')
            print(f"📋 找到 {len(forms)} 个表单")
            
            for i, form in enumerate(forms):
                print(f"\n📄 表单 {i+1}:")
                
                # 获取表单属性
                action = form.get('action', '')
                method = form.get('method', 'GET').upper()
                enctype = form.get('enctype', '')
                
                print(f"   Action: {action}")
                print(f"   Method: {method}")
                print(f"   Enctype: {enctype}")
                
                # 查找文件输入字段
                file_inputs = form.find_all('input', {'type': 'file'})
                print(f"   文件输入字段: {len(file_inputs)} 个")
                
                for input_field in file_inputs:
                    name = input_field.get('name', '')
                    accept = input_field.get('accept', '')
                    print(f"     - 名称: {name}, 接受类型: {accept}")
                
                # 查找提交按钮
                submit_buttons = form.find_all(['input', 'button'], {
                    'type': ['submit', 'button']
                })
                print(f"   提交按钮: {len(submit_buttons)} 个")
            
            # 查找JavaScript代码中的上传逻辑
            scripts = soup.find_all('script')
            print(f"\n📜 找到 {len(scripts)} 个脚本")
            
            for script in scripts:
                if script.string:
                    # 查找AJAX上传相关的代码
                    if 'upload' in script.string.lower() or 'ajax' in script.string.lower():
                        print("🔍 发现上传相关的JavaScript代码")
                        
                        # 提取URL模式
                        url_patterns = re.findall(r'["\'](/[^"\']*upload[^"\']*)["\']', script.string)
                        if url_patterns:
                            print("   发现上传URL模式:")
                            for pattern in set(url_patterns):
                                print(f"     - {pattern}")
                        
                        # 提取文件字段名称
                        file_field_patterns = re.findall(r'["\'](file[^"\']*)["\']', script.string, re.IGNORECASE)
                        if file_field_patterns:
                            print("   发现文件字段名称:")
                            for field in set(file_field_patterns):
                                print(f"     - {field}")
            
            # 检查页面文本内容
            if "上传" in response.text or "upload" in response.text.lower():
                print("\n📝 页面包含上传相关文本")
                
                # 提取上传相关的文本片段
                upload_lines = [line.strip() for line in response.text.split('\n') 
                               if '上传' in line or 'upload' in line.lower()]
                
                for line in upload_lines[:5]:  # 显示前5行
                    if len(line) > 100:
                        print(f"   ...{line[:100]}...")
                    else:
                        print(f"   {line}")
        
        else:
            print(f"❌ 获取OTA页面失败: {response.status_code}")
            
    except Exception as e:
        print(f"❌ 分析失败: {e}")

def test_actual_upload_endpoints(device_ip="192.168.16.105"):
    """测试实际可能的上传端点"""
    
    base_url = f"http://{device_ip}"
    
    print("\n🧪 测试实际上传端点...")
    print("-" * 50)
    
    # 常见ESP8266 OTA上传端点
    upload_endpoints = [
        "/update",                    # 标准OTA端点
        "/firmware/update",           # 固件更新端点
        "/ota",                       # 简化OTA端点
        "/upload",                    # 通用上传端点
        "/webupdate",                 # Web更新端点
        "/esp/update",                # ESP特定端点
        "/api/update",                # API更新端点
        "/system/update",             # 系统更新端点
    ]
    
    # 准备测试文件
    test_file_path = "test_ota_files/test_config.json"
    
    for endpoint in upload_endpoints:
        try:
            print(f"\n🔍 测试端点: {endpoint}")
            
            # 首先检查端点是否存在
            response = requests.get(f"{base_url}{endpoint}", timeout=5)
            print(f"   GET请求状态: {response.status_code}")
            
            # 尝试POST上传
            with open(test_file_path, 'rb') as f:
                files = {'file': ('test.json', f, 'application/json')}
                
                # 尝试不同的字段名称
                field_names = ['file', 'firmware', 'upload', 'update_file']
                
                for field_name in field_names:
                    try:
                        files = {field_name: ('test.json', open(test_file_path, 'rb'), 'application/json')}
                        
                        response = requests.post(
                            f"{base_url}{endpoint}",
                            files=files,
                            timeout=10
                        )
                        
                        print(f"   POST上传({field_name}): {response.status_code}")
                        
                        if response.status_code == 200:
                            print(f"   ✅ 上传成功!")
                            print(f"   响应: {response.text[:200]}")
                            return endpoint, field_name
                            
                    except Exception as e:
                        print(f"   ❌ {field_name}上传失败: {e}")
        
        except Exception as e:
            print(f"   ❌ 端点测试失败: {e}")
    
    return None, None

if __name__ == "__main__":
    print("🚀 OTA接口配置分析")
    print("=" * 60)
    
    # 分析页面结构
    analyze_ota_interface()
    
    # 测试实际端点
    endpoint, field_name = test_actual_upload_endpoints()
    
    if endpoint and field_name:
        print(f"\n🎯 发现有效上传配置:")
        print(f"   端点: {endpoint}")
        print(f"   字段名: {field_name}")
    else:
        print("\n⚠️ 未发现有效的上传配置")
        print("💡 建议检查:")
        print("   1. OTA页面是否包含实际的上传功能")
        print("   2. 是否需要特定的认证或参数")
        print("   3. 固件是否支持文件系统上传")