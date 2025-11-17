import requests
import json
import time
import os
from pathlib import Path

class OTAFunctionalTest:
    def __init__(self, device_ip="192.168.16.105"):
        self.device_ip = device_ip
        self.base_url = f"http://{device_ip}"
        self.test_results = {}
        
    def test_ota_page_accessibility(self):
        """测试OTA页面可访问性"""
        print("🔍 测试OTA页面可访问性...")
        
        try:
            response = requests.get(f"{self.base_url}/ota_update", timeout=10)
            if response.status_code == 200:
                print("✅ OTA页面可正常访问")
                self.test_results["ota_page_access"] = "PASS"
                
                # 检查页面内容
                if "upload" in response.text.lower() or "ota" in response.text.lower():
                    print("✅ OTA页面包含上传功能")
                    self.test_results["ota_page_content"] = "PASS"
                else:
                    print("⚠️ OTA页面内容可能不完整")
                    self.test_results["ota_page_content"] = "WARNING"
                    
            else:
                print(f"❌ OTA页面访问失败: {response.status_code}")
                self.test_results["ota_page_access"] = "FAIL"
                
        except Exception as e:
            print(f"❌ OTA页面连接失败: {e}")
            self.test_results["ota_page_access"] = "FAIL"
    
    def test_file_upload_functionality(self):
        """测试文件上传功能"""
        print("\n🔍 测试文件上传功能...")
        
        # 准备测试文件
        test_files = [
            ("test_config.json", "application/json"),
            ("test_style.css", "text/css")
        ]
        
        for filename, content_type in test_files:
            file_path = Path("test_ota_files") / filename
            
            if not file_path.exists():
                print(f"⚠️ 测试文件不存在: {file_path}")
                continue
                
            try:
                print(f"📤 上传文件: {filename}")
                
                with open(file_path, 'rb') as f:
                    files = {'file': (filename, f, content_type)}
                    
                    # 尝试上传到不同端点
                    upload_endpoints = [
                        "/ota_update",
                        "/upload",
                        "/update"
                    ]
                    
                    uploaded = False
                    for endpoint in upload_endpoints:
                        try:
                            response = requests.post(
                                f"{self.base_url}{endpoint}",
                                files=files,
                                timeout=30
                            )
                            
                            if response.status_code == 200:
                                print(f"✅ 文件上传成功到 {endpoint}")
                                print(f"   响应: {response.text[:200]}...")
                                uploaded = True
                                self.test_results[f"upload_{filename}"] = "PASS"
                                break
                            else:
                                print(f"⚠️ {endpoint} 上传失败: {response.status_code}")
                                
                        except Exception as e:
                            print(f"❌ {endpoint} 上传异常: {e}")
                    
                    if not uploaded:
                        print(f"❌ 所有上传端点都失败")
                        self.test_results[f"upload_{filename}"] = "FAIL"
                        
            except Exception as e:
                print(f"❌ 文件上传测试失败: {e}")
                self.test_results[f"upload_{filename}"] = "FAIL"
    
    def test_device_reboot_functionality(self):
        """测试设备重启功能（如果支持）"""
        print("\n🔍 测试设备重启功能...")
        
        reboot_endpoints = [
            "/reboot",
            "/restart",
            "/system/reboot"
        ]
        
        for endpoint in reboot_endpoints:
            try:
                print(f"🔄 尝试重启端点: {endpoint}")
                response = requests.post(f"{self.base_url}{endpoint}", timeout=5)
                
                if response.status_code == 200:
                    print(f"✅ 重启命令发送成功")
                    self.test_results["reboot_function"] = "PASS"
                    
                    # 等待设备重启
                    print("⏳ 等待设备重启...")
                    time.sleep(10)
                    
                    # 检查设备是否重新上线
                    if self.check_device_online():
                        print("✅ 设备重启后重新上线")
                        self.test_results["reboot_recovery"] = "PASS"
                    else:
                        print("⚠️ 设备重启后未及时上线")
                        self.test_results["reboot_recovery"] = "WARNING"
                    
                    break
                    
            except requests.exceptions.Timeout:
                print(f"⚠️ {endpoint} 请求超时（可能是正常现象）")
            except Exception as e:
                print(f"❌ {endpoint} 重启测试失败: {e}")
    
    def check_device_online(self, max_attempts=10):
        """检查设备是否在线"""
        for attempt in range(max_attempts):
            try:
                response = requests.get(f"{self.base_url}/", timeout=5)
                if response.status_code == 200:
                    return True
            except:
                pass
            time.sleep(2)
        return False
    
    def test_version_verification(self):
        """测试版本验证功能"""
        print("\n🔍 测试版本验证功能...")
        
        # 获取当前版本信息
        try:
            response = requests.get(f"{self.base_url}/status", timeout=10)
            if response.status_code == 200:
                status_data = response.json()
                current_version = status_data.get("firmware_version", "unknown")
                print(f"📋 当前固件版本: {current_version}")
                self.test_results["current_version"] = current_version
                
                # 检查版本信息是否包含在状态中
                if current_version != "unknown":
                    print("✅ 版本信息可正常获取")
                    self.test_results["version_verification"] = "PASS"
                else:
                    print("⚠️ 版本信息获取不完整")
                    self.test_results["version_verification"] = "WARNING"
                    
        except Exception as e:
            print(f"❌ 版本验证测试失败: {e}")
            self.test_results["version_verification"] = "FAIL"
    
    def run_comprehensive_test(self):
        """运行完整的OTA功能测试"""
        print("🚀 开始OTA功能全面测试")
        print("=" * 60)
        
        # 检查设备是否在线
        if not self.check_device_online():
            print("❌ 设备不在线，无法进行测试")
            return
        
        # 执行各项测试
        self.test_ota_page_accessibility()
        self.test_version_verification()
        self.test_file_upload_functionality()
        self.test_device_reboot_functionality()
        
        # 生成测试报告
        self.generate_test_report()
    
    def generate_test_report(self):
        """生成测试报告"""
        print("\n" + "=" * 60)
        print("📊 OTA功能测试报告")
        print("=" * 60)
        
        passed = sum(1 for result in self.test_results.values() if result == "PASS")
        total = len(self.test_results)
        
        print(f"测试项目总数: {total}")
        print(f"通过项目: {passed}")
        print(f"成功率: {passed/total*100:.1f}%")
        
        print("\n详细结果:")
        for test_name, result in self.test_results.items():
            status_icon = "✅" if result == "PASS" else "⚠️" if result == "WARNING" else "❌"
            print(f"  {status_icon} {test_name}: {result}")
        
        # 测试建议
        print("\n💡 测试建议:")
        if passed == total:
            print("  ✅ OTA功能测试全部通过，功能正常")
        else:
            print("  ⚠️ 部分功能需要进一步验证")
            if "ota_page_access" in self.test_results and self.test_results["ota_page_access"] == "FAIL":
                print("  • 检查OTA页面是否正常部署")
            if any("upload_" in key and self.test_results[key] == "FAIL" for key in self.test_results):
                print("  • 检查文件上传接口配置")

if __name__ == "__main__":
    # 创建测试目录
    test_dir = Path("test_ota_files")
    test_dir.mkdir(exist_ok=True)
    
    # 运行测试
    tester = OTAFunctionalTest()
    tester.run_comprehensive_test()