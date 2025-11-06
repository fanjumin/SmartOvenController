#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
æºè½çµç¤ç®±æ§å¶å¨ - ç¼è¯åä¸ä¼ å·¥å·
"""

import os
import sys
import subprocess
import platform

def check_arduino_ide():
    """æ£æ¥Arduino IDEæ¯å¦å®è£"""
    possible_paths = [
        r"C:\Program Files (x86)\Arduino\arduino.exe",
        r"C:\Program Files\Arduino\arduino.exe",
        r"D:\Program Files (x86)\Arduino\arduino.exe",
        r"D:\Program Files\Arduino\arduino.exe",
    ]
    
    for path in possible_paths:
        if os.path.exists(path):
            return path
    return None

def check_platformio():
    """æ£æ¥PlatformIOæ¯å¦å®è£"""
    try:
        result = subprocess.run(["pio", "--version"], capture_output=True, text=True)
        if result.returncode == 0:
            return "pio"
    except FileNotFoundError:
        pass
    
    try:
        result = subprocess.run(["python", "-m", "platformio", "--version"], capture_output=True, text=True)
        if result.returncode == 0:
            return "python -m platformio"
    except FileNotFoundError:
        pass
    
    return None

def check_source_files():
    """æ£æ¥æºæä»¶æ¯å¦å­å¨"""
    source_files = [
        "src/SmartOvenController.ino",
        "platformio.ini",
        "data/index.html",
        "data/login.html"
    ]
    
    missing_files = []
    for file in source_files:
        if not os.path.exists(file):
            missing_files.append(file)
    
    return missing_files

def main():
    print("=" * 50)
    print("æºè½çµç¤ç®±æ§å¶å¨ - ç¼è¯åä¸ä¼ å·¥å·")
    print("=" * 50)
    print()
    
    # æ£æ¥ç³»ç»
    print("ð æ£æ¥ç³»ç»ç¯å¢...")
    print(f"   æä½ç³»ç»: {platform.system()} {platform.release()}")
    print(f"   å·¥ä½ç®å½: {os.getcwd()}")
    print()
    
    # æ£æ¥ç¼è¯å·¥å·
    print("ð§ æ£æ¥ç¼è¯å·¥å·...")
    
    arduino_path = check_arduino_ide()
    if arduino_path:
        print(f"   â Arduino IDE: {arduino_path}")
    else:
        print("   â Arduino IDE: æªæ¾å°")
    
    pio_tool = check_platformio()
    if pio_tool:
        print(f"   â PlatformIO: {pio_tool}")
    else:
        print("   â PlatformIO: æªæ¾å°")
    
    print()
    
    # æ£æ¥æºæä»¶
    print("ð æ£æ¥æºæä»¶...")
    missing_files = check_source_files()
    if missing_files:
        print("   â ç¼ºå°ä»¥ä¸æä»¶:")
        for file in missing_files:
            print(f"      - {file}")
    else:
        print("   â æææºæä»¶é½å­å¨")
    
    print()
    
    # æä¾ç¼è¯æå
    print("ð ç¼è¯åä¸ä¼ æå:")
    print("=" * 40)
    
    if arduino_path:
        print("\nð¯ æ¹æ³1: ä½¿ç¨Arduino IDE")
        print("   1. æå¼Arduino IDE")
        print("   2. æä»¶ â æå¼ â éæ© src/SmartOvenController.ino")
        print("   3. å·¥å· â å¼åæ¿ â ESP8266 Boards â NodeMCU 1.0")
        print("   4. å·¥å· â ç«¯å£ â éæ©æ­£ç¡®çCOMç«¯å£")
        print("   5. ç¹å» â éªè¯æé®ç¼è¯ä»£ç ")
        print("   6. ç¹å» â ä¸ä¼ æé®ä¸ä¼ å°è®¾å¤")
    
    if pio_tool:
        print("\nð¯ æ¹æ³2: ä½¿ç¨PlatformIO")
        print(f"   1. è¿è¡: {pio_tool} run")
        print(f"   2. è¿è¡: {pio_tool} run --target upload")
    
    if not arduino_path and not pio_tool:
        print("\nâ ï¸  æªæ¾å°ç¼è¯å·¥å·ï¼è¯·åå®è£ä»¥ä¸å·¥å·ä¹ä¸:")
        print("   - Arduino IDE + ESP8266å¼åæ¿æ¯æ")
        print("   - PlatformIO (VS Codeæä»¶æå½ä»¤è¡)")
    
    print("\nð¯ æ¹æ³3: æå¨ç¼è¯ä¸ä¼ ")
    print("   1. å®è£Arduino IDE")
    print("   2. æ·»å ESP8266å¼åæ¿æ¯æ:")
    print("      - æä»¶ â é¦éé¡¹ â éå å¼åæ¿ç®¡çå¨ç½å")
    print("      - æ·»å : http://arduino.esp8266.com/stable/package_esp8266com_index.json")
    print("   3. å·¥å· â å¼åæ¿ â å¼åæ¿ç®¡çå¨ â æç´¢esp8266 â å®è£")
    print("   4. æç§æ¹æ³1çæ­¥éª¤ç¼è¯ä¸ä¼ ")
    
    print()
    print("ð± è®¾å¤è¿æ¥ä¿¡æ¯:")
    print("   - å¼åæ¿: NodeMCU ESP8266")
    print("   - ç«¯å£: COM11 (æ ¹æ®å®éæåµä¿®æ¹)")
    print("   - ä¸ä¼ éåº¦: 115200 baud")
    print("   - Flashæ¨¡å¼: DIO")
    print("   - Flashå¤§å°: 4MB")
    
    print()
    print("ð ç¸å³ææ¡£:")
    print("   - ä»£ç ç§å½æå.md")
    print("   - ä»£ç ä¸ä¼ æå.md")
    print("   - å®è£æå.md")
    
    print()
    print("ð¡ æç¤º:")
    print("   - ä¸ä¼ åç¡®ä¿è®¾å¤è¿å¥ç¼ç¨æ¨¡å¼")
    print("   - å¦æä¸ä¼ å¤±è´¥ï¼å°è¯éå¯è®¾å¤ææ´æ¢USBç«¯å£")
    print("   - ä¸ä¼ æååï¼è®¾å¤ä¼èªå¨éå¯")
    
    print()
    print("=" * 50)
    print("ç¼è¯ä¸ä¼ å·¥å·æ£æ¥å®æï¼")
    print("=" * 50)

if __name__ == "__main__":
    main()