#!/usr/bin/env python3
"""
测试网页温度显示性能
"""

import requests
import time
import json

def test_web_response_time():
    """测试网页响应时间"""
    print("=== 网页温度显示性能测试 ===")
    
    device_ip = "192.168.16.104"
    urls_to_test = [
        "/",           # 主页
        "/status",     # 状态API
        "/temperature", # 温度页面
        "/api/temp"     # 温度API
    ]
    
    for url_path in urls_to_test:
        url = f"http://{device_ip}{url_path}"
        print(f"\n测试URL: {url}")
        
        try:
            # 测试响应时间
            start_time = time.time()
            response = requests.get(url, timeout=10)
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000  # 毫秒
            
            print(f"响应时间: {response_time:.1f}ms")
            print(f"状态码: {response.status_code}")
            print(f"内容长度: {len(response.text)} 字节")
            
            # 如果是JSON响应，显示温度数据
            if response.headers.get('Content-Type', '').startswith('application/json'):
                try:
                    data = response.json()
                    if 'temperature' in data:
                        print(f"温度数据: {data['temperature']}°C")
                except:
                    pass
            
            # 显示前100个字符的内容
            content_preview = response.text[:100]
            print(f"内容预览: {content_preview}")
            
        except requests.exceptions.Timeout:
            print("❌ 请求超时")
        except Exception as e:
            print(f"❌ 请求失败: {e}")

def test_multiple_requests():
    """测试多次请求的性能"""
    print("\n=== 多次请求性能测试 ===")
    
    device_ip = "192.168.16.104"
    url = f"http://{device_ip}/status"
    
    response_times = []
    
    for i in range(10):
        try:
            start_time = time.time()
            response = requests.get(url, timeout=5)
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000
            response_times.append(response_time)
            
            # 显示温度数据
            if response.status_code == 200:
                try:
                    data = response.json()
                    temp = data.get('temperature', '未知')
                    print(f"第{i+1}次请求: {response_time:.1f}ms, 温度: {temp}°C")
                except:
                    print(f"第{i+1}次请求: {response_time:.1f}ms")
            
            time.sleep(0.5)  # 间隔0.5秒
            
        except Exception as e:
            print(f"第{i+1}次请求失败: {e}")
    
    # 统计结果
    if response_times:
        avg_time = sum(response_times) / len(response_times)
        max_time = max(response_times)
        min_time = min(response_times)
        
        print(f"\n📊 性能统计:")
        print(f"平均响应时间: {avg_time:.1f}ms")
        print(f"最快响应时间: {min_time:.1f}ms")
        print(f"最慢响应时间: {max_time:.1f}ms")
        
        # 判断性能问题
        if avg_time > 1000:
            print("🔴 严重性能问题: 响应时间超过1秒")
        elif avg_time > 500:
            print("🟡 性能问题: 响应时间超过500ms")
        else:
            print("🟢 性能良好: 响应时间正常")

def check_web_interface():
    """检查Web界面"""
    print("\n=== Web界面检查 ===")
    
    device_ip = "192.168.16.104"
    
    try:
        # 检查主页
        response = requests.get(f"http://{device_ip}/", timeout=10)
        print(f"主页状态: {response.status_code}")
        
        # 检查是否有温度显示相关的JavaScript
        if "temperature" in response.text.lower() or "temp" in response.text.lower():
            print("✅ 页面包含温度显示相关代码")
        else:
            print("⚠️ 页面可能缺少温度显示代码")
        
        # 检查是否有自动刷新机制
        if "setInterval" in response.text or "setTimeout" in response.text:
            print("✅ 页面有自动刷新机制")
        else:
            print("⚠️ 页面可能缺少自动刷新")
            
    except Exception as e:
        print(f"❌ Web界面检查失败: {e}")

if __name__ == "__main__":
    test_web_response_time()
    test_multiple_requests()
    check_web_interface()