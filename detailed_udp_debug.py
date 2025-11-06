#!/usr/bin/env python3
"""
è¯¦ç»çUDPéä¿¡è°è¯èæ¬
ç¨äºè¯æ­è®¾å¤åç°åè®®çå·ä½é®é¢
"""

import socket
import time
import sys

def detailed_udp_debug():
    """è¯¦ç»çUDPéä¿¡è°è¯"""
    print("=== è¯¦ç»UDPéä¿¡è°è¯ ===")
    
    device_ip = "192.168.16.104"
    discovery_port = 8888
    
    # æµè¯1: æ£æ¥è®¾å¤UDPç«¯å£æ¯å¦ççå¼æ¾
    print("\n=== æµè¯1: UDPç«¯å£è¿æ¥æµè¯ ===")
    try:
        test_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        test_sock.settimeout(3)
        test_sock.connect((device_ip, discovery_port))
        print("â UDPç«¯å£8888è¿æ¥æå")
        test_sock.close()
    except Exception as e:
        print(f"â UDPç«¯å£8888è¿æ¥å¤±è´¥: {e}")
    
    # æµè¯2: åéUDPæ°æ®åå¹¶æ£æ¥æ¯å¦è¢«æ¥æ¶
    print("\n=== æµè¯2: UDPæ°æ®ååéæµè¯ ===")
    
    # åå»ºåésocket
    send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    send_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    # åå»ºæ¥æ¶socketï¼ç»å®å°ç¹å®ç«¯å£ï¼
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.settimeout(5)
    recv_port = 8889  # ä½¿ç¨è®¾å¤ææçååºç«¯å£
    recv_sock.bind(('', recv_port))
    
    print(f"æ¥æ¶socketç»å®å°ç«¯å£: {recv_port}")
    
    # åéåç°è¯·æ±
    discovery_message = "DISCOVER_SMARTOVEN"
    
    # åéå°è®¾å¤IP
    print(f"åéåç°è¯·æ±å° {device_ip}:{discovery_port}")
    send_sock.sendto(discovery_message.encode(), (device_ip, discovery_port))
    print("â ç´æ¥åéæå")
    
    # åéå¹¿æ­
    print(f"åéåç°è¯·æ±å° 255.255.255.255:{discovery_port}")
    send_sock.sendto(discovery_message.encode(), ("255.255.255.255", discovery_port))
    print("â å¹¿æ­åéæå")
    
    # çå¬ååº
    print(f"ç­å¾è®¾å¤ååºï¼ç«¯å£{recv_port}ï¼...")
    start_time = time.time()
    
    while time.time() - start_time < 10:
        try:
            data, addr = recv_sock.recvfrom(1024)
            response = data.decode('utf-8')
            
            print(f"\nâ æ¶å°è®¾å¤ååºï¼")
            print(f"æ¥æºIP: {addr[0]}")
            print(f"æ¥æºç«¯å£: {addr[1]}")
            print(f"ååºåå®¹: {response}")
            
            # æ£æ¥ååºæ ¼å¼
            if "DEVICE_INFO" in response:
                print("â ååºæ ¼å¼æ­£ç¡®ï¼åå«DEVICE_INFOï¼")
            else:
                print("â ï¸ ååºæ ¼å¼å¯è½ä¸æ­£ç¡®")
            
            send_sock.close()
            recv_sock.close()
            return True
            
        except socket.timeout:
            print("â æ¥æ¶è¶æ¶ï¼æªæ¶å°ååº")
            break
        except Exception as e:
            print(f"â æ¥æ¶éè¯¯: {e}")
            break
    
    send_sock.close()
    recv_sock.close()
    
    # æµè¯3: å°è¯ä¸åçæ¥æ¶ç«¯å£
    print("\n=== æµè¯3: å¤ç«¯å£çå¬æµè¯ ===")
    
    ports_to_try = [8888, 8889, 8890, 8891, 8892]
    
    for port in ports_to_try:
        print(f"\nå°è¯çå¬ç«¯å£ {port}...")
        
        recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        recv_sock.settimeout(3)
        
        try:
            recv_sock.bind(('', port))
            
            # åéè¯·æ±
            send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            send_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            send_sock.sendto(discovery_message.encode(), ("255.255.255.255", discovery_port))
            
            # çå¬ååº
            try:
                data, addr = recv_sock.recvfrom(1024)
                response = data.decode('utf-8')
                
                print(f"â å¨ç«¯å£ {port} æ¶å°ååºï¼")
                print(f"ååºåå®¹: {response}")
                
                send_sock.close()
                recv_sock.close()
                return True
                
            except socket.timeout:
                print(f"â ç«¯å£ {port} æªæ¶å°ååº")
            except Exception as e:
                print(f"â ç«¯å£ {port} æ¥æ¶éè¯¯: {e}")
            
            send_sock.close()
            recv_sock.close()
            
        except Exception as e:
            print(f"â æ æ³ç»å®å°ç«¯å£ {port}: {e}")
    
    return False

def test_tcp_connection():
    """æµè¯TCPè¿æ¥"""
    print("\n=== TCPè¿æ¥æµè¯ ===")
    
    device_ip = "192.168.16.104"
    tcp_port = 8888
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((device_ip, tcp_port))
        print("â TCPè¿æ¥æå")
        
        # åéç¶ææ¥è¯¢å½ä»¤
        command = "GET_STATUS\n"
        sock.send(command.encode())
        
        # æ¥æ¶ååº
        response = sock.recv(1024).decode('utf-8')
        print(f"è®¾å¤ååº: {response}")
        
        sock.close()
        return True
        
    except Exception as e:
        print(f"â TCPè¿æ¥å¤±è´¥: {e}")
        return False

if __name__ == "__main__":
    print("æºè½çµç¤ç®±UDPéä¿¡è¯¦ç»è°è¯å·¥å·")
    print("=" * 50)
    
    # è¿è¡è¯¦ç»è°è¯
    udp_success = detailed_udp_debug()
    tcp_success = test_tcp_connection()
    
    print("\n=== è°è¯æ»ç» ===")
    print(f"UDPéä¿¡: {'â æ­£å¸¸' if udp_success else 'â å¼å¸¸'}")
    print(f"TCPè¿æ¥: {'â æ­£å¸¸' if tcp_success else 'â å¼å¸¸'}")
    
    if not udp_success and tcp_success:
        print("\nð é®é¢åæ:")
        print("è®¾å¤TCPè¿æ¥æ­£å¸¸ï¼ä½UDPéä¿¡å¤±è´¥")
        print("å¯è½çåå :")
        print("1. è®¾å¤UDPç«¯å£çå¬éç½®é®é¢")
        print("2. è®¾å¤æ²¡ææ­£ç¡®å¤çUDPåç°è¯·æ±")
        print("3. ç½ç»é²ç«å¢æè·¯ç±å¨é»æ­¢äºUDPéä¿¡")
        print("4. è®¾å¤åºä»¶ä¸­çUDPå¤çé»è¾æé®é¢")
    
    print("\nè°è¯å®æ")