import requests
import time
import os

def test_filesystem_update():
    # 设备IP地址（需要根据实际情况修改）
    device_ip = "192.168.4.1"  # 这是设备的默认IP地址
    url = f"http://{device_ip}/fs_update"
    
    # 创建一个测试文件
    test_file_path = "littlefs.bin"
    if not os.path.exists(test_file_path):
        with open(test_file_path, "wb") as f:
            # 创建一个1KB的测试文件
            f.write(b"0" * 1024)
    
    print(f"使用测试文件: {test_file_path}")
    
    # 准备上传文件
    with open(test_file_path, "rb") as f:
        # 使用正确的参数名 'littlefs'
        files = {"littlefs": f}
        print("开始上传文件系统镜像...")
        
        try:
            # 发送POST请求上传文件
            response = requests.post(url, files=files, timeout=30)
            
            print(f"状态码: {response.status_code}")
            print(f"响应内容: {response.text}")
            
            if response.status_code == 200:
                print("文件上传成功!")
            else:
                print(f"上传失败")
                
        except requests.exceptions.RequestException as e:
            print(f"请求发生错误: {e}")
        except Exception as e:
            print(f"发生未知错误: {e}")
    
    print("测试完成")

if __name__ == "__main__":
    test_filesystem_update()