#!/usr/bin/env python3
"""
æ£æ¥è®¾å¤æ§è½ç»è®¡ä¿¡æ¯
åæ¬åå­å ç¨ãæ¸©åº¦è¯»åæ§è½ç­
"""

import socket
import time

def get_performance_stats():
    """è·åè®¾å¤æ§è½ç»è®¡ä¿¡æ¯"""
    print("=== è®¾å¤æ§è½ç»è®¡æ£æ¥ ===")
    
    # è®¾å¤IPåç«¯å£
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        # åå»ºTCPè¿æ¥
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((device_ip, device_port))
        
        # æ¥æ¶æ¬¢è¿æ¶æ¯
        welcome = sock.recv(1024).decode('utf-8')
        print(f"è®¾å¤æ¬¢è¿æ¶æ¯: {welcome.strip()}")
        
        # åéGET_PERFORMANCEå½ä»¤
        command = "GET_PERFORMANCE\n"
        sock.send(command.encode('utf-8'))
        print(f"åéå½ä»¤: {command.strip()}")
        
        # æ¥æ¶æ§è½ç»è®¡ä¿¡æ¯
        performance_data = ""
        start_time = time.time()
        
        while time.time() - start_time < 5:  # æå¤ç­å¾5ç§
            try:
                data = sock.recv(1024).decode('utf-8')
                if data:
                    performance_data += data
                    # æ£æ¥æ¯å¦æ¶å°å®æ´ååº
                    if "æ¸©åº¦è¯»åæ§è½" in performance_data or "åå­è­¦å" in performance_data:
                        break
            except socket.timeout:
                break
        
        print("æ§è½ç»è®¡ä¿¡æ¯:")
        print(performance_data)
        
        # å¦ææ§è½ç»è®¡ä¸ºç©ºï¼å°è¯è·åè®¾å¤ç¶æ
        if not performance_data.strip():
            print("\nâ ï¸ æªæ¶å°æ§è½ç»è®¡ä¿¡æ¯ï¼å°è¯è·åè®¾å¤ç¶æ...")
            sock.send("GET_STATUS\n".encode('utf-8'))
            status_response = sock.recv(1024).decode('utf-8')
            print(f"è®¾å¤ç¶æ: {status_response.strip()}")
        
        sock.close()
        print("â æ§è½æ£æ¥å®æ")
        
    except Exception as e:
        print(f"â æ§è½æ£æ¥å¤±è´¥: {e}")

def test_temperature_response_time():
    """æµè¯æ¸©åº¦ååºæ¶é´"""
    print("\n=== æ¸©åº¦ååºæ¶é´æµè¯ ===")
    
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, device_port))
        
        # æ¥æ¶æ¬¢è¿æ¶æ¯
        welcome = sock.recv(1024).decode('utf-8')
        print(f"è®¾å¤è¿æ¥æå")
        
        # æµè¯å¤æ¬¡æ¸©åº¦è·åçååºæ¶é´
        response_times = []
        for i in range(5):
            start_time = time.time()
            sock.send("GET_TEMP\n".encode('utf-8'))
            response = sock.recv(1024).decode('utf-8')
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000  # è½¬æ¢ä¸ºæ¯«ç§
            response_times.append(response_time)
            
            print(f"ç¬¬{i+1}æ¬¡æ¸©åº¦è·å: {response.strip()} (ååºæ¶é´: {response_time:.1f}ms)")
            time.sleep(0.5)  # é´é0.5ç§
        
        # è®¡ç®å¹³åååºæ¶é´
        avg_response_time = sum(response_times) / len(response_times)
        print(f"\nð æ¸©åº¦è·åæ§è½ç»è®¡:")
        print(f"å¹³åååºæ¶é´: {avg_response_time:.1f}ms")
        print(f"æå¿«ååºæ¶é´: {min(response_times):.1f}ms")
        print(f"ææ¢ååºæ¶é´: {max(response_times):.1f}ms")
        
        sock.close()
        
    except Exception as e:
        print(f"â æ¸©åº¦ååºæ¶é´æµè¯å¤±è´¥: {e}")

def check_memory_usage():
    """æ£æ¥åå­ä½¿ç¨æåµ"""
    print("\n=== åå­ä½¿ç¨æåµæ£æ¥ ===")
    
    device_ip = "192.168.16.104"
    device_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, device_port))
        
        # æ¥æ¶æ¬¢è¿æ¶æ¯
        welcome = sock.recv(1024).decode('utf-8')
        
        # åéGET_STATUSå½ä»¤è·ååºæ¬ä¿¡æ¯
        sock.send("GET_STATUS\n".encode('utf-8'))
        status_response = sock.recv(1024).decode('utf-8')
        
        print("è®¾å¤åºæ¬ä¿¡æ¯:")
        if "TEMP:" in status_response:
            parts = status_response.strip().split(',')
            for part in parts:
                if ':' in part:
                    key, value = part.split(':', 1)
                    print(f"  {key}: {value}")
        
        # å°è¯è·åæ§è½ä¿¡æ¯
        sock.send("GET_PERFORMANCE\n".encode('utf-8'))
        time.sleep(1)  # ç»è®¾å¤æ¶é´å¤ç
        
        performance_data = ""
        try:
            performance_data = sock.recv(1024).decode('utf-8')
        except:
            pass
        
        if performance_data:
            print("\næ§è½ä¿¡æ¯:")
            print(performance_data)
        else:
            print("\nâ ï¸ æªæ¶å°æ§è½ç»è®¡ä¿¡æ¯")
        
        sock.close()
        
    except Exception as e:
        print(f"â åå­æ£æ¥å¤±è´¥: {e}")

if __name__ == "__main__":
    get_performance_stats()
    test_temperature_response_time()
    check_memory_usage()