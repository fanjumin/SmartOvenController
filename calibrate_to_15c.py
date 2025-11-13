"""
æ¸©åº¦ç²¾ç¡®æ ¡åå°15Â°Cèæ¬
ä¸é¨ç¨äºå°è®¾å¤æ¸©åº¦ä¼ æå¨æ ¡åå°15Â°C
"""

import serial
import time
import datetime
import json

def calibrate_to_15c():
    """å°æ¸©åº¦ç²¾ç¡®æ ¡åå°15Â°C"""
    print("=== æ¸©åº¦ç²¾ç¡®æ ¡åå°15Â°C ===")
    print("ç®æ æ¸©åº¦: 15.00Â°C")
    print("æ ¡åå½ä»¤: CALIBRATE_TEMP 15")
    print()
    
    try:
        # å°è¯è¿æ¥ä¸²å£
        print("æ­£å¨è¿æ¥ä¸²å£ COM11...")
        ser = serial.Serial('COM11', 115200, timeout=2)
        time.sleep(2)  # ç­å¾ä¸²å£åå§å
        print("ä¸²å£è¿æ¥æå")
        
        # åè·åå½åæ¸©åº¦
        print("\\nè·åå½åæ¸©åº¦...")
        ser.write(b'GET_TEMP\\r\\n')
        time.sleep(1)
        current_temp_response = ser.read(ser.in_waiting)
        print("å½åæ¸©åº¦ååº:")
        print(current_temp_response.decode('utf-8', errors='ignore'))
        
        # åéæ ¡åå½ä»¤
        print("\\nå¼å§æ¸©åº¦æ ¡åå°15Â°C...")
        ser.write(b'CALIBRATE_TEMP 15\\r\\n')
        time.sleep(3)  # ç­å¾æ ¡åå®æ
        
        # è¯»åæ ¡åååº
        response = b''
        start_time = time.time()
        while time.time() - start_time < 8:  # 8ç§è¶æ¶ï¼ç¡®ä¿å®æ´ååº
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
            time.sleep(0.1)
        
        if response:
            print("\\næ ¡åååº:")
            response_text = response.decode('utf-8', errors='ignore')
            print(response_text)
            
            # æ£æ¥æ ¡åæ¯å¦æå
            if "æ ¡åå®æ" in response_text or "Calibration" in response_text:
                print("\\nâ æ¸©åº¦æ ¡åæå!")
            else:
                print("\\nâ ï¸  æ ¡åååºå¼å¸¸ï¼è¯·æ£æ¥è®¾å¤ç¶æ")
        else:
            print("\\nâ æ æ ¡åååº")
        
        # éªè¯æ ¡åç»æ
        print("\\néªè¯æ ¡åç»æ...")
        ser.write(b'GET_TEMP\\r\\n')
        time.sleep(1)
        verify_response = ser.read(ser.in_waiting)
        print("æ ¡ååæ¸©åº¦:")
        print(verify_response.decode('utf-8', errors='ignore'))
        
        # è·åå®æ´ç¶æ
        print("\\nè·åè®¾å¤ç¶æ...")
        ser.write(b'GET_STATUS\\r\\n')
        time.sleep(1)
        status_response = ser.read(ser.in_waiting)
        print("è®¾å¤ç¶æ:")
        print(status_response.decode('utf-8', errors='ignore'))
        
        ser.close()
        
        # ä¿å­æ ¡åè®°å½
        calibration_record = {
            "target_temperature": 15.0,
            "calibration_time": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "device_id": "oven-8591756",
            "firmware_version": "0.8.1",
            "calibration_command": "CALIBRATE_TEMP 15",
            "response": response_text if response else "æ ååº"
        }
        
        # ä¿å­å°æä»¶
        with open('calibration_15c_record.json', 'w', encoding='utf-8') as f:
            json.dump(calibration_record, f, indent=2, ensure_ascii=False)
        
        print(f"\\nâ æ ¡åè®°å½å·²ä¿å­å° calibration_15c_record.json")
        print(f"ð æ ¡åæ¶é´: {calibration_record['calibration_time']}")
        
    except serial.SerialException as e:
        print(f"â ä¸²å£è¿æ¥å¤±è´¥: {e}")
        print("\\nå¯è½çåå :")
        print("1. ä¸²å£COM11è¢«å¶ä»ç¨åºå ç¨")
        print("2. è®¾å¤æªè¿æ¥æçµæºæªå¼å¯")
        print("3. ä¸²å£é©±å¨ç¨åºé®é¢")
        print("\\nè§£å³æ¹æ³:")
        print("1. å³é­å¶ä»å¯è½å ç¨ä¸²å£çç¨åº")
        print("2. æ£æ¥è®¾å¤è¿æ¥åçµæº")
        print("3. ç¨åéè¯")
        
    except Exception as e:
        print(f"â æ ¡åè¿ç¨ä¸­åçéè¯¯: {e}")

def check_web_temperature():
    """æ£æ¥Webé¡µé¢æ¸©åº¦æ¾ç¤º"""
    import requests
    
    try:
        print("\\næ£æ¥Webé¡µé¢æ¸©åº¦æ¾ç¤º...")
        response = requests.get('http://192.168.16.104/status', timeout=5)
        if response.status_code == 200:
            data = response.json()
            print(f"Webé¡µé¢æ¸©åº¦: {data.get('temperature', 'N/A')}Â°C")
            print(f"ç®æ æ¸©åº¦: {data.get('target_temperature', 'N/A')}Â°C")
            print(f"æ ¡åç¶æ: {data.get('calibrated', 'N/A')}")
        else:
            print(f"Webé¡µé¢è®¿é®å¤±è´¥: {response.status_code}")
    except Exception as e:
        print(f"Webé¡µé¢æ£æ¥å¤±è´¥: {e}")

if __name__ == "__main__":
    print("æ¸©åº¦ç²¾ç¡®æ ¡åèæ¬ v0.8.1")
    print("=" * 50)
    
    # æ§è¡æ ¡å
    calibrate_to_15c()
    
    # æ£æ¥Webé¡µé¢
    check_web_temperature()
    
    print("\\n" + "=" * 50)
    print("æ ¡åèæ¬æ§è¡å®æ")
    print("\\nåç»­æä½:")
    print("1. å¦æä¸²å£è¢«å ç¨ï¼è¯·ç¨åéè¯")
    print("2. æ ¡åæååï¼æ¸©åº¦åºæ¾ç¤ºä¸º15Â°Cå·¦å³")
    print("3. æ ¡ååæ°å·²èªå¨ä¿å­å°EEPROM")
    print("4. å¯éè¿Webé¡µé¢ http://192.168.16.104 æ¥çå®æ¶æ¸©åº¦")