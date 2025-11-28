import requests

# 简单测试固件升级
try:
    with open('test_firmware.bin', 'rb') as f:
        files = {'firmware': f}
        print("正在上传固件...")
        response = requests.post('http://192.168.16.100/firmware_update', files=files, timeout=10)
        print("固件升级响应状态码:", response.status_code)
        print("固件升级响应头:", response.headers)
        print("固件升级响应内容:", response.text)
except Exception as e:
    print(f"固件升级发生错误: {e}")