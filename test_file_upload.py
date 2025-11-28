#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
测试OTA单个文件上传功能
"""

import requests
import os

def test_file_upload():
    """测试单个文件上传功能"""
    device_ip = "192.168.16.100"
    upload_url = f"http://{device_ip}/upload_file"

    # 测试文件路径
    test_file = "test_upload.txt"

    if not os.path.exists(test_file):
        print(f"错误：测试文件 {test_file} 不存在")
        return False

    try:
        # 准备文件上传
        with open(test_file, 'rb') as f:
            files = {'file': (test_file, f, 'text/plain')}
            data = {'target_path': '/test_upload_new.txt'}

            print(f"正在上传文件 {test_file} 到 {device_ip}...")
            response = requests.post(upload_url, files=files, data=data, timeout=30)

            print(f"响应状态码: {response.status_code}")
            print(f"响应内容: {response.text}")

            if response.status_code == 200:
                try:
                    result = response.json()
                    if result.get('status') == 'success':
                        print("✅ 文件上传成功！")
                        return True
                    else:
                        print(f"❌ 上传失败: {result.get('message', '未知错误')}")
                        return False
                except:
                    print("❌ 响应解析失败")
                    return False
            else:
                print(f"❌ HTTP错误: {response.status_code}")
                return False

    except requests.exceptions.RequestException as e:
        print(f"❌ 网络错误: {e}")
        return False

if __name__ == "__main__":
    print("开始测试OTA单个文件上传功能...")
    success = test_file_upload()
    if success:
        print("🎉 测试通过！")
    else:
        print("💥 测试失败！")