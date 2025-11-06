#!/usr/bin/env python3
"""
è®¾å¤åç°åè®®è°è¯èæ¬
ç¨äºè¯æ­è®¾å¤åç°åè®®çé®é¢
"""

import socket
import time
import sys

def debug_device_discovery():
    """è°è¯è®¾å¤åç°åè®®"""
    print("=== è®¾å¤åç°åè®®è°è¯ ===")
    
    # åå»ºUDP socketï¼ç»å®å°ä»»æç«¯å£ï¼è®©ç³»ç»åééæºç«¯å£ï¼
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(5)
    sock.bind(('', 0))  # ç»å®å°ä»»æå¯ç¨ç«¯å£
    
    # æµè¯1: ç´æ¥åéå°è®¾å¤IP
    print("\n=== æµè¯1: ç´æ¥åéå°è®¾å¤IP ===")
    device_ip = "192.168.16.104"
    discovery_port = 8888
    
    discovery_message = "DISCOVER_SMARTOVEN"
    
    print(f"åéåç°è¯·æ±å° {device_ip}:{discovery_port}")
    print(f"æ¶æ¯åå®¹: {discovery_message}")
    
    try:
        sock.sendto(discovery_message.encode(), (device_ip, discovery_port))
        print("â ç´æ¥åéæå")
        
        # çå¬ååº
        print("ç­å¾è®¾å¤ååº...")
        start_time = time.time()
        
        while time.time() - start_time < 10:
            try:
                data, addr = sock.recvfrom(1024)
                response = data.decode('utf-8')
                
                print(f"â æ¶å°è®¾å¤ååº")
                print(f"æ¥æºIP: {addr[0]}")
                print(f"æ¥æºç«¯å£: {addr[1]}")
                print(f"ååºåå®¹: {response}")
                
                return True
                
            except socket.timeout:
                print("â ç´æ¥åéæµè¯è¶æ¶ï¼æªæ¶å°ååº")
                break
            except Exception as e:
                print(f"â æ¥æ¶éè¯¯: {e}")
                break
                
    except Exception as e:
        print(f"â ç´æ¥åéå¤±è´¥: {e}")
    
    # æµè¯2: å¹¿æ­åé
    print("\n=== æµè¯2: å¹¿æ­åé ===")
    broadcast_address = "255.255.255.255"
    
    print(f"åéåç°è¯·æ±å° {broadcast_address}:{discovery_port}")
    print(f"æ¶æ¯åå®¹: {discovery_message}")
    
    try:
        sock.sendto(discovery_message.encode(), (broadcast_address, discovery_port))
        print("â å¹¿æ­åéæå")
        
        # çå¬ååº
        print("ç­å¾è®¾å¤ååº...")
        start_time = time.time()
        
        while time.time() - start_time < 10:
            try:
                data, addr = sock.recvfrom(1024)
                response = data.decode('utf-8')
                
                print(f"â æ¶å°è®¾å¤ååº")
                print(f"æ¥æºIP: {addr[0]}")
                print(f"æ¥æºç«¯å£: {addr[1]}")
                print(f"ååºåå®¹: {response}")
                
                return True
                
            except socket.timeout:
                print("â å¹¿æ­åéæµè¯è¶æ¶ï¼æªæ¶å°ååº")
                break
            except Exception as e:
                print(f"â æ¥æ¶éè¯¯: {e}")
                break
                
    except Exception as e:
        print(f"â å¹¿æ­åéå¤±è´¥: {e}")
    
    # æµè¯3: æ£æ¥UDPç«¯å£ç¶æ
    print("\n=== æµè¯3: UDPç«¯å£ç¶ææ£æ¥ ===")
    
    # æ£æ¥è®¾å¤UDPç«¯å£æ¯å¦å¼æ¾
    test_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    test_sock.settimeout(3)
    
    try:
        test_sock.connect((device_ip, discovery_port))
        print("â UDPç«¯å£8888å·²å¼æ¾")
    except:
        print("â UDPç«¯å£8888æªå¼æ¾")
    
    test_sock.close()
    
    # æµè¯4: ç½ç»è¿éæ§æ£æ¥
    print("\n=== æµè¯4: ç½ç»è¿éæ§æ£æ¥ ===")
    
    # æ£æ¥è®¾å¤æ¯å¦å¯è¾¾
    try:
        # ä½¿ç¨pingå½ä»¤æ£æ¥è¿éæ§
        import subprocess
        result = subprocess.run(['ping', '-n', '1', device_ip], 
                              capture_output=True, text=True, timeout=5)
        if result.returncode == 0:
            print("â è®¾å¤ç½ç»å¯è¾¾")
        else:
            print("â è®¾å¤ç½ç»ä¸å¯è¾¾")
            print(f"Pingè¾åº: {result.stdout}")
    except Exception as e:
        print(f"â ç½ç»è¿éæ§æ£æ¥å¤±è´¥: {e}")
    
    sock.close()
    return False

def test_tcp_connection():
    """æµè¯TCPè¿æ¥"""
    print("\n=== TCPè¿æ¥æµè¯ ===")
    
    device_ip = "192.168.16.104"
    tcp_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        
        print(f"å°è¯è¿æ¥å° {device_ip}:{tcp_port}")
        sock.connect((device_ip, tcp_port))
        print("â TCPè¿æ¥æå")
        
        # åéæµè¯å½ä»¤
        command = "GET_STATUS\n"
        sock.send(command.encode())
        print(f"åéå½ä»¤: {command.strip()}")
        
        # æ¥æ¶ååº
        response = sock.recv(1024).decode('utf-8')
        print(f"è®¾å¤ååº: {response}")
        
        sock.close()
        return True
        
    except Exception as e:
        print(f"â TCPè¿æ¥å¤±è´¥: {e}")
        return False

if __name__ == "__main__":
    print("æºè½çµç¤ç®±è®¾å¤åç°åè®®è°è¯å·¥å·")
    print("=" * 50)
    
    # è¿è¡è°è¯æµè¯
    discovery_success = debug_device_discovery()
    tcp_success = test_tcp_connection()
    
    print("\n=== è°è¯æ»ç» ===")
    print(f"è®¾å¤åç°åè®®: {'â æ­£å¸¸' if discovery_success else 'â å¼å¸¸'}")
    print(f"TCPè¿æ¥: {'â æ­£å¸¸' if tcp_success else 'â å¼å¸¸'}")
    
    if not discovery_success and tcp_success:
        print("\nð é®é¢åæ:")
        print("è®¾å¤TCPè¿æ¥æ­£å¸¸ï¼ä½è®¾å¤åç°åè®®å¤±è´¥")
        print("å¯è½çåå :")
        print("1. è®¾å¤UDPç«¯å£çå¬é®é¢")
        print("2. è®¾å¤æ²¡ææ­£ç¡®å¤çåç°è¯·æ±")
        print("3. ç½ç»é²ç«å¢æè·¯ç±å¨è®¾ç½®é»æ­¢äºUDPå¹¿æ­")
        print("4. è®¾å¤åºä»¶ä¸­çè®¾å¤åç°åè½æªå¯ç¨")
    elif discovery_success and tcp_success:
        print("\nâ æææµè¯éè¿ï¼è®¾å¤åç°åè®®å·¥ä½æ­£å¸¸")
    else:
        print("\nâ è®¾å¤è¿æ¥å­å¨ä¸¥éé®é¢")

    print("\nè°è¯å®æ")