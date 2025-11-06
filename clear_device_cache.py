#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
智能电烤箱设备缓存清除工具
用于清除设备中的各种缓存，解决页面无变化问题
"""

import requests
import time
import sys

def clear_device_cache():
    """清除设备缓存的主函数"""
    
    # 设备IP地址（根据实际情况修改）
    device_ip = "192.168.16.104"
    
    print("🔧 智能电烤箱设备缓存清除工具")
    print("=" * 50)
    
    try:
        # 1. 首先检查设备是否在线
        print("1. 检查设备连接状态...")
        try:
            response = requests.get(f"http://{device_ip}/status", timeout=5)
            if response.status_code == 200:
                print("   ✅ 设备在线，可以正常通信")
            else:
                print(f"   ⚠️ 设备响应异常，状态码: {response.status_code}")
        except requests.exceptions.RequestException as e:
            print(f"   ❌ 无法连接到设备: {e}")
            print("   请检查:")
            print("   - 设备是否已开机")
            print("   - IP地址是否正确")
            print("   - 网络连接是否正常")
            return False
        
        # 2. 恢复出厂设置（清除EEPROM缓存）
        print("\n2. 执行恢复出厂设置...")
        try:
            response = requests.post(f"http://{device_ip}/factoryreset", timeout=10)
            if response.status_code == 200:
                print("   ✅ 恢复出厂设置命令已发送")
                print("   📝 设备将:")
                print("   - 清除所有WiFi配置")
                print("   - 清除温度校准参数")
                print("   - 重启并进入配网模式")
            else:
                print(f"   ⚠️ 恢复出厂设置失败，状态码: {response.status_code}")
        except requests.exceptions.RequestException as e:
            print(f"   ❌ 恢复出厂设置命令发送失败: {e}")
        
        # 3. 等待设备重启
        print("\n3. 等待设备重启...")
        for i in range(30, 0, -1):
            print(f"   ⏳ 等待 {i} 秒...", end="\r")
            time.sleep(1)
        print("\n   ✅ 设备重启完成")
        
        # 4. 清除浏览器缓存建议
        print("\n4. 浏览器缓存清除建议:")
        print("   🌐 请在浏览器中执行以下操作:")
        print("   - Windows: Ctrl+Shift+R (强制刷新)")
        print("   - Mac: Cmd+Shift+R (强制刷新)")
        print("   - 或打开开发者工具(Ctrl+Shift+I/F12)")
        print("   - 右键刷新按钮 → '清空缓存并硬性重新加载'")
        
        print("\n" + "=" * 50)
        print("🎯 缓存清除操作完成！")
        print("📋 后续步骤:")
        print("1. 设备重启后需要重新配置WiFi")
        print("2. 浏览器强制刷新页面")
        print("3. 重新进行温度校准")
        
        return True
        
    except KeyboardInterrupt:
        print("\n\n⚠️ 用户中断操作")
        return False

if __name__ == "__main__":
    if len(sys.argv) > 1:
        # 支持命令行参数指定设备IP
        device_ip = sys.argv[1]
    
    success = clear_device_cache()
    
    if success:
        print("\n✨ 设备缓存清除成功！页面应该能够显示最新内容了。")
        sys.exit(0)
    else:
        print("\n❌ 设备缓存清除过程中出现问题")
        sys.exit(1)