#!/usr/bin/env python3
"""
SmartOven Controller API端点测试脚本
测试所有API端点的连接可用性
"""

import requests
import json
import time
from datetime import datetime

class APITester:
    def __init__(self, device_ip="192.168.16.104", timeout=5):
        self.device_ip = device_ip
        self.base_url = f"http://{device_ip}"
        self.timeout = timeout
        self.results = {}
        
        # 定义所有API端点及其测试方法
        self.endpoints = [
            {
                "name": "状态查询",
                "path": "/status",
                "method": "GET",
                "description": "获取设备状态信息"
            },
            {
                "name": "设备控制",
                "path": "/control",
                "method": "POST",
                "description": "控制加热器开关和温度",
                "data": {
                    "action": "get_status"
                }
            },
            {
                "name": "WiFi扫描",
                "path": "/scanwifi",
                "method": "GET",
                "description": "扫描可用的WiFi网络"
            },
            {
                "name": "WiFi保存",
                "path": "/savewifi",
                "method": "POST",
                "description": "保存WiFi配置",
                "data": {
                    "ssid": "test",
                    "password": "test123"
                }
            },
            {
                "name": "设备重启",
                "path": "/restart",
                "method": "POST",
                "description": "重启设备"
            },
            {
                "name": "恢复出厂设置",
                "path": "/factoryreset",
                "method": "POST",
                "description": "恢复设备出厂设置"
            },
            {
                "name": "OTA页面",
                "path": "/ota",
                "method": "GET",
                "description": "OTA升级页面"
            },
            {
                "name": "版本检查",
                "path": "/checkupdate",
                "method": "GET",
                "description": "检查固件版本更新"
            },
            {
                "name": "诊断信息",
                "path": "/diagnostic",
                "method": "GET",
                "description": "获取设备诊断信息"
            },
            {
                "name": "重置校准",
                "path": "/reset_calibration",
                "method": "POST",
                "description": "重置温度校准参数"
            },
            # 新添加的API端点
            {
                "name": "温度历史记录",
                "path": "/temperature_history",
                "method": "GET",
                "description": "获取温度历史记录数据"
            },
            {
                "name": "定时任务管理",
                "path": "/scheduler",
                "method": "GET",
                "description": "获取定时任务配置信息"
            },
            {
                "name": "设备信息",
                "path": "/device_info",
                "method": "GET",
                "description": "获取详细的设备信息"
            },
            {
                "name": "安全监控",
                "path": "/security_monitor",
                "method": "GET",
                "description": "获取安全监控状态"
            },
            {
                "name": "能耗统计",
                "path": "/energy_stats",
                "method": "GET",
                "description": "获取能耗统计信息"
            }
        ]
    
    def test_endpoint(self, endpoint):
        """测试单个API端点"""
        url = self.base_url + endpoint["path"]
        method = endpoint["method"]
        
        print(f"测试 {endpoint['name']} ({method} {endpoint['path']})...")
        
        try:
            if method == "GET":
                response = requests.get(url, timeout=self.timeout)
            else:  # POST
                data = endpoint.get("data", {})
                response = requests.post(url, json=data, timeout=self.timeout)
            
            # 记录结果
            result = {
                "status_code": response.status_code,
                "success": response.status_code == 200,
                "response_time": response.elapsed.total_seconds(),
                "content_type": response.headers.get('content-type', ''),
                "content_length": len(response.content)
            }
            
            # 尝试解析JSON响应
            try:
                result["json_response"] = response.json()
            except:
                result["text_response"] = response.text[:200]  # 只取前200个字符
            
            print(f"  ✓ 成功 - 状态码: {response.status_code}, 响应时间: {result['response_time']:.2f}s")
            return result
            
        except requests.exceptions.Timeout:
            print(f"  ✗ 超时 - 请求超过{self.timeout}秒")
            return {"success": False, "error": "请求超时"}
            
        except requests.exceptions.ConnectionError:
            print(f"  ✗ 连接失败 - 无法连接到设备")
            return {"success": False, "error": "连接失败"}
            
        except Exception as e:
            print(f"  ✗ 错误 - {str(e)}")
            return {"success": False, "error": str(e)}
    
    def run_all_tests(self):
        """运行所有API端点测试"""
        print(f"开始测试SmartOven Controller API端点")
        print(f"设备IP: {self.device_ip}")
        print(f"测试时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print("=" * 60)
        
        for endpoint in self.endpoints:
            self.results[endpoint["path"]] = self.test_endpoint(endpoint)
            time.sleep(0.5)  # 避免请求过于频繁
        
        self.generate_report()
    
    def generate_report(self):
        """生成测试报告"""
        print("\n" + "=" * 60)
        print("API端点测试报告")
        print("=" * 60)
        
        successful = 0
        failed = 0
        
        for endpoint in self.endpoints:
            path = endpoint["path"]
            result = self.results[path]
            
            if result.get("success"):
                successful += 1
                status = "✓ 可用"
            else:
                failed += 1
                status = "✗ 不可用"
            
            print(f"{endpoint['name']:15} {endpoint['method']:6} {path:20} {status}")
            if not result.get("success") and "error" in result:
                print(f"               错误: {result['error']}")
        
        print("=" * 60)
        print(f"总计: {len(self.endpoints)} 个端点")
        print(f"可用: {successful} 个")
        print(f"不可用: {failed} 个")
        print(f"成功率: {successful/len(self.endpoints)*100:.1f}%")
        
        # 保存详细结果到文件
        self.save_detailed_results()
    
    def save_detailed_results(self):
        """保存详细测试结果到JSON文件"""
        detailed_results = {
            "test_time": datetime.now().isoformat(),
            "device_ip": self.device_ip,
            "results": {}
        }
        
        for endpoint in self.endpoints:
            path = endpoint["path"]
            detailed_results["results"][path] = {
                "name": endpoint["name"],
                "method": endpoint["method"],
                "description": endpoint["description"],
                **self.results[path]
            }
        
        filename = f"api_test_results_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(detailed_results, f, indent=2, ensure_ascii=False)
        
        print(f"\n详细测试结果已保存到: {filename}")

def main():
    # 可以修改设备IP地址
    tester = APITester(device_ip="192.168.16.104", timeout=10)
    tester.run_all_tests()

if __name__ == "__main__":
    main()