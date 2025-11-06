#!/usr/bin/env python3
import serial
import time
import sys

def check_device_status():
    try:
        # æå¼ä¸²å£
        ser = serial.Serial('COM11', 115200, timeout=5)
        print("æåè¿æ¥å°COM11ç«¯å£")
        print("æ­£å¨è¯»åè®¾å¤ç¶æ...")
        print("-" * 50)
        
        # è¯»åè®¾å¤è¾åº
        start_time = time.time()
        reboot_count = 0
        last_reboot_time = start_time
        temperature_readings = []
        
        while time.time() - start_time < 30:  # çæ§30ç§
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"è®¾å¤è¾åº: {line}")
                    
                    # æ£æ¥æ¯å¦éå¯
                    if "æºè½çµç¤ç®±æ§å¶å¨" in line or "MAX6675" in line or "WiFi" in line:
                        current_time = time.time()
                        if current_time - last_reboot_time < 10:  # 10ç§ååæ¬¡çå°å¯å¨ä¿¡æ¯
                            reboot_count += 1
                            print(f"â ï¸  æ£æµå°è®¾å¤éå¯ (ç¬¬{reboot_count}æ¬¡)")
                        last_reboot_time = current_time
                    
                    # æ£æ¥æ¸©åº¦æ°æ®
                    if "Â°C" in line or "æ¸©åº¦" in line:
                        temperature_readings.append(line)
                        print(f"ð¡ï¸  æ¸©åº¦æ°æ®: {line}")
            
            time.sleep(0.1)
        
        ser.close()
        
        print("-" * 50)
        print("\nð è®¾å¤ç¶æåææ¥å:")
        print(f"1. éå¯æ¬¡æ°: {reboot_count} æ¬¡")
        print(f"2. æ¸©åº¦è¯»æ°æ°é: {len(temperature_readings)} ä¸ª")
        
        if reboot_count > 2:
            print("â è®¾å¤é¢ç¹éå¯ - å¯è½å­å¨ç¡¬ä»¶æè½¯ä»¶é®é¢")
        else:
            print("â è®¾å¤å¯å¨ç¨³å®")
            
        if len(temperature_readings) > 0:
            print("â æ¸©åº¦ä¼ æå¨å·¥ä½æ­£å¸¸")
            print(f"   æè¿æ¸©åº¦è¯»æ°: {temperature_readings[-1] if temperature_readings else 'æ '}")
        else:
            print("â æªæ£æµå°æ¸©åº¦æ°æ® - ä¼ æå¨å¯è½æé®é¢")
            
    except serial.SerialException as e:
        print(f"â æ æ³æå¼ä¸²å£: {e}")
        print("è¯·æ£æ¥:")
        print("1. COM11ç«¯å£æ¯å¦è¢«å¶ä»ç¨åºå ç¨")
        print("2. è®¾å¤æ¯å¦æ­£ç¡®è¿æ¥")
        print("3. ä¸²å£é©±å¨æ¯å¦æ­£å¸¸å®è£")
    except Exception as e:
        print(f"â åçéè¯¯: {e}")

if __name__ == "__main__":
    check_device_status()