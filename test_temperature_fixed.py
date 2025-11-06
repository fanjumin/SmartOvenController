#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ä¿®å¤åçæ¸©åº¦è¯»åæµè¯èæ¬
éªè¯SPIæ¶åºä¼ååéè¯æºå¶çææ
"""

import serial
import time
import sys

def test_temperature_fixed():
    """æµè¯ä¿®å¤åçæ¸©åº¦è¯»ååè½"""
    
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
    
    print("\nð§ æµè¯ä¿®å¤åçæ¸©åº¦è¯»ååè½...")
    print("=" * 60)
    
    # æµè¯è¿ç»­è¯»åç¨³å®æ§
    success_count = 0
    total_tests = 10
    
    for i in range(total_tests):
        print(f"\nð ç¬¬{i+1}æ¬¡æµè¯...")
        
        # åéGET_RAW_TEMPå½ä»¤
        ser.write(b'GET_RAW_TEMP\n')
        time.sleep(0.5)
        
        # è¯»åååº
        response = ser.read_all().decode('utf-8', errors='ignore')
        
        # åæååº
        if "åå§å¼: 0x0" in response or "æ¸©åº¦ä½: 0" in response:
            print("â è¯»åå¤±è´¥: è¿å0å¼")
        elif "éè¯¯æ å¿ä½" in response and "1" in response:
            print("â è¯»åå¤±è´¥: ä¼ æå¨éè¯¯")
        elif "åå§å¼: 0x" in response and "æ¸©åº¦ä½:" in response:
            print("â è¯»åæå")
            
            # æåæ¸©åº¦æ°æ®
            lines = response.split('\n')
            for line in lines:
                if "åå§å¼:" in line:
                    print(f"   {line.strip()}")
                elif "æ¸©åº¦ä½:" in line:
                    print(f"   {line.strip()}")
                elif "æªæ ¡åæ¸©åº¦:" in line:
                    print(f"   {line.strip()}")
                elif "æ ¡ååæ¸©åº¦:" in line:
                    print(f"   {line.strip()}")
            
            success_count += 1
        else:
            print("â æªç¥ååºæ ¼å¼")
            print(f"ååºåå®¹: {response}")
        
        time.sleep(0.5)  # æµè¯é´é
    
    # ç»è®¡ç»æ
    print("\n" + "=" * 60)
    print("ð æµè¯ç»æç»è®¡:")
    print(f"æ»æµè¯æ¬¡æ°: {total_tests}")
    print(f"æåæ¬¡æ°: {success_count}")
    print(f"æåç: {success_count/total_tests*100:.1f}%")
    
    if success_count / total_tests >= 0.8:
        print("â æ¸©åº¦è¯»åç¨³å®æ§è¯å¥½")
    elif success_count / total_tests >= 0.5:
        print("â ï¸ æ¸©åº¦è¯»åç¨³å®æ§ä¸è¬ï¼å»ºè®®æ£æ¥ç¡¬ä»¶è¿æ¥")
    else:
        print("â æ¸©åº¦è¯»åç¨³å®æ§å·®ï¼éè¦è¿ä¸æ­¥è°è¯")
    
    # æµè¯GET_TEMPå½ä»¤
    print("\nð¡ï¸ æµè¯GET_TEMPå½ä»¤...")
    ser.write(b'GET_TEMP\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"æ¸©åº¦ååº: {response.strip()}")
    
    # æµè¯GET_STATUSå½ä»¤
    print("\nð æµè¯GET_STATUSå½ä»¤...")
    ser.write(b'GET_STATUS\n')
    time.sleep(0.5)
    response = ser.read_all().decode('utf-8', errors='ignore')
    print(f"ç¶æååº: {response.strip()}")
    
    ser.close()
    print("\nâ æµè¯å®æ")

if __name__ == "__main__":
    test_temperature_fixed()