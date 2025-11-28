import requests
import time

# 服务器地址
server_url = "http://192.168.16.100"

# 创建一个更大的测试文件（满足最小大小要求）
with open("test_filesystem.bin", "wb") as f:
    f.write(b"TEST_FILESYSTEM_DATA_" * 128)  # 创建2048字节的测试文件系统镜像

print("Created test filesystem file with sufficient size")

# 测试文件系统更新
def test_filesystem_update():
    print("\n=== Testing Filesystem Update ===")
    try:
        with open("test_filesystem.bin", "rb") as fs_file:
            files = {"filesystem": ("littlefs.bin", fs_file, "application/octet-stream")}
            print("Sending filesystem update request...")
            
            # 使用流式响应来处理SSE
            response = requests.post(
                f"{server_url}/filesystem_update",
                files=files,
                stream=True,
                timeout=30
            )
            
            print(f"Initial Response Status Code: {response.status_code}")
            print(f"Response Headers: {dict(response.headers)}")
            
            # 读取响应内容直到连接关闭
            try:
                for line in response.iter_lines():
                    if line:
                        decoded_line = line.decode('utf-8')
                        print(f"SSE Event: {decoded_line}")
            except requests.exceptions.ChunkedEncodingError as e:
                print(f"Stream ended (this is expected after completion): {e}")
            except Exception as e:
                print(f"Stream error: {e}")
                
    except Exception as e:
        print(f"Filesystem update failed: {e}")

if __name__ == "__main__":
    test_filesystem_update()