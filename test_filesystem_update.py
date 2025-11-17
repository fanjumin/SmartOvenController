import requests
import time
import os

def test_filesystem_update():
    # 设备IP地址（需要根据实际情况修改）
    device_ip = "192.168.16.100"  # 这是设备的实际IP地址

    print("=== 文件系统更新测试 ===")

    # 1. 先检查设备状态
    print("\n1. 检查设备状态...")
    try:
        status_url = f"http://{device_ip}/status"
        response = requests.get(status_url, timeout=10)
        if response.status_code == 200:
            print("   设备可访问")
            print(f"   状态响应: {response.text[:200]}...")
        else:
            print(f"   设备状态检查失败: {response.status_code}")
            return
    except Exception as e:
        print(f"   设备连接失败: {e}")
        print("   请确保设备已启动并连接到网络")
        return

    # 2. 检查OTA页面
    print("\n2. 检查OTA页面...")
    try:
        ota_url = f"http://{device_ip}/ota_update"
        response = requests.get(ota_url, timeout=10)
        if response.status_code == 200:
            print("   OTA页面可访问")
        else:
            print(f"   OTA页面访问失败: {response.status_code}")
    except Exception as e:
        print(f"   OTA页面访问失败: {e}")

    url = f"http://{device_ip}/update"

    # 使用编译出来的文件系统镜像
    test_file_path = ".pio/build/nodemcuv2/littlefs.bin"
    if not os.path.exists(test_file_path):
        print(f"   错误: 找不到文件系统镜像文件 {test_file_path}")
        print("   请先运行 'pio run --target buildfs' 编译文件系统")
        return

    print(f"\n3. 使用测试文件: {test_file_path}")
    print(f"   文件大小: {os.path.getsize(test_file_path)} 字节")

    # 准备上传文件
    with open(test_file_path, "rb") as f:
        # 使用正确的参数名 'filesystem'
files = {"filesystem": f}
        print("\n4. 开始上传文件系统镜像...")

        try:
            # 发送POST请求上传文件，使用更短的超时时间
            response = requests.post(url, files=files, timeout=30)

            print(f"状态码: {response.status_code}")
            print(f"响应内容: {response.text}")

            if response.status_code == 200:
                print("文件上传成功!")
                print("设备应该会在几秒后重启以应用更新")
                print("请等待设备重启后检查版本号是否更新为v0.8.3")
            else:
                print(f"上传失败，状态码: {response.status_code}")

        except requests.exceptions.Timeout:
            print("请求超时 - 可能是文件太大或网络问题")
            print("建议：通过网页界面手动上传文件系统镜像")
        except requests.exceptions.ConnectionError:
            print("连接错误 - 设备可能已重启")
            print("请等待几秒后重新检查设备状态")
        except requests.exceptions.RequestException as e:
            print(f"请求发生错误: {e}")
        except Exception as e:
            print(f"发生未知错误: {e}")

    print("\n=== 测试完成 ===")

if __name__ == "__main__":
    test_filesystem_update()