#!/usr/bin/env python3
"""
æµè¯ç½é¡µæ¸©åº¦æ¾ç¤ºæ§è½
"""

import requests
import time
import json

def test_web_response_time():
    """æµè¯ç½é¡µååºæ¶é´"""
    print("=== ç½é¡µæ¸©åº¦æ¾ç¤ºæ§è½æµè¯ ===")
    
    device_ip = "192.168.16.104"
    urls_to_test = [
        "/",           # ä¸»é¡µ
        "/status",     # ç¶æAPI
        "/temperature", # æ¸©åº¦é¡µé¢
        "/api/temp"     # æ¸©åº¦API
    ]
    
    for url_path in urls_to_test:
        url = f"http://{device_ip}{url_path}"
        print(f"\næµè¯URL: {url}")
        
        try:
            # æµè¯ååºæ¶é´
            start_time = time.time()
            response = requests.get(url, timeout=10)
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000  # æ¯«ç§
            
            print(f"ååºæ¶é´: {response_time:.1f}ms")
            print(f"ç¶æç : {response.status_code}")
            print(f"åå®¹é¿åº¦: {len(response.text)} å­è")
            
            # å¦ææ¯JSONååºï¼æ¾ç¤ºæ¸©åº¦æ°æ®
            if response.headers.get('Content-Type', '').startswith('application/json'):
                try:
                    data = response.json()
                    if 'temperature' in data:
                        print(f"æ¸©åº¦æ°æ®: {data['temperature']}Â°C")
                except:
                    pass
            
            # æ¾ç¤ºå100ä¸ªå­ç¬¦çåå®¹
            content_preview = response.text[:100]
            print(f"åå®¹é¢è§: {content_preview}")
            
        except requests.exceptions.Timeout:
            print("â è¯·æ±è¶æ¶")
        except Exception as e:
            print(f"â è¯·æ±å¤±è´¥: {e}")

def test_multiple_requests():
    """æµè¯å¤æ¬¡è¯·æ±çæ§è½"""
    print("\n=== å¤æ¬¡è¯·æ±æ§è½æµè¯ ===")
    
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
            
            # æ¾ç¤ºæ¸©åº¦æ°æ®
            if response.status_code == 200:
                try:
                    data = response.json()
                    temp = data.get('temperature', 'æªç¥')
                    print(f"ç¬¬{i+1}æ¬¡è¯·æ±: {response_time:.1f}ms, æ¸©åº¦: {temp}Â°C")
                except:
                    print(f"ç¬¬{i+1}æ¬¡è¯·æ±: {response_time:.1f}ms")
            
            time.sleep(0.5)  # é´é0.5ç§
            
        except Exception as e:
            print(f"ç¬¬{i+1}æ¬¡è¯·æ±å¤±è´¥: {e}")
    
    # ç»è®¡ç»æ
    if response_times:
        avg_time = sum(response_times) / len(response_times)
        max_time = max(response_times)
        min_time = min(response_times)
        
        print(f"\nð æ§è½ç»è®¡:")
        print(f"å¹³åååºæ¶é´: {avg_time:.1f}ms")
        print(f"æå¿«ååºæ¶é´: {min_time:.1f}ms")
        print(f"ææ¢ååºæ¶é´: {max_time:.1f}ms")
        
        # å¤æ­æ§è½é®é¢
        if avg_time > 1000:
            print("ð´ ä¸¥éæ§è½é®é¢: ååºæ¶é´è¶è¿1ç§")
        elif avg_time > 500:
            print("ð¡ æ§è½é®é¢: ååºæ¶é´è¶è¿500ms")
        else:
            print("ð¢ æ§è½è¯å¥½: ååºæ¶é´æ­£å¸¸")

def check_web_interface():
    """æ£æ¥Webçé¢"""
    print("\n=== Webçé¢æ£æ¥ ===")
    
    device_ip = "192.168.16.104"
    
    try:
        # æ£æ¥ä¸»é¡µ
        response = requests.get(f"http://{device_ip}/", timeout=10)
        print(f"ä¸»é¡µç¶æ: {response.status_code}")
        
        # æ£æ¥æ¯å¦ææ¸©åº¦æ¾ç¤ºç¸å³çJavaScript
        if "temperature" in response.text.lower() or "temp" in response.text.lower():
            print("â é¡µé¢åå«æ¸©åº¦æ¾ç¤ºç¸å³ä»£ç ")
        else:
            print("â ï¸ é¡µé¢å¯è½ç¼ºå°æ¸©åº¦æ¾ç¤ºä»£ç ")
        
        # æ£æ¥æ¯å¦æèªå¨å·æ°æºå¶
        if "setInterval" in response.text or "setTimeout" in response.text:
            print("â é¡µé¢æèªå¨å·æ°æºå¶")
        else:
            print("â ï¸ é¡µé¢å¯è½ç¼ºå°èªå¨å·æ°")
            
    except Exception as e:
        print(f"â Webçé¢æ£æ¥å¤±è´¥: {e}")

if __name__ == "__main__":
    test_web_response_time()
    test_multiple_requests()
    check_web_interface()