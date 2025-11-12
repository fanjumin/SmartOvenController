import requests
from bs4 import BeautifulSoup

def find_upload_page():
    # 设备IP地址
    device_ip = "192.168.16.105"
    base_url = f"http://{device_ip}"
    
    # 获取主页内容
    try:
        response = requests.get(base_url, timeout=5)
        if response.status_code == 200:
            soup = BeautifulSoup(response.text, 'html.parser')
            
            # 查找所有链接
            links = soup.find_all('a', href=True)
            upload_links = []
            
            for link in links:
                href = link['href']
                if 'upload' in href.lower() or 'update' in href.lower() or 'filesystem' in href.lower():
                    upload_links.append(href)
            
            if upload_links:
                print("找到可能的上传页面链接:")
                for link in upload_links:
                    print(f"  - {link}")
                    
                    # 尝试访问这些链接
                    try:
                        page_url = f"{base_url}/{link}" if not link.startswith('http') else link
                        page_response = requests.get(page_url, timeout=5)
                        print(f"    状态码: {page_response.status_code}")
                    except requests.exceptions.RequestException as e:
                        print(f"    错误: {type(e).__name__}")
            else:
                print("未找到明显的上传页面链接")
                
                # 尝试一些常见的上传页面URL
                common_upload_pages = [
                    "upload.html",
                    "update.html",
                    "filesystem.html",
                    "file_upload.html",
                    "upload_filesystem.html"
                ]
                
                print("尝试常见的上传页面URL:")
                for page in common_upload_pages:
                    try:
                        page_url = f"{base_url}/{page}"
                        page_response = requests.get(page_url, timeout=5)
                        if page_response.status_code == 200:
                            print(f"  找到页面: {page} (状态码: {page_response.status_code})")
                        else:
                            print(f"  {page} (状态码: {page_response.status_code})")
                    except requests.exceptions.RequestException as e:
                        print(f"  {page} (错误: {type(e).__name__})")
        else:
            print(f"无法获取主页，状态码: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"请求发生错误: {type(e).__name__}")

if __name__ == "__main__":
    find_upload_page()