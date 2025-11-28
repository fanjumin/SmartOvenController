#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
测试缓存清除功能
"""

import requests
import time
import json

def test_clear_cache():
    """测试缓存清除功能"""
    device_ip = "192.168.16.100"
    clear_cache_url = f"http://{device_ip}/clear_cache"

    try:
        print("正在清除设备缓存...")
        response = requests.post(clear_cache_url, timeout=30)

        print(f"响应状态码: {response.status_code}")
        print(f"响应内容: {response.text}")

        if response.status_code == 200:
            try:
                result = response.json()
                if result.get('status') == 'success':
                    print("✅ 缓存清除成功！")
                    print(f"删除了 {result.get('deleted_files', 0)} 个文件")
                    print(f"缓存清除时间戳: {result.get('cache_bust', 'N/A')}")
                    return True
                else:
                    print(f"❌ 清除失败: {result.get('message', '未知错误')}")
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

def test_file_list():
    """测试文件列表功能"""
    device_ip = "192.168.16.100"
    file_list_url = f"http://{device_ip}/file_list"

    try:
        print("\n正在获取设备文件列表...")
        response = requests.get(file_list_url, timeout=30)

        print(f"响应状态码: {response.status_code}")

        if response.status_code == 200:
            try:
                result = response.json()
                files = result.get('files', [])
                print(f"设备上共有 {len(files)} 个文件:")
                for file_info in files:
                    print(f"  - {file_info['name']} ({file_info['size']} 字节)")
                return True
            except:
                print("❌ 响应解析失败")
                return False
        else:
            print(f"❌ HTTP错误: {response.status_code}")
            return False

    except requests.exceptions.RequestException as e:
        print(f"❌ 网络错误: {e}")
        return False

def test_upload_file_after_clear():
    """测试清除缓存后上传文件"""
    device_ip = "192.168.16.100"
    upload_url = f"http://{device_ip}/upload_file"

    # 创建测试文件
    test_content = f"测试文件 - 清除缓存后上传 - 时间戳: {int(time.time())}"
    test_file_path = "test_after_clear.txt"

    with open(test_file_path, 'w', encoding='utf-8') as f:
        f.write(test_content)

    try:
        print(f"\n正在上传测试文件 {test_file_path}...")
        with open(test_file_path, 'rb') as f:
            files = {'file': (test_file_path, f, 'text/plain')}
            data = {'target_path': '/test_after_clear.txt'}

            response = requests.post(upload_url, files=files, data=data, timeout=30)

        print(f"响应状态码: {response.status_code}")
        print(f"响应内容: {response.text}")

        if response.status_code == 200:
            try:
                result = response.json()
                if result.get('status') == 'success':
                    print("✅ 文件上传成功！")
                    print(f"目标路径: {result.get('target_path', 'N/A')}")
                    print(f"文件大小: {result.get('file_size', 0)} 字节")
                    print(f"缓存清除时间戳: {result.get('cache_bust', 'N/A')}")
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
    finally:
        # 清理本地测试文件
        import os
        if os.path.exists(test_file_path):
            os.remove(test_file_path)

if __name__ == "__main__":
    print("开始测试缓存清除功能...")
    print("=" * 50)

    # 测试1: 清除缓存
    success1 = test_clear_cache()

    if success1:
        # 等待设备重新初始化
        print("\n等待设备重新初始化...")
        time.sleep(3)

        # 测试2: 获取文件列表
        success2 = test_file_list()

        # 测试3: 上传文件验证
        success3 = test_upload_file_after_clear()

        print("\n" + "=" * 50)
        if success1 and success2 and success3:
            print("🎉 所有测试通过！缓存清除功能正常工作")
        else:
            print("💥 部分测试失败，请检查设备状态")
    else:
        print("💥 缓存清除失败，无法继续测试")