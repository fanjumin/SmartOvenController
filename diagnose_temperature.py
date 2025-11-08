#!/usr/bin/env python3

"""
æ¸©åº¦ä¼ æå¨è¯æ­èæ¬
ç¨äºè¯æ­MAX6675æ¸©åº¦ä¼ æå¨è¯»åé®é¢
"""

import serial
import time
import sys

def diagnose_temperature():
    """è¯æ­æ¸©åº¦ä¼ æå¨é®é¢"""
    
    # å°è¯è¿æ¥ä¸²å£
    try:
        ser = serial.Serial('COM11', 115200, timeout=2)
        print("â ä¸²å£è¿æ¥æå")
    except Exception as e:
        print(f"â ä¸²å£è¿æ¥å¤±è´¥: {e}")
        return
    
    # ç­å¾è®¾å¤å¯å¨
    time.sleep(2)
    
    # æ¸ç©ºç¼å²åº
    ser.reset_input_buffer()
    
    print("\nð å¼å§æ¸©åº¦ä¼ æå¨è¯æ­...")
    print("=" * 50)
    
    # 1. è·åè®¾å¤ç¶æ
    print("\n1. è·åè®¾å¤ç¶æ...")
    ser.write(b'GET_STATUS\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"ç¶æååº: {response}")
    
    # 2. è·ååå§æ¸©åº¦æ°æ®
    print("\n2. è·ååå§æ¸©åº¦æ°æ®...")
    ser.write(b'GET_RAW_TEMP\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"åå§æ¸©åº¦ååº:\n{response}")
    
    # 3. æ£æ¥SPIéä¿¡
    print("\n3. æ£æ¥SPIéä¿¡ç¶æ...")
    ser.write(b'GET_PERFORMANCE\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"æ§è½ä¿¡æ¯:\n{response}")
    
    # 4. å¤æ¬¡è¯»åæ¸©åº¦æ°æ®ä»¥ç¡®è®¤é®é¢
    print("\n4. è¿ç»­è¯»åæ¸©åº¦æ°æ®ï¼5æ¬¡ï¼...")
    for i in range(5):
        ser.write(b'GET_RAW_TEMP\n')
        time.sleep(0.3)
        response = ser.read_all().decode('utf-8', errors='ignore')
        print(f"ç¬¬{i+1}æ¬¡è¯»å:\n{response}")
        time.sleep(0.2)
    
    # 5. æ£æ¥å¼èç¶æ
    print("\n5. æ£æ¥å¼èéç½®...")
    print("MAX6675å¼èéç½®:")
    print("  - THERMO_CLK (æ¶é): GPIO14 (D5)")
    print("  - THERMO_CS (çé): GPIO12 (D6)")
    print("  - THERMO_DO (æ°æ®): GPIO13 (D7)")
    print("  - çµæº: 3.3V")
    print("  - æ¥å°: GND")
    
    # 6. è¯æ­å»ºè®®
    print("\n6. è¯æ­å»ºè®®:")
    print("=" * 50)
    
    # åæååºæ°æ®
    if "åå§å¼: 0x0" in response or "æ¸©åº¦ä½: 0" in response:
        print("â é®é¢ç¡®è®¤: æ¸©åº¦ä¼ æå¨è¿åå¨0æ°æ®")
        print("\nð§ å¯è½çè§£å³æ¹æ¡:")
        print("1. æ£æ¥MAX6675æ¨¡åççµæºè¿æ¥ï¼3.3VåGNDï¼")
        print("2. éªè¯ç­çµå¶æ¯å¦æ­£ç¡®æå¥MAX6675æ¨¡å")
        print("3. æ£æ¥SPIå¼èè¿æ¥ï¼D5, D6, D7ï¼")
        print("4. ç¡®è®¤MAX6675æ¨¡åæ¯å¦æ­£å¸¸å·¥ä½")
        print("5. å°è¯æ´æ¢ç­çµå¶æMAX6675æ¨¡å")
    elif "éè¯¯æ å¿ä½" in response and "1" in response:
        print("â é®é¢ç¡®è®¤: æ¸©åº¦ä¼ æå¨éä¿¡éè¯¯")
        print("\nð§ å¯è½çè§£å³æ¹æ¡:")
        print("1. æ£æ¥ç­çµå¶è¿æ¥æ¯å¦æ¾å¨")
        print("2. éªè¯ç­çµå¶ç±»åæ¯å¦æ­£ç¡®ï¼Kåï¼")
        print("3. æ£æ¥MAX6675æ¨¡åæ¯å¦æå")
    else:
        print("â æ¸©åº¦ä¼ æå¨éä¿¡æ­£å¸¸")
        print("\nð§ å¦æä»æé®é¢ï¼è¯·æ£æ¥:")
        print("1. ç­çµå¶æ¯å¦æ¥è§¦è¯å¥½")
        print("2. ç¯å¢æ¸©åº¦æ¯å¦å¨æµéèå´å")
        print("3. æ ¡ååæ°æ¯å¦æ­£ç¡®")
    
    print("\nð ç¡¬ä»¶æ£æ¥æ¸å:")
    print("  - [ ] MAX6675æ¨¡åVCCè¿æ¥å°3.3V")
    print("  - [ ] MAX6675æ¨¡åGNDè¿æ¥å°GND")
    print("  - [ ] CLKå¼èè¿æ¥å°GPIO14 (D5)")
    print("  - [ ] CSå¼èè¿æ¥å°GPIO12 (D6)")
    print("  - [ ] DOå¼èè¿æ¥å°GPIO13 (D7)")
    print("  - [ ] ç­çµå¶æ­£ç¡®æå¥MAX6675æ¨¡å")
    print("  - [ ] ç­çµå¶æ¢å¤´æ¥è§¦è¯å¥½")
    
    ser.close()
    print("\nâ è¯æ­å®æ")

if __name__ == "__main__":
    diagnose_temperature()