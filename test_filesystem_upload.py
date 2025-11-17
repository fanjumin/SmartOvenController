import requests
import time
import json

def test_filesystem_upload():
    # 设备IP地址
    device_ip = "192.168.16.105"
    url = f"http://{device_ip}/update"
    
    # 创建一个测试文件
    test_file_path = "test_filesystem.bin"
    with open(test_file_path, "wb") as f:
        # 创建一个1MB的测试文件
        f.write(b"0" * 1024 * 1024)
    
    print(f"创建测试文件: {test_file_path}")
    
    # 准备上传文件
    with open(test_file_path, "rb") as f:
        files = {"filesystem": f}
        print("开始上传文件系统镜像...")
        
        try:
            # 发送POST请求上传文件
            response = requests.post(url, files=files, timeout=60)
            
            if response.status_code == 200:
                print("文件上传成功!")
                print("响应内容:", response.text)
            else:
                print(f"上传失败，状态码: {response.status_code}")
                print("响应内容:", response.text)
                
        except requests.exceptions.RequestException as e:
            print(f"请求发生错误: {e}")
        except Exception as e:
            print(f"发生未知错误: {e}")
    
    # 删除测试文件
    import os
    os.remove(test_file_path)
    print(f"删除测试文件: {test_file_path}")

if __name__ == "__main__":
    test_filesystem_upload()