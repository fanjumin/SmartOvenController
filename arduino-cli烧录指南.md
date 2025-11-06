# SmartOven æºè½çµç¤ç®±æ§å¶å¨ - arduino-cliç§å½æå

## ð å¿«éå¼å§ç§å½

### 1. æ£æ¥è®¾å¤è¿æ¥
```bash
# æ£æ¥å¯ç¨çå¼åæ¿åç«¯å£
arduino-cli board list
```
åºè¯¥æ¾ç¤ºç±»ä¼¼ï¼
```
COM11 serial   Serial Port (USB) Unknown
```

### 2. ç¼è¯ä»£ç 
```bash
# ç¼è¯SmartOvenController.ino
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

### 3. ç§å½å°è®¾å¤
```bash
# ç§å½å°COM11ç«¯å£
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

## ð å®æ´ç§å½æµç¨

### æ­¥éª¤1ï¼ç¯å¢åå¤
```bash
# 1. æ´æ°æ ¸å¿ç´¢å¼
arduino-cli core update-index

# 2. å®è£ESP8266æ ¸å¿
arduino-cli core install esp8266:esp8266

# 3. æ£æ¥å®è£çæ ¸å¿
arduino-cli core list
```

### æ­¥éª¤2ï¼å®è£å¿è¦çåº
```bash
# å®è£ESP8266WiFiåº
arduino-cli lib install "ESP8266WiFi"

# å®è£ESP8266WebServeråº
arduino-cli lib install "ESP8266WebServer"

# å®è£MAX6675åº
arduino-cli lib install "MAX6675"

# æ£æ¥å·²å®è£çåº
arduino-cli lib list
```

### æ­¥éª¤3ï¼ä¿®æ¹WiFiéç½®
ç¼è¾ `SmartOvenController.ino` æä»¶ï¼ä¿®æ¹ä»¥ä¸éç½®ï¼
```cpp
// ä¿®æ¹ä¸ºæ¨çå®éWiFiä¿¡æ¯
const char* sta_ssid = "æ¨çWiFiåç§°";
const char* sta_password = "æ¨çWiFiå¯ç ";
```

### æ­¥éª¤4ï¼ç¼è¯ä»£ç 
```bash
# åºæ¬ç¼è¯
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

# è¯¦ç»ç¼è¯ï¼æ¾ç¤ºæ´å¤ä¿¡æ¯ï¼
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --verbose SmartOvenController.ino

# å¸¦ä¼åç¼è¯
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-Os" SmartOvenController.ino
```

### æ­¥éª¤5ï¼ç§å½å°è®¾å¤
```bash
# åºæ¬ç§å½
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

# è¯¦ç»ç§å½ï¼æ¾ç¤ºè¿åº¦ï¼
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 --verbose SmartOvenController.ino

# å¼ºå¶ç§å½ï¼å¿½ç¥è­¦åï¼
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 --verify SmartOvenController.ino
```

## ð§ é«çº§ç§å½éé¡¹

### æ¹éç§å½èæ¬
åå»º `flash_all.bat` æ¹å¤çæä»¶ï¼
```batch
@echo off
echo å¼å§ç§å½SmartOvenæ§å¶å¨...

:: æ£æ¥è®¾å¤è¿æ¥
arduino-cli board list

:: ç¼è¯ä»£ç 
echo æ­£å¨ç¼è¯ä»£ç ...
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

if %errorlevel% neq 0 (
    echo ç¼è¯å¤±è´¥ï¼è¯·æ£æ¥éè¯¯ä¿¡æ¯
    pause
    exit /b 1
)

:: ç§å½å°è®¾å¤
echo æ­£å¨ç§å½å°COM11...
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

if %errorlevel% neq 0 (
    echo ç§å½å¤±è´¥ï¼è¯·æ£æ¥è®¾å¤è¿æ¥
    pause
    exit /b 1
)

echo ç§å½æåï¼
echo è¯·æå¼ä¸²å£çè§å¨æ¥çè®¾å¤ç¶æ
pause
```

### å¤è®¾å¤ç§å½
å¦æè¿æ¥äºå¤ä¸ªè®¾å¤ï¼
```bash
# æ£æ¥ææå¯ç¨ç«¯å£
arduino-cli board list

# ç§å½å°ç¹å®ç«¯å£
arduino-cli upload -p COM3 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
arduino-cli upload -p COM4 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

## ð ç§å½éªè¯

### 1. ä¸²å£çè§å¨éªè¯
```bash
# æå¼ä¸²å£çè§å¨ï¼æ³¢ç¹ç115200ï¼
arduino-cli monitor -p COM11 --config baudrate=115200
```

åºè¯¥çå°ç±»ä¼¼è¾åºï¼
```
â SmartOvenæ§å¶å¨å¯å¨æå
ð¡ WiFiè¿æ¥ä¸­...
ð è¿æ¥å°WiFi: æ¨çWiFiåç§°
ð è®¾å¤IPå°å: 192.168.1.100
ð è®¾å¤åç°æå¡å·²å¯å¨
```

### 2. å¿«ééªè¯èæ¬
åå»º `verify_flash.bat`ï¼
```batch
@echo off
echo éªè¯ç§å½ç»æ...

:: æ£æ¥è®¾å¤æ¯å¦ååº
arduino-cli board list | find "COM11"
if %errorlevel% neq 0 (
    echo éè¯¯ï¼COM11ç«¯å£æªæ£æµå°è®¾å¤
    pause
    exit /b 1
)

echo è®¾å¤æ£æµæ­£å¸¸ï¼æ­£å¨æ£æ¥ä¸²å£è¾åº...

:: ç­ææå¼ä¸²å£çè§å¨ï¼5ç§ï¼
start /B arduino-cli monitor -p COM11 --config baudrate=115200 --timeout 5

echo éªè¯å®æï¼
pause
```

## ð æéæé¤

### å¸¸è§é®é¢åè§£å³æ¹æ¡

#### 1. ç¼è¯éè¯¯ - åºç¼ºå¤±
```bash
# æ£æ¥ç¼ºå¤±çåº
arduino-cli lib search "åºåç§°"

# å®è£ç¹å®çæ¬çåº
arduino-cli lib install "åºåç§°@çæ¬å·"
```

#### 2. ç§å½å¤±è´¥ - ç«¯å£è¢«å ç¨
```bash
# æ£æ¥ç«¯å£ç¶æ
netstat -ano | findstr "COM11"

# å¼ºå¶å³é­å ç¨ç«¯å£çè¿ç¨
taskkill /F /PID è¿ç¨ID
```

#### 3. è®¾å¤æ æ³è¯å«
```bash
# éæ°æ«æè®¾å¤
arduino-cli board list --watch

# æ£æ¥é©±å¨ç¨åº
pnputil /enum-devices /connected | findstr "USB"
```

#### 4. ç§å½è¶æ¶
```bash
# å¢å è¶æ¶æ¶é´
arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 --timeout 60 SmartOvenController.ino

# éç½®è®¾å¤è¿å¥ç¼ç¨æ¨¡å¼
# NodeMCU: æä½FLASHæé®ï¼æRESETï¼æ¾å¼FLASH
```

### è°è¯å½ä»¤
```bash
# è¯¦ç»æ¨¡å¼æ¥çç¼è¯è¿ç¨
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --verbose SmartOvenController.ino

# æ¥çç¼è¯è¾åºç®å½
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-path ./build SmartOvenController.ino

# æ¸çç¼è¯ç¼å­
arduino-cli cache clean
```

## ð æ§è½ä¼å

### ç¼è¯ä¼åéé¡¹
```bash
# æå°å°ºå¯¸ä¼å
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-Os" SmartOvenController.ino

# è°è¯çæ¬ï¼å¸¦ç¬¦å·ï¼
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-g" SmartOvenController.ino

# æ§è½ä¼å
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-property "compiler.c.elf.flags=-O2" SmartOvenController.ino
```

### æ¹éç§å½ä¼å
åå»º `flash_optimized.bat`ï¼
```batch
@echo off
echo ä¼åç§å½æµç¨...

:: å¹¶è¡ç¼è¯åç§å½åå¤
start /B arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

:: ç­å¾ç¼è¯å®æ
timeout /t 10

:: æ¹éç§å½
for %%p in (COM3 COM4 COM5 COM6 COM7 COM8 COM9 COM10 COM11 COM12) do (
    arduino-cli board list | find "%%p" >nul
    if not errorlevel 1 (
        echo ç§å½å° %%p...
        arduino-cli upload -p %%p --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
    )
)

echo æ¹éç§å½å®æï¼
pause
```

## ð èªå¨åèæ¬

### å®æ´çç§å½èæ¬ `auto_flash.bat`
```batch
@echo off
title SmartOvenèªå¨ç§å½èæ¬
color 0A

echo ========================================
echo      SmartOvenæ§å¶å¨èªå¨ç§å½å·¥å·
echo ========================================
echo.

:: æ­¥éª¤1ï¼ç¯å¢æ£æ¥
echo [1/5] æ£æ¥ç¯å¢...
arduino-cli --help >nul 2>&1
if %errorlevel% neq 0 (
    echo éè¯¯ï¼arduino-cliæªå®è£ææªå¨PATHä¸­
    pause
    exit /b 1
)

:: æ­¥éª¤2ï¼è®¾å¤æ£æµ
echo [2/5] æ£æµè®¾å¤...
set DEVICE_FOUND=0
for /f "tokens=*" %%i in ('arduino-cli board list') do (
    echo %%i | find "COM" >nul
    if !errorlevel! equ 0 set DEVICE_FOUND=1
)

if %DEVICE_FOUND% equ 0 (
    echo è­¦åï¼æªæ£æµå°è®¾å¤ï¼è¯·æ£æ¥USBè¿æ¥
    echo ç»§ç»­æ§è¡ç¼è¯...
)

:: æ­¥éª¤3ï¼ç¼è¯ä»£ç 
echo [3/5] ç¼è¯ä»£ç ...
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
if %errorlevel% neq 0 (
    echo éè¯¯ï¼ç¼è¯å¤±è´¥
    pause
    exit /b 1
)

echo ç¼è¯æåï¼

:: æ­¥éª¤4ï¼ç§å½å°è®¾å¤
echo [4/5] ç§å½å°è®¾å¤...
if %DEVICE_FOUND% equ 1 (
    arduino-cli upload -p COM11 --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
    if %errorlevel% neq 0 (
        echo éè¯¯ï¼ç§å½å¤±è´¥
        pause
        exit /b 1
    )
    echo ç§å½æåï¼
else
    echo è·³è¿ç§å½ï¼æªæ£æµå°è®¾å¤ï¼
fi

:: æ­¥éª¤5ï¼éªè¯
echo [5/5] éªè¯ç§å½...
echo è¯·æå¨æå¼ä¸²å£çè§å¨æ¥çè®¾å¤ç¶æ
echo å½ä»¤ï¼arduino-cli monitor -p COM11 --config baudrate=115200

echo.
echo ========================================
echo     ç§å½æµç¨å®æï¼
echo ========================================
pause
```

## ð ææ¯æ¯æ

### è·åå¸®å©
```bash
# æ¥çarduino-cliå¸®å©
arduino-cli help
arduino-cli compile --help
arduino-cli upload --help

# æ¥çæ ¸å¿ä¿¡æ¯
arduino-cli core search esp8266
```

### æ¥å¿åè°è¯
```bash
# å¯ç¨è¯¦ç»æ¥å¿
arduino-cli --log-level debug compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino

# ä¿å­æ¥å¿å°æä»¶
arduino-cli --log-level debug --log-file arduino.log compile --fqbn esp8266:esp8266:nodemcuv2 SmartOvenController.ino
```

---

## ð¯ ç§å½æåæ å¿

å½æ¨çå°ä»¥ä¸è¾åºæ¶ï¼è¡¨ç¤ºç§å½æåï¼

```
â ç¼è¯æå - ä»£ç å¤§å°ï¼xxx bytes
â ç§å½æå - è®¾å¤å·²æ´æ°
â ä¸²å£è¾åºæ¾ç¤ºè®¾å¤æ­£å¸¸å¯å¨
â Webçé¢å¯ä»¥æ­£å¸¸è®¿é®
```

**ä½¿ç¨arduino-cliç§å½æ¯Arduino IDEæ´å¿«éãæ´ç¨³å®ï¼** ð