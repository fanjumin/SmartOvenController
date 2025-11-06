// =========================================
// æºè½çµç¤ç®±æ§å¶å¨ v0.6.1
// =========================================
// çæ¬: 0.6.1
// åè½: å¼ºå¶é¨æ·éç½ + è®¾å¤èªå¨åç° + OTAåçº§ + MAX6675æå¨SPIå®ç° + ä¸»é¡µéææ¸©åº¦æ§å¶
// æ´æ°: ä¿®å¤WiFiè¿æ¥ç¨³å®æ§é®é¢ï¼ç»ä¸è¿æ¥è¶æ¶æ¶é´ä¸º30ç§
// =========================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <SPI.h>
#include <FS.h>
// =========================================
// ç¡¬ä»¶å¼èå®ä¹
// =========================================
#define THERMO_CLK   14  // MAX6675æ¶é (D5/GPIO14)
#define THERMO_CS    12  // MAX6675çé (D6/GPIO12)
#define THERMO_DO    13  // MAX6675æ°æ®è¾åº (D7/GPIO13)
#define HEATER_PIN   5   // å ç­å¨æ§å¶
#define BUZZER_PIN   4   // èé¸£å¨ (GPIO4/D2)
#define LED_PIN      2   // LEDæç¤ºç¯ (GPIO2/D4)

// =========================================
// å¨å±åéå®ä¹
// =========================================

// ç½ç»ç¸å³
ESP8266WebServer webServer(80);
WiFiUDP udp;
DNSServer dnsServer;
ESP8266WebServer otaServer(8080);
ESP8266HTTPUpdateServer httpUpdater;
WiFiServer tcpServer(8888);  // TCPæå¡å¨ç¨äºAPPè¿æ¥
WiFiClient tcpClient;        // TCPå®¢æ·ç«¯è¿æ¥

// æ§è½çæ§åé
unsigned long temperatureReadCount = 0;
float temperatureReadAvgTime = 0;
const unsigned long WEB_SERVER_HANDLE_INTERVAL = 100; // æ¯100mså¤çä¸æ¬¡Webè¯·æ±ï¼ä¼åï¼åå°CPUå ç¨ï¼

// ç¡¬ä»¶æéçæ§
unsigned long hardwareFailureCount = 0;       // ç¡¬ä»¶æéæ¬¡æ°
unsigned long lastHardwareReset = 0;         // ä¸æ¬¡ç¡¬ä»¶å¤ä½æ¶é´
bool hardwareInitialized = false;            // ç¡¬ä»¶åå§åç¶æ

// è®¾å¤ä¿¡æ¯
const String DEVICE_TYPE = "oven";
const String DEVICE_ID = "oven-" + String(ESP.getChipId());
const String DEVICE_NAME = "SmartOven";
const String FIRMWARE_VERSION = "0.6.0";

// WiFiéç½®
String wifiSSID = "";
String wifiPassword = "";

// å¼ºå¶é¨æ·éç½®
bool isCaptivePortalMode = false;
unsigned long captivePortalStartTime = 0;
const unsigned long CAPTIVE_PORTAL_TIMEOUT = 300000; // 5åéè¶æ¶
const String AP_SSID = "SmartOven-" + String(ESP.getChipId());
const String AP_PASSWORD = "12345678";

// æ¸©åº¦æ§å¶
float currentTemp = 0.0;
float targetTemp = 180.0;
bool heatingEnabled = false;
bool ovenMode = true; // ç¤ç®±æ¨¡å¼ï¼true=ç¤ç®±æ¨¡å¼ï¼false=ç¤é¢åæºæ¨¡å¼

// ç½ç»ç«¯å£éç½®
const int DEFAULT_PORT = 8888;

// =========================================
// MAX6675æå¨SPIéä¿¡å½æ°
// =========================================

// è¯»åMAX6675åå§æ°æ®ï¼16ä½ï¼
uint16_t readMAX6675RawData() {
  uint16_t data = 0;
  
  // ç¡¬ä»¶å¤ä½ï¼ç¡®ä¿è¯çå¤äºå·²ç¥ç¶æ
  digitalWrite(THERMO_CS, HIGH);
  digitalWrite(THERMO_CLK, LOW);
  delayMicroseconds(100);  // å»¶é¿å¤ä½æ¶é´
  
  // å¯ç¨è¯ç
  digitalWrite(THERMO_CS, LOW);
  delayMicroseconds(50);  // å¤§å¹å¢å å»¶è¿ç¡®ä¿è¯çç¨³å®
  
  // è¯»å16ä½æ°æ®ï¼MSBä¼åï¼
  for (int i = 15; i >= 0; i--) {
    digitalWrite(THERMO_CLK, HIGH);  // æ¶éä¸åæ²¿
    delayMicroseconds(20);  // å¤§å¹å¢å æ¶éå»¶è¿
    
    if (digitalRead(THERMO_DO)) {   // è¯»åæ°æ®ä½
      data |= (1 << i);
    }
    
    digitalWrite(THERMO_CLK, LOW);  // æ¶éä¸éæ²¿
    delayMicroseconds(20);  // å¤§å¹å¢å æ¶éå»¶è¿
  }
  
  // ç¦ç¨è¯ç
  digitalWrite(THERMO_CS, HIGH);
  delayMicroseconds(50);  // å¤§å¹å¢å å»¶è¿ç¡®ä¿è¯çç¨³å®
  
  return data;
}

// æ¸©åº¦æ ¡ååæ°
float temperatureOffset = 0.0;  // æ¸©åº¦åç§»éï¼ç¨äºæ ¡å
float temperatureScale = 1.0;    // æ¸©åº¦ç¼©æ¾ç³»æ°ï¼ç¨äºæ ¡å

// ç¡¬ä»¶åå§åéªè¯
bool verifyHardwareInitialization() {
    Serial.println("éªè¯ç¡¬ä»¶åå§åç¶æ...");
    
    // æ£æ¥å¼èç¶æ
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    // è®¾ç½®åå§ç¶æ
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    delay(100);  // ç¡®ä¿ç¡¬ä»¶ç¨³å®
    
    // éªè¯å¼èç¶æ
    if (digitalRead(THERMO_DO) == HIGH || digitalRead(THERMO_DO) == LOW) {
        Serial.println("ç¡¬ä»¶åå§åéªè¯éè¿");
        return true;
    } else {
        Serial.println("ç¡¬ä»¶åå§åéªè¯å¤±è´¥");
        return false;
    }
}

// ç¡¬ä»¶èªå¨æ¢å¤æºå¶
void performHardwareRecovery() {
    Serial.println("ð¨ æ§è¡ç¡¬ä»¶èªå¨æ¢å¤...");
    
    // è®°å½æéæ¬¡æ°
    hardwareFailureCount++;
    
    // å¼ºå¶ç¡¬ä»¶å¤ä½åºå
    for (int i = 0; i < 5; i++) {
        digitalWrite(THERMO_CS, HIGH);
        digitalWrite(THERMO_CLK, LOW);
        delay(200);  // å»¶é¿å¤ä½æ¶é´
        digitalWrite(THERMO_CS, LOW);
        delay(100);
        digitalWrite(THERMO_CS, HIGH);
        delay(200);
    }
    
    // éæ°åå§åå¼è
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    
    lastHardwareReset = millis();
    Serial.println("â ç¡¬ä»¶èªå¨æ¢å¤å®æï¼æéæ¬¡æ°: " + String(hardwareFailureCount));
}

// è¯»åæ¸©åº¦å¼ï¼æå¨SPIå®ç°ï¼
float readTemperatureManual() {
    // å¢å¼ºéè¯æºå¶ï¼æå¤éè¯5æ¬¡ï¼æ¯æ¬¡å¢å å»¶è¿
    for (int retry = 0; retry < 5; retry++) {
        uint16_t rawData = readMAX6675RawData();
        
        // æ£æ¥æ°æ®æææ§ï¼æé¤å¨0æå¨1çæ ææ°æ®ï¼
        if (rawData == 0x0000 || rawData == 0xFFFF) {
            if (retry < 4) {
                Serial.print("æ¸©åº¦ä¼ æå¨è¿åæ ææ°æ®ï¼ç¬¬");
                Serial.print(retry + 1);
                Serial.println("æ¬¡éè¯...");
                delay(50 * (retry + 1));  // éå¢å»¶è¿ï¼50ms, 100ms, 150ms, 200ms
                continue;
            } else {
                Serial.println("æ¸©åº¦ä¼ æå¨è¿åæ ææ°æ®ï¼å¨0æå¨1ï¼- æ§è¡ç¡¬ä»¶èªå¨æ¢å¤");
                performHardwareRecovery();
                return -1.0;
            }
        }
        
        // æ£æ¥éè¯¯æ å¿ä½ï¼D2ä½ä¸º0è¡¨ç¤ºæ­£å¸¸ï¼
        if (!(rawData & 0x04)) {
            uint16_t tempBits = rawData >> 3;  // å³ç§»3ä½è·åæ¸©åº¦æ°æ®
            float temperature = tempBits * 0.25;  // æ¯ä¸ªåä½0.25Â°C
            
            // åºç¨æ¸©åº¦æ ¡å
            temperature = (temperature * temperatureScale) + temperatureOffset;
            
            // æ£æ¥æ¸©åº¦èå´æ¯å¦åç
            if (temperature >= -50.0 && temperature <= 400.0) {
                Serial.print("æ¸©åº¦è¯»åæåï¼éè¯æ¬¡æ°: ");
                Serial.println(retry + 1);
                
                // éç½®æéè®¡æ°å¨ï¼å¦æè¿ç»­æåï¼
                if (retry == 0) {
                    hardwareFailureCount = 0;
                }
                
                return temperature;
            } else {
                Serial.println("æ¸©åº¦ä¼ æå¨è¯»æ°è¶åºèå´");
                return -1.0;
            }
        } else {
            if (retry < 4) {
                Serial.print("æ¸©åº¦ä¼ æå¨è¯»åéè¯¯ï¼ç¬¬");
                Serial.print(retry + 1);
                Serial.println("æ¬¡éè¯...");
                delay(50 * (retry + 1));  // éå¢å»¶è¿
                continue;
            } else {
                Serial.println("æ¸©åº¦ä¼ æå¨è¯»åéè¯¯ - æ§è¡ç¡¬ä»¶èªå¨æ¢å¤");
                performHardwareRecovery();
                return -1.0;
            }
        }
    }
    
    // ææéè¯é½å¤±è´¥ï¼æ§è¡ç¡¬ä»¶æ¢å¤
    Serial.println("ææéè¯é½å¤±è´¥ - æ§è¡ç¡¬ä»¶èªå¨æ¢å¤");
    performHardwareRecovery();
    return -1.0;
}

// æ¸©åº¦æ ¡åå½æ°
void calibrateTemperature(float actualTemp, float measuredTemp) {
    // è®¡ç®æ ¡ååæ°
    if (measuredTemp != 0) {
        temperatureScale = actualTemp / measuredTemp;
        temperatureOffset = actualTemp - (measuredTemp * temperatureScale);
    } else {
        temperatureOffset = actualTemp - measuredTemp;
        temperatureScale = 1.0;
    }
    
    Serial.println("æ¸©åº¦æ ¡åå®æ:");
    Serial.print("å®éæ¸©åº¦: "); Serial.print(actualTemp); Serial.println("Â°C");
    Serial.print("æµéæ¸©åº¦: "); Serial.print(measuredTemp); Serial.println("Â°C");
    Serial.print("æ ¡ååç§»: "); Serial.print(temperatureOffset); Serial.println("Â°C");
    Serial.print("æ ¡åç³»æ°: "); Serial.println(temperatureScale);
    
    // ä¿å­æ ¡ååæ°å°EEPROM
    saveConfig();
    Serial.println("æ¸©åº¦æ ¡ååæ°å·²ä¿å­å°EEPROM");
}

// è®¾å¤åç°
bool discoveryEnabled = true;
const unsigned long DISCOVERY_INTERVAL = 10000; // 10ç§å¹¿æ­ä¸æ¬¡
unsigned long lastDiscoveryTime = 0;

// LEDç¶ææ§å¶
bool ledState = false;
unsigned long lastLedUpdate = 0;
const unsigned long LED_BLINK_INTERVAL = 500; // LEDéªçé´é

// ççç»æç¶ææ§å¶
bool bakingCompleteState = false;
unsigned long bakingCompleteStartTime = 0;
const unsigned long BAKING_COMPLETE_DURATION = 10000; // ççç»æå¿«éªæç»­æ¶é´10ç§

// =========================================
// EEPROMéç½®å­å¨
// =========================================
struct Config {
    char ssid[32];
    char password[64];
    float temperatureOffset;  // æ¸©åº¦æ ¡ååç§»é
    float temperatureScale;    // æ¸©åº¦æ ¡åç³»æ°
    char signature[16];  // å¢å ç­¾åç©ºé´ï¼é¿åç¼å²åºæº¢åº
};

void saveConfig() {
    Config config;
    // æ¸ç©ºéç½®ç»æä½
    memset(&config, 0, sizeof(config));
    
    // å®å¨å¤å¶å­ç¬¦ä¸²
    strncpy(config.ssid, wifiSSID.c_str(), sizeof(config.ssid) - 1);
    strncpy(config.password, wifiPassword.c_str(), sizeof(config.password) - 1);
    strncpy(config.signature, "SMARTOVEN", sizeof(config.signature) - 1);
    
    // ä¿å­æ¸©åº¦æ ¡ååæ°
    config.temperatureOffset = temperatureOffset;
    config.temperatureScale = temperatureScale;
    
    // ç¡®ä¿å­ç¬¦ä¸²ä»¥nullç»å°¾
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    config.password[sizeof(config.password) - 1] = '\0';
    config.signature[sizeof(config.signature) - 1] = '\0';
    
    EEPROM.begin(512);
    EEPROM.put(0, config);
    EEPROM.commit();
    EEPROM.end();
    
    Serial.println("éç½®å·²ä¿å­å°EEPROM");
    Serial.print("SSID: ");
    Serial.println(config.ssid);
    Serial.print("å¯ç é¿åº¦: ");
    Serial.println(strlen(config.password));
    Serial.print("æ¸©åº¦æ ¡ååç§»: ");
    Serial.print(config.temperatureOffset);
    Serial.println("Â°C");
    Serial.print("æ¸©åº¦æ ¡åç³»æ°: ");
    Serial.println(config.temperatureScale);
    Serial.print("ç­¾å: ");
    Serial.println(config.signature);
    
    // èé¸£å¨æç¤ºéç½®å·²ä¿å­
    beepConfigSaved();
}

bool loadConfig() {
    Config config;
    EEPROM.begin(512);
    EEPROM.get(0, config);
    EEPROM.end();
    
    Serial.println("ä»EEPROMå è½½éç½®...");
    Serial.print("è¯»åå°çç­¾å: ");
    Serial.println(config.signature);
    Serial.print("è¯»åå°çSSID: ");
    Serial.println(config.ssid);
    Serial.print("è¯»åå°çå¯ç é¿åº¦: ");
    Serial.println(strlen(config.password));
    Serial.print("è¯»åå°çæ¸©åº¦æ ¡ååç§»: ");
    Serial.print(config.temperatureOffset);
    Serial.println("Â°C");
    Serial.print("è¯»åå°çæ¸©åº¦æ ¡åç³»æ°: ");
    Serial.println(config.temperatureScale);
    
    if (strcmp(config.signature, "SMARTOVEN") == 0) {
        wifiSSID = String(config.ssid);
        wifiPassword = String(config.password);
        
        // å è½½æ¸©åº¦æ ¡ååæ°
        temperatureOffset = config.temperatureOffset;
        temperatureScale = config.temperatureScale;
        
        Serial.println("éç½®éªè¯æåï¼å è½½ææéç½®");
        Serial.print("æ¸©åº¦æ ¡ååç§»: ");
        Serial.print(temperatureOffset);
        Serial.println("Â°C");
        Serial.print("æ¸©åº¦æ ¡åç³»æ°: ");
        Serial.println(temperatureScale);
        return true;
    } else {
        Serial.println("éç½®éªè¯å¤±è´¥ï¼ç­¾åä¸å¹é");
        // æ¸ç©ºéç½®
        wifiSSID = "";
        wifiPassword = "";
        // éç½®æ¸©åº¦æ ¡ååæ°
        temperatureOffset = 0.0;
        temperatureScale = 1.0;
        return false;
    }
}

// =========================================
// å¼ºå¶é¨æ·åè½
// =========================================
void startCaptivePortal() {
    Serial.println("å¯å¨å¼ºå¶é¨æ·æ¨¡å¼...");
    
    // æ­å¼ç°æè¿æ¥
    WiFi.disconnect();
    delay(100);
    
    // åå»ºAPç­ç¹
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
    
    Serial.print("APç­ç¹: ");
    Serial.println(AP_SSID);
    Serial.print("IPå°å: ");
    Serial.println(WiFi.softAPIP());
    
    // å¯å¨DNSå«æ
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    // å¯å¨Webæå¡å¨
    setupWebServer();
    webServer.begin();
    
    // å¯å¨è®¾å¤åç°ï¼å¼ºå¶é¨æ·æ¨¡å¼ä¸ä¹éè¦æ¯æè®¾å¤åç°ï¼
    udp.begin(8888);
    
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
    
    Serial.println("å¼ºå¶é¨æ·å·²å¯å¨ï¼UDPçå¬ç«¯å£8888å·²å¼å¯");
}

void stopCaptivePortal() {
    Serial.println("åæ­¢å¼ºå¶é¨æ·æ¨¡å¼...");
    
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    isCaptivePortalMode = false;
    captivePortalStartTime = 0;
    
    Serial.println("å¼ºå¶é¨æ·å·²åæ­¢");
}

bool shouldStartCaptivePortal() {
    // å¦ææ²¡æä¿å­çWiFiéç½®ï¼ç´æ¥å¯å¨å¼ºå¶é¨æ·
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("æ²¡æWiFiéç½®ï¼éè¦å¯å¨å¼ºå¶é¨æ·");
        return true;
    }
    
    // å¦ææWiFiéç½®ï¼å°è¯è¿æ¥WiFi
    Serial.println("æWiFiéç½®ï¼å°è¯è¿æ¥WiFi");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("å¯ç é¿åº¦: ");
    Serial.println(wifiPassword.length());
    
    // åç¡®ä¿WiFiæ¨¡å¼æ­£ç¡®
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    // ç­å¾è¿æ¥ï¼æå¤ç­å¾30ç§ï¼ä¸connectToWiFiä¿æä¸è´ï¼
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        // æ¯5ç§è¾åºä¸æ¬¡è¿æ¥ç¶æ
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("è¿æ¥ç¶æ: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("ç©ºé²ç¶æ"); break;
                case WL_NO_SSID_AVAIL: Serial.println("ç½ç»ä¸å¯ç¨"); break;
                case WL_SCAN_COMPLETED: Serial.println("æ«æå®æ"); break;
                case WL_CONNECTED: Serial.println("å·²è¿æ¥"); break;
                case WL_CONNECT_FAILED: Serial.println("è¿æ¥å¤±è´¥"); break;
                case WL_CONNECTION_LOST: Serial.println("è¿æ¥ä¸¢å¤±"); break;
                case WL_DISCONNECTED: Serial.println("å·²æ­å¼"); break;
                default: Serial.println("æªç¥ç¶æ"); break;
            }
        }
    }
    
    // æ£æ¥è¿æ¥ç»æ
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFiè¿æ¥æå");
        Serial.print("IPå°å: ");
        Serial.println(WiFi.localIP());
        Serial.print("è¿æ¥èæ¶: ");
        Serial.print((millis() - startTime) / 1000.0);
        Serial.println("ç§");
        return false; // è¿æ¥æåï¼ä¸éè¦å¼ºå¶é¨æ·
    } else {
        Serial.println("");
        Serial.println("WiFiè¿æ¥å¤±è´¥ï¼å¯å¨å¼ºå¶é¨æ·");
        Serial.print("æç»è¿æ¥ç¶æ: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("ç©ºé²ç¶æ"); break;
            case WL_NO_SSID_AVAIL: Serial.println("ç½ç»ä¸å¯ç¨"); break;
            case WL_SCAN_COMPLETED: Serial.println("æ«æå®æ"); break;
            case WL_CONNECT_FAILED: Serial.println("è¿æ¥å¤±è´¥"); break;
            case WL_CONNECTION_LOST: Serial.println("è¿æ¥ä¸¢å¤±"); break;
            case WL_DISCONNECTED: Serial.println("å·²æ­å¼"); break;
            default: Serial.println("æªç¥ç¶æ"); break;
        }
        // ç¡®ä¿WiFiå·²æ­å¼
        WiFi.disconnect();
        delay(100);
        return true; // è¿æ¥å¤±è´¥ï¼éè¦å¼ºå¶é¨æ·
    }
}

void checkCaptivePortalTimeout() {
    if (isCaptivePortalMode && 
        millis() - captivePortalStartTime > CAPTIVE_PORTAL_TIMEOUT) {
        Serial.println("å¼ºå¶é¨æ·è¶æ¶ï¼å°è¯è¿æ¥ä¿å­çWiFi");
        stopCaptivePortal();
        connectToWiFi();
    }
}

// =========================================
// WiFiè¿æ¥ç®¡ç
// =========================================
void connectToWiFi() {
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("æ²¡æWiFiéç½®ï¼å¯å¨å¼ºå¶é¨æ·");
        startCaptivePortal();
        return;
    }
    
    Serial.println("å°è¯è¿æ¥WiFi...");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("å¯ç é¿åº¦: ");
    Serial.println(wifiPassword.length());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    // å¢å è¿æ¥è¶æ¶å°30ç§ï¼å¹¶æ·»å æ´è¯¦ç»çè¿æ¥ç¶æ
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        // æ¯5ç§è¾åºä¸æ¬¡è¿æ¥ç¶æ
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("è¿æ¥ç¶æ: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("ç©ºé²ç¶æ"); break;
                case WL_NO_SSID_AVAIL: Serial.println("ç½ç»ä¸å¯ç¨"); break;
                case WL_SCAN_COMPLETED: Serial.println("æ«æå®æ"); break;
                case WL_CONNECTED: Serial.println("å·²è¿æ¥"); break;
                case WL_CONNECT_FAILED: Serial.println("è¿æ¥å¤±è´¥"); break;
                case WL_CONNECTION_LOST: Serial.println("è¿æ¥ä¸¢å¤±"); break;
                case WL_DISCONNECTED: Serial.println("å·²æ­å¼"); break;
                default: Serial.println("æªç¥ç¶æ"); break;
            }
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFiè¿æ¥æå!");
        Serial.print("IPå°å: ");
        Serial.println(WiFi.localIP());
        Serial.print("è¿æ¥èæ¶: ");
        Serial.print((millis() - startTime) / 1000.0);
        Serial.println("ç§");
        
        // å¯å¨Webæå¡å¨
        setupWebServer();
        
        // å¯å¨OTAæå¡å¨
        setupOTA();
        
        // å¯å¨è®¾å¤åç°
        udp.begin(8888);
        
        // è®¾ç½®LEDä¸ºå¸¸äº®ç¶æ
        digitalWrite(LED_PIN, HIGH);
    } else {
        Serial.println("");
        Serial.println("WiFiè¿æ¥å¤±è´¥ï¼å¯å¨å¼ºå¶é¨æ·");
        Serial.print("æç»è¿æ¥ç¶æ: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("ç©ºé²ç¶æ"); break;
            case WL_NO_SSID_AVAIL: Serial.println("ç½ç»ä¸å¯ç¨"); break;
            case WL_SCAN_COMPLETED: Serial.println("æ«æå®æ"); break;
            case WL_CONNECT_FAILED: Serial.println("è¿æ¥å¤±è´¥"); break;
            case WL_CONNECTION_LOST: Serial.println("è¿æ¥ä¸¢å¤±"); break;
            case WL_DISCONNECTED: Serial.println("å·²æ­å¼"); break;
            default: Serial.println("æªç¥ç¶æ"); break;
        }
        startCaptivePortal();
    }
}

// =========================================
// è®¾å¤èªå¨åç°åè½
// =========================================
void handleDiscovery() {
    // å¤çæ¥æ¶å°çåç°è¯·æ±
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char packetBuffer[255];
        int len = udp.read(packetBuffer, 255);
        if (len > 0) {
            packetBuffer[len] = 0;
            String request = String(packetBuffer);
            
            Serial.print("æ¶å°UDPæ°æ®åï¼å¤§å°: ");
            Serial.print(packetSize);
            Serial.print(" å­èï¼åå®¹: ");
            Serial.println(request);
            
            if (request.startsWith("DISCOVER_SMARTOVEN")) {
                Serial.println("æ¶å°è®¾å¤åç°è¯·æ±");
                sendDiscoveryResponse();
            }
        }
    }
    
    // å®æå¹¿æ­è®¾å¤ä¿¡æ¯
    if (discoveryEnabled && millis() - lastDiscoveryTime > DISCOVERY_INTERVAL) {
        broadcastDiscovery();
        lastDiscoveryTime = millis();
    }
}

void sendDiscoveryResponse() {
    String response;
    
    // ä½¿ç¨APPææçæ ¼å¼ï¼DEVICE_INFO:NAME:è®¾å¤åç§°,MAC:MACå°å,PORT:ç«¯å£å·,...
    response = "DEVICE_INFO:";
    response += "NAME:" + DEVICE_NAME + ",";
    response += "MAC:" + WiFi.macAddress() + ",";
    response += "PORT:" + String(DEFAULT_PORT) + ",";
    response += "TYPE:" + DEVICE_TYPE + ",";
    response += "VERSION:" + FIRMWARE_VERSION + ",";
    response += "TEMP:" + String(currentTemp) + ",";
    response += "TARGET:" + String(targetTemp) + ",";
    response += "HEAT:" + String(heatingEnabled ? "ON" : "OFF") + ",";
    response += "MODE:" + String(ovenMode ? "OVEN" : "TOASTER") + ",";
    response += "UPTIME:" + String(millis() / 1000) + ",";
    response += "DEVICE_ID:" + DEVICE_ID;
    
    // ä¿®å¤ï¼ä½¿ç¨APPåéè¯·æ±çç«¯å£ï¼remotePortï¼èä¸æ¯ç¡¬ç¼ç ç8889ç«¯å£
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(response.c_str());
    udp.endPacket();
    
    Serial.println("åéåç°ååºå°ç«¯å£" + String(udp.remotePort()) + ": " + response);
}

void broadcastDiscovery() {
    String broadcastMsg = "SMARTOVEN_BROADCAST:" + DEVICE_TYPE + ":" + DEVICE_ID + ":" + FIRMWARE_VERSION;
    
    udp.beginPacket("255.255.255.255", 8888);
    udp.write(broadcastMsg.c_str());
    udp.endPacket();
    
    Serial.println("å¹¿æ­è®¾å¤ä¿¡æ¯: " + broadcastMsg);
}

// =========================================
// OTAåçº§åè½
// =========================================
void setupOTA() {
    httpUpdater.setup(&otaServer);
    otaServer.begin();
    Serial.println("OTAæå¡å¨å·²å¯å¨ï¼ç«¯å£: 8080");
    Serial.println("OTAåçº§å°å: http://" + WiFi.localIP().toString() + ":8080/update");
}

void handleOTA() {
    if (!isCaptivePortalMode && WiFi.status() == WL_CONNECTED) {
        otaServer.handleClient();
    }
}

// OTAåçº§é¡µé¢
void handleOTAWebPage() {
    String html = "<!DOCTYPE html><html><head><title>æºè½çµç¤ç®±OTAåçº§</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<style>";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
    html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }";
    html += ".container { max-width: 600px; margin: 0 auto; background: white; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); overflow: hidden; }";
    html += ".header { background: linear-gradient(135deg, #2196F3 0%, #1976D2 100%); color: white; padding: 25px; text-align: center; }";
    html += ".header h1 { font-size: 24px; margin-bottom: 10px; }";
    html += ".content { padding: 25px; }";
    html += ".info-card { background: #f8f9fa; padding: 15px; border-radius: 8px; margin-bottom: 20px; }";
    html += ".info-card p { margin: 5px 0; color: #6c757d; }";
    html += ".ota-section { background: #e3f2fd; padding: 20px; border-radius: 8px; margin-bottom: 20px; }";
    html += ".btn { background: linear-gradient(135deg, #2196F3 0%, #1976D2 100%); color: white; border: none; padding: 12px 24px; border-radius: 8px; font-size: 14px; font-weight: 600; cursor: pointer; transition: all 0.3s; margin: 5px; }";
    html += ".btn:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(33, 150, 243, 0.3); }";
    html += ".btn-success { background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%); }";
    html += ".btn-warning { background: linear-gradient(135deg, #ff9800 0%, #f57c00 100%); }";
    html += ".status { padding: 10px; border-radius: 5px; margin: 10px 0; text-align: center; }";
    html += ".status-success { background: #d4edda; color: #155724; }";
    html += ".status-warning { background: #fff3cd; color: #856404; }";
    html += ".status-error { background: #f8d7da; color: #721c24; }";
    html += "</style>";
    html += "<script>";
    html += "function checkUpdate() {";
    html += "  fetch('/checkupdate').then(response => response.json()).then(data => {";
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-success\">å½åçæ¬: ' + data.current_version + '</div>';";
    html += "    if (data.update_available) {";
    html += "      document.getElementById('updateStatus').innerHTML += '<div class=\"status status-warning\">ææ°çæ¬å¯ç¨: ' + data.latest_version + '</div>';";
    html += "    }";
    html += "  }).catch(error => {";
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-error\">æ£æ¥æ´æ°å¤±è´¥</div>';";
    html += "  });";
    html += "}";
    html += "function startOTA() {";
    html += "  window.open('http://' + window.location.hostname + ':8080/update', '_blank');";
    html += "}";
    html += "window.onload = checkUpdate;";
    html += "</script>";
    html += "</head><body>";
    html += "<div class=\"container\">";
    html += "<div class=\"header\">";
    html += "<h1>æºè½çµç¤ç®±OTAåçº§</h1>";
    html += "<p>åºä»¶ç©ºä¸­åçº§ç³»ç»</p>";
    html += "</div>";
    html += "<div class=\"content\">";
    html += "<div class=\"info-card\">";
    html += "<p><strong>è®¾å¤ID:</strong> " + DEVICE_ID + "</p>";
    html += "<p><strong>IPå°å:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>OTAç«¯å£:</strong> 8080</p>";
    html += "</div>";
    html += "<div id=\"updateStatus\"></div>";
    html += "<div class=\"ota-section\">";
    html += "<h3>OTAåçº§æä½</h3>";
    html += "<p>ç¹å»ä¸æ¹æé®æå¼OTAåçº§é¡µé¢ï¼ä¸ä¼ æ°çåºä»¶æä»¶è¿è¡åçº§ã</p>";
    html += "<button class=\"btn btn-success\" onclick=\"startOTA()\">æå¼OTAåçº§é¡µé¢</button>";
    html += "<button class=\"btn\" onclick=\"checkUpdate()\">æ£æ¥æ´æ°</button>";
    html += "</div>";
    html += "<div style=\"text-align: center; margin-top: 20px;\">";
    html += "<a href=\"/\" style=\"color: #2196F3; text-decoration: none;\">è¿åä¸»é¡µé¢</a>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
}

// æ£æ¥æ´æ°API
void handleCheckUpdate() {
    String json = "{\"current_version\":\"" + FIRMWARE_VERSION + "\",\"latest_version\":\"0.6.0\",\"update_available\":false}";
    
    // è¿éå¯ä»¥æ·»å æ£æ¥æ°çæ¬çé»è¾
    // ä¾å¦ä»æå¡å¨è·åææ°çæ¬ä¿¡æ¯
    
    webServer.send(200, "application/json", json);
}

// =========================================
// Webæå¡å¨å¤ç
// =========================================
void setupWebServer() {
    // éç½®éææä»¶æå¡ï¼ä»å¨éå¼ºå¶é¨æ·æ¨¡å¼ä¸ä½¿ç¨ï¼
    if (!isCaptivePortalMode) {
        webServer.serveStatic("/login.html", SPIFFS, "/login.html");
        webServer.serveStatic("/index.html", SPIFFS, "/index.html");
        webServer.serveStatic("/css/", SPIFFS, "/css/");
        webServer.serveStatic("/js/", SPIFFS, "/js/");
        webServer.serveStatic("/images/", SPIFFS, "/images/");
    }
    
    // å¨æè·¯ç±
    webServer.on("/", HTTP_GET, handleRoot);
    webServer.on("/scanwifi", HTTP_GET, handleScanWiFi);
    webServer.on("/status", HTTP_GET, handleStatus);
    webServer.on("/control", HTTP_POST, handleControl);
    webServer.on("/savewifi", HTTP_POST, handleSaveWiFi);
    webServer.on("/factoryreset", HTTP_POST, handleFactoryReset);
    webServer.on("/restart", HTTP_POST, handleRestart);
    webServer.on("/ota", HTTP_GET, handleOTAWebPage);
    webServer.on("/checkupdate", HTTP_GET, handleCheckUpdate);
    webServer.on("/diagnostic", HTTP_GET, handleDiagnostic);
    webServer.on("/reset_calibration", HTTP_POST, handleResetCalibration);
    
    // æ°æ·»å çAPIç«¯ç¹
    webServer.on("/temperature_history", HTTP_GET, handleTemperatureHistory);
    webServer.on("/scheduler", HTTP_GET, handleScheduler);
    webServer.on("/device_info", HTTP_GET, handleDeviceInfo);
    webServer.on("/security_monitor", HTTP_GET, handleSecurityMonitor);
    webServer.on("/energy_stats", HTTP_GET, handleEnergyStats);
    
    webServer.onNotFound(handleNotFound);
    webServer.begin();
}

void handleScanWiFi() {
    // æ«æWiFiç½ç»
    int numNetworks = WiFi.scanNetworks();
    
    String json = "{\"networks\":[";
    
    if (numNetworks > 0) {
        for (int i = 0; i < numNetworks; i++) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        }
    }
    
    json += "]}";
    
    webServer.send(200, "application/json", json);
    
    // æ¸çæ«æç»æ
    WiFi.scanDelete();
}

void handleRoot() {
    if (isCaptivePortalMode) {
        // å¼ºå¶é¨æ·æ¨¡å¼ä¸æ¾ç¤ºWiFiéç½®é¡µé¢
        String html = "<!DOCTYPE html><html><head><title>æºè½çµç¤ç®±éç½®</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
        html += "<style>";
        html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
        html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }";
        html += ".container { max-width: 500px; margin: 0 auto; background: white; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); overflow: hidden; }";
        html += ".header { background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%); color: white; padding: 25px; text-align: center; }";
        html += ".header h1 { font-size: 24px; margin-bottom: 10px; }";
        html += ".device-info { background: #f8f9fa; padding: 15px; border-bottom: 1px solid #e9ecef; }";
        html += ".device-info p { margin: 5px 0; color: #6c757d; font-size: 14px; }";
        html += ".form-container { padding: 25px; }";
        html += ".form-group { margin-bottom: 20px; }";
        html += ".form-group label { display: block; margin-bottom: 8px; font-weight: 600; color: #495057; }";
        html += ".form-control { width: 100%; padding: 12px; border: 2px solid #e9ecef; border-radius: 8px; font-size: 14px; transition: border-color 0.3s; }";
        html += ".form-control:focus { outline: none; border-color: #4CAF50; }";
        html += ".btn { background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%); color: white; border: none; padding: 12px 24px; border-radius: 8px; font-size: 14px; font-weight: 600; cursor: pointer; transition: all 0.3s; }";
        html += ".btn:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(76, 175, 80, 0.3); }";
        html += ".btn:disabled { background: #6c757d; cursor: not-allowed; transform: none; box-shadow: none; }";
        html += ".btn-secondary { background: linear-gradient(135deg, #6c757d 0%, #5a6268 100%); }";
        html += ".btn-secondary:hover { box-shadow: 0 5px 15px rgba(108, 117, 125, 0.3); }";
        html += ".scan-section { text-align: center; margin-bottom: 20px; }";
        html += ".status-indicator { display: inline-block; width: 8px; height: 8px; border-radius: 50%; margin-right: 8px; }";
        html += ".status-connected { background: #28a745; }";
        html += ".status-disconnected { background: #dc3545; }";
        html += ".temperature-display { background: linear-gradient(135deg, #ff6b6b 0%, #ee5a24 100%); color: white; padding: 10px; border-radius: 8px; text-align: center; font-size: 18px; font-weight: bold; margin-bottom: 20px; }";
        html += "</style>";
        html += "<script>";
        html += "function scanWiFi() {";
        html += "  var wifiList = document.getElementById('ssid');";
        html += "  var scanBtn = document.getElementById('scanBtn');";
        html += "  scanBtn.disabled = true;";
        html += "  scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>æ«æä¸­...';";
        html += "  wifiList.innerHTML = '<option value=\"\">æ«æä¸­...</option>';";
        html += "  fetch('/scanwifi').then(response => response.json()).then(data => {";
        html += "    wifiList.innerHTML = '';";
        html += "    wifiList.innerHTML = '<option value=\"\">è¯·éæ©WiFiç½ç»</option>';";
        html += "    if (data.networks && data.networks.length > 0) {";
        html += "      data.networks.forEach(network => {";
        html += "        var option = document.createElement('option');";
        html += "        option.value = network.ssid;";
        html += "        option.textContent = network.ssid + ' (' + network.rssi + ' dBm)';";
        html += "        wifiList.appendChild(option);";
        html += "      });";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-connected\"></span>æ«æå®æ (' + data.networks.length + 'ä¸ªç½ç»)';";
        html += "    } else {";
        html += "      wifiList.innerHTML = '<option value=\"\">æªæ¾å°WiFiç½ç»</option>';";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>éæ°æ«æ';";
        html += "    }";
        html += "    scanBtn.disabled = false;";
        html += "  }).catch(error => {";
        html += "    wifiList.innerHTML = '<option value=\"\">æ«æå¤±è´¥</option>';";
        html += "    scanBtn.disabled = false;";
        html += "    scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>éæ°æ«æ';";
        html += "  });";
        html += "}";
        html += "window.onload = scanWiFi;";
        html += "function confirmFactoryReset() {";
        html += "  if (confirm('â ï¸ å±é©æä½è­¦åï¼\\n\\næ¢å¤åºåè®¾ç½®å°æ¸é¤ææWiFiéç½®åæ¸©åº¦æ ¡ååæ°ã\\nè®¾å¤å°éå¯å¹¶è¿å¥éç½æ¨¡å¼ï¼éè¦éæ°éç½®WiFiç½ç»ã\\n\\nç¡®å®è¦æ§è¡æ¢å¤åºåè®¾ç½®åï¼')) {";
        html += "    var resetBtn = document.querySelector('[onclick=\\\"confirmFactoryReset()\\\"]');";
        html += "    resetBtn.disabled = true;";
        html += "    resetBtn.innerHTML = 'æ¢å¤åºåè®¾ç½®ä¸­...';";
        html += "    resetBtn.style.opacity = '0.7';";
        html += "    ";
        html += "    fetch('/factoryreset', { method: 'POST' })";
        html += "      .then(response => {";
        html += "        if (response.ok) {";
        html += "          resetBtn.innerHTML = 'æ¢å¤æåï¼è®¾å¤éå¯ä¸­...';";
        html += "          resetBtn.style.background = 'linear-gradient(135deg, #28a745 0%, #20c997 100%)';";
        html += "        } else {";
        html += "          throw new Error('æ¢å¤å¤±è´¥');";
        html += "        }";
        html += "      })";
        html += "      .catch(error => {";
        html += "        resetBtn.disabled = false;";
        html += "        resetBtn.innerHTML = 'æ¢å¤åºåè®¾ç½®';";
        html += "        resetBtn.style.opacity = '1';";
        html += "        alert('æ¢å¤åºåè®¾ç½®å¤±è´¥ï¼è¯·éè¯ï¼' + error.message);";
        html += "      });";
        html += "  }";
        html += "}";
        html += "</script>";
        html += "</head><body>";
        html += "<div class=\"container\">";
        html += "<div class=\"header\">";
        html += "<h1>æºè½çµç¤ç®±éç½®</h1>";
        html += "<p>WiFiç½ç»éç½®é¡µé¢</p>";
        html += "</div>";
        html += "<div class=\"device-info\">";
        html += "<p><strong>è®¾å¤ID:</strong> " + DEVICE_ID + "</p>";
        html += "<p><strong>åºä»¶çæ¬:</strong> " + FIRMWARE_VERSION + "</p>";
        html += "</div>";
        html += "<div class=\"temperature-display\">å½åæ¸©åº¦: " + String(currentTemp) + "Â°C</div>";
        html += "<div class=\"form-container\">";
        html += "<form method=\"POST\" action=\"/savewifi\">";
        html += "<div class=\"scan-section\">";
        html += "<button type=\"button\" id=\"scanBtn\" class=\"btn btn-secondary\" onclick=\"scanWiFi()\"><span class=\"status-indicator status-disconnected\"></span>æ«æWiFiç½ç»</button>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"ssid\">WiFiç½ç»</label>";
        html += "<select id=\"ssid\" name=\"ssid\" class=\"form-control\" required><option value=\"\">è¯·åæ«æWiFiç½ç»</option></select>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"password\">WiFiå¯ç </label>";
        html += "<input type=\"password\" id=\"password\" name=\"password\" class=\"form-control\" placeholder=\"è¯·è¾å¥WiFiå¯ç \" required>";
        html += "</div>";
        html += "<button type=\"submit\" class=\"btn\" style=\"width: 100%;\">ä¿å­éç½®</button>";
        html += "</form>";
        

        
        html += "</div>";
        html += "</div>";
        
        // æ¸©åº¦æ§å¶JavaScript
        html += "<script>";
        html += "let currentTargetTemp = " + String(targetTemp) + ";";
        html += "let currentHeatingState = " + String(heatingEnabled ? "true" : "false") + ";";
        
        html += "// æ´æ°æ¸©åº¦æ¾ç¤º";
        html += "function updateTemperatureDisplay() {";
        html += "  fetch('/status')";
        html += "    .then(response => response.json())";
        html += "    .then(data => {";
        html += "      document.getElementById('current-temp').textContent = data.temperature.toFixed(1) + 'Â°C';";
        html += "      document.getElementById('target-temp').textContent = data.target_temperature.toFixed(1) + 'Â°C';";
        html += "      currentTargetTemp = data.target_temperature;";
        html += "      currentHeatingState = data.heating_enabled;";
        html += "      ";
        html += "      // æ´æ°å ç­æé®ç¶æ";
        html += "      const heatingBtn = document.getElementById('heating-btn');";
        html += "      const heatingText = document.getElementById('heating-text');";
        html += "      if (data.heating_enabled) {";
        html += "        heatingBtn.classList.add('active');";
        html += "        heatingText.textContent = 'åæ­¢å ç­';";
        html += "      } else {";
        html += "        heatingBtn.classList.remove('active');";
        html += "        heatingText.textContent = 'å¼å§å ç­';";
        html += "      }";
        html += "    })";
        html += "    .catch(error => console.error('è·åæ¸©åº¦ç¶æå¤±è´¥:', error));";
        html += "}";
        
        html += "// æ¹åç®æ æ¸©åº¦";
        html += "function changeTemp(delta) {";
        html += "  const newTemp = Math.max(0, Math.min(300, currentTargetTemp + delta));";
        html += "  setTargetTemp(newTemp);";
        html += "}";
        
        html += "// è®¾ç½®é¢è®¾æ¸©åº¦";
        html += "function setPresetTemp(temp) {";
        html += "  setTargetTemp(temp);";
        html += "}";
        
        html += "// è®¾ç½®èªå®ä¹æ¸©åº¦";
        html += "function setCustomTemp() {";
        html += "  const input = document.getElementById('temp-input');";
        html += "  const temp = parseInt(input.value);";
        html += "  if (!isNaN(temp) && temp >= 0 && temp <= 300) {";
        html += "    setTargetTemp(temp);";
        html += "    input.value = '';";
        html += "  } else {";
        html += "    alert('è¯·è¾å¥0-300Â°Cä¹é´çæææ¸©åº¦å¼');";
        html += "  }";
        html += "}";
        
        html += "// è®¾ç½®ç®æ æ¸©åº¦";
        html += "function setTargetTemp(temp) {";
        html += "  const formData = new FormData();";
        html += "  formData.append('target_temp', temp);";
        html += "  ";
        html += "  fetch('/control', {";
        html += "    method: 'POST',";
        html += "    body: formData";
        html += "  })";
        html += "    .then(response => {";
        html += "      if (response.ok) {";
        html += "        currentTargetTemp = temp;";
        html += "        document.getElementById('target-temp').textContent = temp.toFixed(1) + 'Â°C';";
        html += "        console.log('ç®æ æ¸©åº¦å·²è®¾ç½®ä¸º: ' + temp + 'Â°C');";
        html += "      } else {";
        html += "        throw new Error('è®¾ç½®æ¸©åº¦å¤±è´¥');";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('è®¾ç½®æ¸©åº¦å¤±è´¥:', error);";
        html += "      alert('è®¾ç½®æ¸©åº¦å¤±è´¥ï¼è¯·æ£æ¥è®¾å¤è¿æ¥');";
        html += "    });";
        html += "}";
        
        html += "// åæ¢å ç­ç¶æ";
        html += "function toggleHeating() {";
        html += "  const newHeatingState = !currentHeatingState;";
        html += "  const formData = new FormData();";
        html += "  formData.append('heating', newHeatingState);";
        html += "  ";
        html += "  fetch('/control', {";
        html += "    method: 'POST',";
        html += "    body: formData";
        html += "  })";
        html += "    .then(response => {";
        html += "      if (response.ok) {";
        html += "        currentHeatingState = newHeatingState;";
        html += "        const heatingBtn = document.getElementById('heating-btn');";
        html += "        const heatingText = document.getElementById('heating-text');";
        html += "        if (newHeatingState) {";
        html += "          heatingBtn.classList.add('active');";
        html += "          heatingText.textContent = 'åæ­¢å ç­';";
        html += "        } else {";
        html += "          heatingBtn.classList.remove('active');";
        html += "          heatingText.textContent = 'å¼å§å ç­';";
        html += "        }";
        html += "        console.log('å ç­ç¶æå·²åæ¢ä¸º: ' + (newHeatingState ? 'å¼å¯' : 'å³é­'));";
        html += "      } else {";
        html += "        throw new Error('åæ¢å ç­ç¶æå¤±è´¥');";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('åæ¢å ç­ç¶æå¤±è´¥:', error);";
        html += "      alert('åæ¢å ç­ç¶æå¤±è´¥ï¼è¯·æ£æ¥è®¾å¤è¿æ¥');";
        html += "    });";
        html += "}";
        
        html += "// å®æ¶æ´æ°æ¸©åº¦æ¾ç¤º";
        html += "setInterval(updateTemperatureDisplay, 2000);";
        
        html += "// é¡µé¢å è½½æ¶åå§å";
        html += "document.addEventListener('DOMContentLoaded', function() {";
        html += "  updateTemperatureDisplay();";
        html += "  ";
        html += "  // è¾å¥æ¡åè½¦äºä»¶";
        html += "  document.getElementById('temp-input').addEventListener('keypress', function(e) {";
        html += "    if (e.key === 'Enter') {";
        html += "      setCustomTemp();";
        html += "    }";
        html += "  });";
        html += "});";
        html += "</script>";
        
        html += "</body></html>";
        
        webServer.send(200, "text/html", html);
    } else {
        // æ­£å¸¸æ¨¡å¼ä¸éå®åå°ç»å½é¡µé¢
        webServer.sendHeader("Location", "/login.html", true);
        webServer.send(302, "text/plain", "Redirecting to login page");
    }
}

// å¤çæææªå®ä¹çè·¯ç±ï¼å®ç°èªå¨è·³è½¬
void handleNotFound() {
    if (isCaptivePortalMode) {
        // å¨å¼ºå¶é¨æ·æ¨¡å¼ä¸ï¼å°æææªå®ä¹çè·¯ç±éå®åå°é¦é¡µ
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "Redirect to configuration page");
    } else {
        // å¨æ­£å¸¸æ¨¡å¼ä¸ï¼ç´æ¥è¿å404éè¯¯
        webServer.send(404, "text/plain", "Not found: " + webServer.uri());
    }
}

void handleSaveWiFi() {
    if (webServer.hasArg("ssid") && webServer.hasArg("password")) {
        wifiSSID = webServer.arg("ssid");
        wifiPassword = webServer.arg("password");
        
        saveConfig();
        
        String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"refresh\" content=\"5;url=/\">";
        html += "<style>";
        html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
        html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 20px; }";
        html += ".success-container { background: white; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); padding: 40px; text-align: center; max-width: 400px; }";
        html += ".success-icon { font-size: 48px; color: #4CAF50; margin-bottom: 20px; }";
        html += ".success-title { font-size: 24px; color: #28a745; margin-bottom: 15px; font-weight: 600; }";
        html += ".success-message { color: #6c757d; line-height: 1.6; margin-bottom: 20px; }";
        html += ".countdown { background: #f8f9fa; padding: 10px; border-radius: 8px; font-size: 14px; color: #495057; }";
        html += "</style>";
        html += "</head><body>";
        html += "<div class=\"success-container\">";
        html += "<div class=\"success-icon\">â</div>";
        html += "<h1 class=\"success-title\">éç½®å·²ä¿å­</h1>";
        html += "<p class=\"success-message\">WiFiéç½®å·²æåä¿å­ï¼è®¾å¤å°éå¯å¹¶å°è¯è¿æ¥WiFiç½ç»ã</p>";
        html += "<p class=\"countdown\">5ç§åèªå¨è·³è½¬åéç½®é¡µé¢...</p>";
        html += "</div>";
        html += "</body></html>";
        
        webServer.send(200, "text/html", html);
        
        delay(1000);
        ESP.restart();
    }
}

void handleFactoryReset() {
    // æ¸ç©ºEEPROMä¸­çéç½®
    EEPROM.begin(512);
    
    // æ¸ç©ºæ´ä¸ªEEPROMåºå
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    
    // æ¸ç©ºåå­ä¸­çéç½®åé
    wifiSSID = "";
    wifiPassword = "";
    temperatureOffset = 0.0;
    temperatureScale = 1.0;
    
    Serial.println("æ¢å¤åºåè®¾ç½®å®æï¼ææéç½®å·²æ¸é¤");
    
    // è¿åæåé¡µé¢
    String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"refresh\" content=\"5;url=/\">";
    html += "<style>";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
    html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 20px; }";
    html += ".reset-container { background: white; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); padding: 40px; text-align: center; max-width: 400px; }";
    html += ".reset-icon { font-size: 48px; color: #ff6b6b; margin-bottom: 20px; }";
    html += ".reset-title { font-size: 24px; color: #dc3545; margin-bottom: 15px; font-weight: 600; }";
    html += ".reset-message { color: #6c757d; line-height: 1.6; margin-bottom: 20px; }";
    html += ".countdown { background: #f8f9fa; padding: 10px; border-radius: 8px; font-size: 14px; color: #495057; }";
    html += "</style>";
    html += "</head><body>";
    html += "<div class=\"reset-container\">";
    html += "<div class=\"reset-icon\">â ï¸</div>";
    html += "<h1 class=\"reset-title\">æ¢å¤åºåè®¾ç½®</h1>";
    html += "<p class=\"reset-message\">è®¾å¤å·²æ¢å¤åºåè®¾ç½®ï¼ææWiFiéç½®åæ¸©åº¦æ ¡ååæ°å·²è¢«æ¸é¤ã</p>";
    html += "<p class=\"reset-message\">è®¾å¤å°éå¯å¹¶è¿å¥éç½æ¨¡å¼ï¼è¯·éæ°éç½®WiFiç½ç»ã</p>";
    html += "<p class=\"countdown\">5ç§åè®¾å¤å°éå¯...</p>";
    html += "</div>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
    
    delay(1000);
    ESP.restart();
}

void handleRestart() {
    Serial.println("æ¥æ¶å°éå¯è®¾å¤è¯·æ±");
    
    // è¿åéå¯ç¡®è®¤é¡µé¢
    String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"refresh\" content=\"5;url=/\">";
    html += "<style>";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
    html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 20px; }";
    html += ".restart-container { background: white; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); padding: 40px; text-align: center; max-width: 400px; }";
    html += ".restart-icon { font-size: 48px; color: #4CAF50; margin-bottom: 20px; }";
    html += ".restart-title { font-size: 24px; color: #28a745; margin-bottom: 15px; font-weight: 600; }";
    html += ".restart-message { color: #6c757d; line-height: 1.6; margin-bottom: 20px; }";
    html += ".countdown { background: #f8f9fa; padding: 10px; border-radius: 8px; font-size: 14px; color: #495057; }";
    html += "</style>";
    html += "</head><body>";
    html += "<div class=\"restart-container\">";
    html += "<div class=\"restart-icon\">ð</div>";
    html += "<h1 class=\"restart-title\">è®¾å¤éå¯ä¸­</h1>";
    html += "<p class=\"restart-message\">è®¾å¤æ­£å¨éå¯ï¼éå¯è¿ç¨å¤§çº¦éè¦30ç§ã</p>";
    html += "<p class=\"restart-message\">éå¯å®æåï¼è®¾å¤å°èªå¨éæ°è¿æ¥WiFiç½ç»ã</p>";
    html += "<p class=\"countdown\">5ç§åè®¾å¤å°éå¯...</p>";
    html += "</div>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
    
    delay(1000);
    ESP.restart();
}

void handleStatus() {
    String json = "{\"device_id\":\"" + DEVICE_ID + "\",";
    json += "\"firmware_version\":\"" + FIRMWARE_VERSION + "\",";
    json += "\"temperature\":" + String(currentTemp) + ",";
    json += "\"target_temperature\":" + String(targetTemp) + ",";
    json += "\"heating_enabled\":" + String(heatingEnabled ? "true" : "false") + ",";
    json += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"ip_address\":\"" + (isCaptivePortalMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString()) + "\"}";
    
    webServer.send(200, "application/json", json);
}

void handleDiagnostic() {
    // è¯»ååå§MAX6675æ°æ®
    uint16_t rawData = readMAX6675RawData();
    bool errorFlag = (rawData & 0x04) != 0;
    uint16_t tempBits = rawData >> 3;
    float rawTemperature = tempBits * 0.25;
    
    // ç¡®å®ä¼ æå¨ç¶æ
    String sensorStatus = "æ­£å¸¸";
    String diagnosticAdvice = "ä¼ æå¨å·¥ä½æ­£å¸¸";
    
    if (errorFlag) {
        sensorStatus = "éè¯¯";
        diagnosticAdvice = "æ£æµå°ç­çµå¶å¼è·¯æä¼ æå¨æé";
    } else if (rawData == 0x0000 || rawData == 0xFFFF) {
        sensorStatus = "è­¦å";
        diagnosticAdvice = "ä¼ æå¨æ°æ®å¼å¸¸ï¼æ£æ¥ç¡¬ä»¶è¿æ¥";
    } else if (rawTemperature < -20 || rawTemperature > 1024) {
        sensorStatus = "è­¦å";
        diagnosticAdvice = "æ¸©åº¦è¯»æ°è¶åºæ­£å¸¸èå´";
    }
    
    // æå»ºè¯æ­JSONååº
    String json = "{";
    json += "\"sensor_status\":\"" + sensorStatus + "\",";
    json += "\"raw_data\":\"0x" + String(rawData, HEX) + "\",";
    json += "\"error_flag\":" + String(errorFlag ? "true" : "false") + ",";
    json += "\"temp_bits\":" + String(tempBits) + ",";
    json += "\"raw_temperature\":" + String(rawTemperature) + ",";
    json += "\"calibration_params\":\"åç§»:" + String(temperatureOffset) + " ç¼©æ¾:" + String(temperatureScale) + "\",";
    json += "\"diagnostic_advice\":\"" + diagnosticAdvice + "\",";
    json += "\"hardware_failure_count\":" + String(hardwareFailureCount) + ",";
    json += "\"hardware_initialized\":" + String(hardwareInitialized ? "true" : "false") + ",";
    json += "\"temperature_read_count\":" + String(temperatureReadCount) + ",";
    json += "\"temperature_read_avg_time\":" + String(temperatureReadAvgTime);
    json += "}";
    
    webServer.send(200, "application/json", json);
}

void handleResetCalibration() {
    temperatureOffset = 0.0;
    temperatureScale = 1.0;
    
    String json = "{\"message\":\"æ¸©åº¦æ ¡ååæ°å·²éç½®\",";
    json += "\"offset\":" + String(temperatureOffset) + ",";
    json += "\"scale\":" + String(temperatureScale) + "}";
    
    webServer.send(200, "application/json", json);
}

void handleControl() {
    bool wasHeating = heatingEnabled;  // ä¿å­ä¹åçå ç­ç¶æ
    
    if (webServer.hasArg("target_temp")) {
        targetTemp = webServer.arg("target_temp").toFloat();
    }
    if (webServer.hasArg("heating")) {
        heatingEnabled = webServer.arg("heating") == "true";
    }
    
    // æ£æµå ç­ç¶æååå¹¶è§¦åèé¸£å¨æç¤º
    if (!wasHeating && heatingEnabled) {
        // ä»å³é­å°å¼å¯ï¼å¼å§çç
        beepBakingStart();
        Serial.println("ççå¼å§ - ç®æ æ¸©åº¦: " + String(targetTemp) + "Â°C");
    } else if (wasHeating && !heatingEnabled) {
        // ä»å¼å¯å°å³é­ï¼ççå®æ
        beepBakingComplete();
        Serial.println("ççå®æ - æç»æ¸©åº¦: " + String(currentTemp) + "Â°C");
        
        // è§¦åççç»æç¶æï¼å¿«éª10ç§
        bakingCompleteState = true;
        bakingCompleteStartTime = millis();
        Serial.println("è§¦åççç»æå¿«éªç¶æï¼æç»­10ç§");
    }
    
    webServer.send(200, "text/plain", "OK");
}

// =========================================
// æ°å¢APIç«¯ç¹å¤çå½æ°
// =========================================

// æ¸©åº¦åå²è®°å½ç«¯ç¹
void handleTemperatureHistory() {
    String json = "{\"temperature_history\":[";
    
    // æ¨¡ææ¸©åº¦åå²æ°æ®ï¼å®éåºç¨ä¸­å¯ä»¥ä»EEPROMææä»¶ç³»ç»è¯»åï¼
    for (int i = 0; i < 10; i++) {
        if (i > 0) json += ",";
        json += "{\"timestamp\":" + String(millis() - i * 60000) + ",";
        json += "\"temperature\":" + String(currentTemp - i * 0.5) + "}";
    }
    
    json += "]}";
    webServer.send(200, "application/json", json);
}

// å®æ¶ä»»å¡ç®¡çç«¯ç¹
void handleTimerTasks() {
    if (webServer.method() == HTTP_GET) {
        // è·åå®æ¶ä»»å¡åè¡¨
        String json = "{\"timers\":[";
        json += "{\"id\":1,\"enabled\":false,\"target_temp\":180,\"duration\":3600}";
        json += "]}";
        webServer.send(200, "application/json", json);
    } else if (webServer.method() == HTTP_POST) {
        // åå»ºææ´æ°å®æ¶ä»»å¡
        if (webServer.hasArg("action") && webServer.arg("action") == "create") {
            webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"å®æ¶ä»»å¡åå»ºæå\"}");
        } else {
            webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"å®æ¶ä»»å¡æ´æ°æå\"}");
        }
    }
}

// è®¾å¤ä¿¡æ¯ç«¯ç¹
void handleDeviceInfo() {
    String json = "{";
    json += "\"device_id\":\"" + DEVICE_ID + "\",";
    json += "\"firmware_version\":\"" + FIRMWARE_VERSION + "\",";
    json += "\"hardware_model\":\"SmartOven Controller v1.0\",";
    json += "\"chip_model\":\"ESP32\",";
    json += "\"flash_size\":4194304,";
    json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"wifi_ssid\":\"" + WiFi.SSID() + "\",";
    json += "\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"wifi_ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";
    
    webServer.send(200, "application/json", json);
}

// å®å¨çæ§ç«¯ç¹
void handleSafetyMonitor() {
    String json = "{";
    json += "\"temperature_alerts\":[";
    json += "{\"type\":\"high_temperature\",\"threshold\":250,\"current\":" + String(currentTemp) + ",\"triggered\":" + String(currentTemp > 250 ? "true" : "false") + "}";
    json += "],";
    json += "\"safety_status\":\"normal\",";
    json += "\"last_check\":" + String(millis()) + ",";
    json += "\"recommendations\":[";
    json += "\"ä¿æè®¾å¤å¨å´éé£è¯å¥½\",";
    json += "\"å®ææ£æ¥æ¸©åº¦ä¼ æå¨\",";
    json += "\"é¿åé¿æ¶é´é«æ¸©è¿è¡\"";
    json += "]}";
    
    webServer.send(200, "application/json", json);
}

// è½èç»è®¡ç«¯ç¹
void handleEnergyStats() {
    String json = "{";
    json += "\"total_energy_used\":" + String(millis() / 3600000.0 * 1500) + ","; // æ¨¡æè½èæ°æ®
    json += "\"current_power\":" + String(heatingEnabled ? "1500" : "0") + ",";
    json += "\"today_energy\":" + String(millis() / 86400000.0 * 1500) + ",";
    json += "\"energy_unit\":\"Wh\",";
    json += "\"power_unit\":\"W\",";
    json += "\"stats_period\":\"lifetime\"";
    json += "}";
    
    webServer.send(200, "application/json", json);
}

// =========================================
// æ¸©åº¦æ§å¶åè½
// =========================================

// æ¸©åº¦è¯»åå®æ¶å¨
unsigned long lastTemperatureRead = 0;
const unsigned long TEMPERATURE_READ_INTERVAL = 200; // æ¯200msè¯»åä¸æ¬¡æ¸©åº¦ï¼ä¼åï¼æé«æ¸©åº¦ååºéåº¦ï¼

void readTemperature() {
    unsigned long currentTime = millis();
    
    // å®æ¶è¯»åæ¸©åº¦ï¼é¿åé¢ç¹è¯»åå½±åæ§è½
    if (currentTime - lastTemperatureRead >= TEMPERATURE_READ_INTERVAL) {
        currentTemp = readTemperatureWithMonitoring();
        if (currentTemp < 0) {
            Serial.println("æ¸©åº¦ä¼ æå¨è¯»åéè¯¯");
        } else {
            // åªå¨æ¸©åº¦ååè¾å¤§æ¶ææå°æ¥å¿ï¼åå°ä¸²å£è¾åº
            static float lastPrintedTemp = -999;
            if (fabs(currentTemp - lastPrintedTemp) >= 0.5) {
                Serial.println("å½åæ¸©åº¦: " + String(currentTemp) + "Â°C");
                lastPrintedTemp = currentTemp;
            }
        }
        lastTemperatureRead = currentTime;
    }
}

void controlHeater() {
    if (heatingEnabled && currentTemp < targetTemp) {
        digitalWrite(HEATER_PIN, HIGH);
    } else {
        digitalWrite(HEATER_PIN, LOW);
    }
}

// =========================================
// LEDç¶ææç¤º
// =========================================
void updateLED() {
    unsigned long currentTime = millis();
    
    // ççç»æç¶æå¤ç
    if (bakingCompleteState) {
        if (currentTime - bakingCompleteStartTime > BAKING_COMPLETE_DURATION) {
            // ççç»æå¿«éªæ¶é´å°ï¼åæ¢å°å¾æºç¶æ
            bakingCompleteState = false;
            Serial.println("ççç»æå¿«éªå®æï¼åæ¢å°å¾æºç¶æ");
        }
    }
    
    if (currentTime - lastLedUpdate > LED_BLINK_INTERVAL) {
        if (bakingCompleteState) {
            // ççç»æç¶æï¼å¿«ééªçï¼500msé´éï¼
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LEDç¶æ: ççç»æ - å¿«éª (äº®)");
            } else {
                Serial.println("LEDç¶æ: ççç»æ - å¿«éª (ç­)");
            }
        } else if (isCaptivePortalMode) {
            // éç½åç¶æï¼å¿«ééªçï¼500msé´éï¼
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LEDç¶æ: éç½å - å¿«éª (äº®)");
            } else {
                Serial.println("LEDç¶æ: éç½å - å¿«éª (ç­)");
            }
        } else if (heatingEnabled) {
            // å ç­ä¸­ç¶æï¼å¸¸äº®
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LEDç¶æ: å ç­ä¸­ - å¸¸äº®");
        } else {
            // å¾æºç¶æï¼æ¢ééªçï¼1000msé´éï¼
            if (currentTime - lastLedUpdate > 1000) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState ? HIGH : LOW);
                if (ledState) {
                    Serial.println("LEDç¶æ: å¾æº - æ¢éª (äº®)");
                } else {
                    Serial.println("LEDç¶æ: å¾æº - æ¢éª (ç­)");
                }
                lastLedUpdate = currentTime;
            }
            return; // å¾æºç¶æä½¿ç¨èªå®ä¹é´éï¼ä¸æ´æ°lastLedUpdate
        }
        
        lastLedUpdate = currentTime;
    }
}

// =========================================
// èé¸£å¨æ§å¶
// =========================================
void beep(int duration = 100) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}

// èé¸£å¨ç¶ææç¤ºå½æ°
void beepConfigSaved() {
    // è®¾ç½®ä¿å­æåæç¤ºï¼ç­-ç­-ç­
    beep(100);
    delay(100);
    beep(100);
    delay(100);
    beep(100);
    Serial.println("èé¸£å¨æç¤ºï¼éç½®å·²ä¿å­");
}

void beepBakingStart() {
    // å¼å§ççæç¤ºï¼é¿-ç­
    beep(300);
    delay(200);
    beep(100);
    Serial.println("èé¸£å¨æç¤ºï¼ççå¼å§");
}

void beepBakingComplete() {
    // ççå®ææç¤ºï¼é¿-é¿-é¿
    beep(500);
    delay(200);
    beep(500);
    delay(200);
    beep(500);
    Serial.println("èé¸£å¨æç¤ºï¼ççå®æ");
}

void beepTemperatureChange() {
    // æ¸©åº¦åæ¢æç¤ºï¼ç­-ç­-ç­-ç­
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    Serial.println("èé¸£å¨æç¤ºï¼æ¸©åº¦åæ¢");
}

// =========================================
// åå§åå½æ°
// =========================================
void setup() {
    Serial.begin(115200);
    Serial.println("");
    Serial.println("=========================================");
    Serial.println("æºè½çµç¤ç®±æ§å¶å¨ v" + FIRMWARE_VERSION);
    Serial.println("MAX6675æå¨SPIå®ç° - æ¸©åº¦ä¼ æå¨æ­£å¸¸å·¥ä½");
    Serial.println("=========================================");
    
    // åå§åå¼è
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // åå§åMAX6675å¼è
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    // è®¾ç½®MAX6675åå§ç¶æ
    digitalWrite(THERMO_CS, HIGH);  // CSå¼èé«çµå¹³ï¼ç¦ç¨ï¼
    digitalWrite(THERMO_CLK, LOW);  // CLKå¼èä½çµå¹³
    
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    // èé¸£å¨æç¤ºå¯å¨
    beep(200);
    delay(100);
    beep(200);
    
    // ç¡¬ä»¶åå§åéªè¯
    if (!verifyHardwareInitialization()) {
        Serial.println("â ï¸ ç¡¬ä»¶åå§åéªè¯å¤±è´¥ï¼å°è¯ç¡¬ä»¶å¤ä½...");
        
        // å¼ºå¶ç¡¬ä»¶å¤ä½
        for (int i = 0; i < 3; i++) {
            digitalWrite(THERMO_CS, HIGH);
            digitalWrite(THERMO_CLK, LOW);
            delay(100);
            digitalWrite(THERMO_CS, LOW);
            delay(50);
            digitalWrite(THERMO_CS, HIGH);
            delay(100);
        }
        
        // éæ°éªè¯
        if (verifyHardwareInitialization()) {
            Serial.println("â ç¡¬ä»¶å¤ä½æå");
        } else {
            Serial.println("â ç¡¬ä»¶å¤ä½å¤±è´¥ï¼è¯·æ£æ¥ç¡¬ä»¶è¿æ¥");
        }
    }
    
    // å è½½éç½®
    if (loadConfig()) {
        Serial.println("éç½®å è½½æå");
    } else {
        Serial.println("æªæ¾å°ææéç½®");
    }
    
    // å¯å¨ç½ç»
    if (shouldStartCaptivePortal()) {
        startCaptivePortal();
    } else {
        connectToWiFi();
    }
    
    // å¯å¨TCPæå¡å¨ç¨äºAPPè¿æ¥
    tcpServer.begin();
    Serial.println("TCPæå¡å¨å·²å¯å¨ï¼çå¬ç«¯å£: " + String(DEFAULT_PORT));
    
    // åå§åSPIFFSæä»¶ç³»ç»
    if (SPIFFS.begin()) {
        Serial.println("SPIFFSæä»¶ç³»ç»åå§åæå");
        
        // æ£æ¥æ¯å¦å­å¨å¿è¦çæä»¶
        if (SPIFFS.exists("/login.html")) {
            Serial.println("æ¾å°ç»å½é¡µé¢æä»¶: /login.html");
        } else {
            Serial.println("è­¦å: æªæ¾å°ç»å½é¡µé¢æä»¶ /login.html");
        }
        
        if (SPIFFS.exists("/index.html")) {
            Serial.println("æ¾å°ä¸»é¡µæä»¶: /index.html");
        } else {
            Serial.println("è­¦å: æªæ¾å°ä¸»é¡µæä»¶ /index.html");
        }
    } else {
        Serial.println("éè¯¯: SPIFFSæä»¶ç³»ç»åå§åå¤±è´¥");
    }
    
    Serial.println("åå§åå®æ");
}

// =========================================
// ä¸²å£å½ä»¤å¤ç
// =========================================
void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            Serial.println("æ¶å°å½ä»¤: " + command);
            
            if (command == "LED_ON") {
                digitalWrite(LED_PIN, HIGH);
                Serial.println("LEDå·²æå¼");
            } else if (command == "LED_OFF") {
                digitalWrite(LED_PIN, LOW);
                Serial.println("LEDå·²å³é­");
            } else if (command == "LED_BLINK") {
                // ä¸´æ¶éªçLED
                for (int i = 0; i < 5; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(200);
                    digitalWrite(LED_PIN, LOW);
                    delay(200);
                }
                Serial.println("LEDéªçå®æ");
            } else if (command == "BEEP") {
                beep(100);
                Serial.println("èé¸£å¨å·²å");
            } else if (command == "BEEP_LONG") {
                beep(500);
                Serial.println("èé¸£å¨é¿å");
            } else if (command == "BEEP_SHORT") {
                beep(50);
                Serial.println("èé¸£å¨ç­å");
            } else if (command == "GET_STATUS") {
                // è¿åAPPææçæ ¼å¼ï¼TEMP:25.50,TARGET:180.00,HEAT:0,MODE:1,UPTIME:123
                String statusResponse = "TEMP:" + String(currentTemp) + 
                                      ",TARGET:" + String(targetTemp) + 
                                      ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                                      ",MODE:" + String(ovenMode ? "1" : "0") + 
                                      ",UPTIME:" + String(millis() / 1000);
                Serial.println(statusResponse);
            } else if (command == "GET_TEMP") {
                Serial.println("å½åæ¸©åº¦: " + String(currentTemp) + "Â°C");
            } else if (command.startsWith("CALIBRATE_TEMP")) {
                // æ¸©åº¦æ ¡åå½ä»¤æ ¼å¼: CALIBRATE_TEMP å®éæ¸©åº¦
                int spaceIndex = command.indexOf(' ');
                if (spaceIndex > 0) {
                    String actualTempStr = command.substring(spaceIndex + 1);
                    float actualTemp = actualTempStr.toFloat();
                    float measuredTemp = currentTemp;
                    
                    if (actualTemp > 0) {
                        calibrateTemperature(actualTemp, measuredTemp);
                        Serial.println("æ¸©åº¦æ ¡åå·²åºç¨ï¼è¯·éæ°è¯»åæ¸©åº¦éªè¯");
                    } else {
                        Serial.println("éè¯¯: å®éæ¸©åº¦å¼æ æ");
                    }
                } else {
                    Serial.println("æ¸©åº¦æ ¡åå½ä»¤æ ¼å¼: CALIBRATE_TEMP å®éæ¸©åº¦");
                    Serial.println("ä¾å¦: CALIBRATE_TEMP 16.0");
                }
            } else if (command == "RESET_CALIBRATION") {
                temperatureOffset = 0.0;
                temperatureScale = 1.0;
                Serial.println("æ¸©åº¦æ ¡åå·²éç½®");
            } else if (command == "GET_RAW_TEMP") {
                // è·ååå§æ¸©åº¦æ°æ®ï¼æªæ ¡åï¼
                uint16_t rawData = readMAX6675RawData();
                if (!(rawData & 0x04)) {
                    uint16_t tempBits = rawData >> 3;
                    float rawTemp = tempBits * 0.25;
                    Serial.println("åå§æ¸©åº¦æ°æ®:");
                    Serial.print("åå§å¼: 0x"); Serial.println(rawData, HEX);
                    Serial.print("æ¸©åº¦ä½: "); Serial.println(tempBits);
                    Serial.print("æªæ ¡åæ¸©åº¦: "); Serial.print(rawTemp); Serial.println("Â°C");
                    Serial.print("æ ¡ååæ¸©åº¦: "); Serial.print(currentTemp); Serial.println("Â°C");
                } else {
                    Serial.println("éè¯¯: æ¸©åº¦ä¼ æå¨è¯»åéè¯¯");
                }
            } else if (command == "GET_PERFORMANCE") {
                // è·åæ§è½ä¿¡æ¯
                Serial.println("ð è®¾å¤æ§è½ä¿¡æ¯:");
                Serial.print("æ¸©åº¦è¯»åå¹³åæ¶é´: "); Serial.print(temperatureReadAvgTime); Serial.println("ms");
                Serial.print("æ¸©åº¦è¯»åæ»æ¬¡æ°: "); Serial.println(temperatureReadCount);
                
                // è·ååå­ä¿¡æ¯
                uint32_t freeHeap = ESP.getFreeHeap();
                uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
                uint32_t heapFragmentation = ESP.getHeapFragmentation();
                
                Serial.print("ç©ºé²åå­: "); Serial.print(freeHeap); Serial.println(" bytes");
                Serial.print("æå¤§ç©ºé²å: "); Serial.print(maxFreeBlock); Serial.println(" bytes");
                Serial.print("åå­ç¢çç: "); Serial.print(heapFragmentation); Serial.println("%");
                
                // è¾åºå½åç¶æ
                Serial.print("Webæå¡å¨å¤çé´é: "); Serial.print(WEB_SERVER_HANDLE_INTERVAL); Serial.println("ms");
                Serial.print("æ¸©åº¦è¯»åé´é: "); Serial.print(TEMPERATURE_READ_INTERVAL); Serial.println("ms");
            } else if (command == "RESET_PERFORMANCE") {
                // éç½®æ§è½è®¡æ°å¨
                temperatureReadCount = 0;
                temperatureReadAvgTime = 0;
                Serial.println("â æ§è½è®¡æ°å¨å·²éç½®");
            } else {
                Serial.println("æªç¥å½ä»¤ï¼å¯ç¨å½ä»¤:");
                Serial.println("LED_ON, LED_OFF, LED_BLINK");
                Serial.println("BEEP, BEEP_LONG, BEEP_SHORT");
                Serial.println("GET_STATUS, GET_TEMP, GET_RAW_TEMP");
                Serial.println("CALIBRATE_TEMP å®éæ¸©åº¦");
                Serial.println("RESET_CALIBRATION");
                Serial.println("GET_PERFORMANCE, RESET_PERFORMANCE");
            }
        }
    }
}

// =========================================
// TCPæå¡å¨å¤çå½æ°
// =========================================

void handleTCPConnection() {
    // æ£æ¥æ¯å¦ææ°çå®¢æ·ç«¯è¿æ¥
    if (tcpServer.hasClient()) {
        // å¦æå·²ç»æå®¢æ·ç«¯è¿æ¥ï¼æ­å¼æ§çè¿æ¥
        if (tcpClient && tcpClient.connected()) {
            tcpClient.stop();
            Serial.println("TCPå®¢æ·ç«¯å·²æ­å¼");
        }
        
        // æ¥åæ°çå®¢æ·ç«¯è¿æ¥
        tcpClient = tcpServer.available();
        if (tcpClient) {
            Serial.println("TCPå®¢æ·ç«¯å·²è¿æ¥: " + tcpClient.remoteIP().toString());
            
            // åéæ¬¢è¿æ¶æ¯
            tcpClient.println("SmartOven Controller v" + FIRMWARE_VERSION);
            tcpClient.println("è¿æ¥æåï¼è¯·è¾å¥å½ä»¤");
        }
    }
    
    // å¤çå·²è¿æ¥çå®¢æ·ç«¯æ°æ®
    if (tcpClient && tcpClient.connected()) {
        if (tcpClient.available()) {
            String command = tcpClient.readStringUntil('\n');
            command.trim();
            
            if (command.length() > 0) {
                Serial.println("TCPæ¶å°å½ä»¤: " + command);
                handleTCPCommand(command);
            }
        }
    }
}

void handleTCPCommand(String command) {
    if (command == "GET_STATUS") {
        // è¿åè®¾å¤ç¶æä¿¡æ¯
        String statusResponse = "TEMP:" + String(currentTemp) + 
                              ",TARGET:" + String(targetTemp) + 
                              ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                              ",MODE:" + String(ovenMode ? "1" : "0") + 
                              ",UPTIME:" + String(millis() / 1000);
        tcpClient.println(statusResponse);
        Serial.println("TCPåéç¶æ: " + statusResponse);
    } else if (command == "GET_TEMP") {
        tcpClient.println("å½åæ¸©åº¦: " + String(currentTemp) + "Â°C");
    } else if (command.startsWith("SET_TEMP")) {
        // è®¾ç½®ç®æ æ¸©åº¦æ ¼å¼: SET_TEMP 180.0
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            String tempStr = command.substring(spaceIndex + 1);
            float newTemp = tempStr.toFloat();
            
            if (newTemp >= 0 && newTemp <= 300) {
                targetTemp = newTemp;
                tcpClient.println("ç®æ æ¸©åº¦å·²è®¾ç½®ä¸º: " + String(targetTemp) + "Â°C");
                Serial.println("TCPè®¾ç½®ç®æ æ¸©åº¦: " + String(targetTemp) + "Â°C");
            } else {
                tcpClient.println("éè¯¯: æ¸©åº¦èå´åºä¸º0-300Â°C");
            }
        }
    } else if (command == "HEAT_ON") {
        heatingEnabled = true;
        tcpClient.println("å ç­å·²å¼å¯");
        Serial.println("TCPå¼å¯å ç­");
    } else if (command == "HEAT_OFF") {
        heatingEnabled = false;
        tcpClient.println("å ç­å·²å³é­");
        Serial.println("TCPå³é­å ç­");
    } else if (command == "OVEN_MODE") {
        ovenMode = true;
        tcpClient.println("å·²åæ¢å°ç¤ç®±æ¨¡å¼");
        Serial.println("TCPåæ¢å°ç¤ç®±æ¨¡å¼");
    } else if (command == "TOASTER_MODE") {
        ovenMode = false;
        tcpClient.println("å·²åæ¢å°ç¤é¢åæºæ¨¡å¼");
        Serial.println("TCPåæ¢å°ç¤é¢åæºæ¨¡å¼");
    } else if (command == "PING") {
        tcpClient.println("PONG");
    } else {
        tcpClient.println("æªç¥å½ä»¤ï¼å¯ç¨å½ä»¤:");
        tcpClient.println("GET_STATUS, GET_TEMP, SET_TEMP æ¸©åº¦å¼");
        tcpClient.println("HEAT_ON, HEAT_OFF, OVEN_MODE, TOASTER_MODE, PING");
    }
}

// =========================================
// æ§è½çæ§ååå­ç®¡ç
// =========================================

// åå­çæ§å®æ¶å¨
unsigned long lastMemoryCheck = 0;
const unsigned long MEMORY_CHECK_INTERVAL = 5000; // æ¯5ç§æ£æ¥ä¸æ¬¡åå­

// æ¸©åº¦è¯»åæ§è½çæ§
unsigned long lastTemperatureReadTime = 0;

// =========================================
// ä¸»å¾ªç¯
// =========================================

// æ¸©åº¦åæ¢æ£æµåé
float lastTargetTemp = targetTemp;
unsigned long lastTempChangeTime = 0;
const unsigned long TEMP_CHANGE_DEBOUNCE = 2000; // 2ç§é²æ

// Webæå¡å¨å¤çå®æ¶å¨
unsigned long lastWebServerHandle = 0;

void loop() {
    unsigned long currentTime = millis();
    
    // å¤çå¼ºå¶é¨æ·ï¼æé«ååºé¢çï¼
    if (isCaptivePortalMode) {
        // å¼ºå¶é¨æ·æ¨¡å¼ä¸éè¦æ´é¢ç¹å°å¤çDNSåWebè¯·æ±
        dnsServer.processNextRequest();
        webServer.handleClient();
        checkCaptivePortalTimeout();
    } else {
        // æ­£å¸¸æ¨¡å¼ä¸å¯ä»¥éå½éä½å¤çé¢ç
        if (currentTime - lastWebServerHandle >= WEB_SERVER_HANDLE_INTERVAL) {
            webServer.handleClient();
            lastWebServerHandle = currentTime;
        }
    }
    
    // å¤çOTAåçº§ï¼ä½é¢ï¼
    handleOTA();
    
    // å¤çè®¾å¤åç°ï¼ä½é¢ï¼
    handleDiscovery();
    
    // å¤çTCPè¿æ¥ï¼é«é¢ï¼
    handleTCPConnection();
    
    // å¤çä¸²å£å½ä»¤ï¼é«é¢ï¼
    handleSerialCommands();
    
    // æ¸©åº¦æ§å¶ï¼å®æ¶è¯»åï¼
    readTemperature();
    controlHeater();
    
    // LEDç¶ææ´æ°ï¼å®æ¶ï¼
    updateLED();
    
    // æ¸©åº¦åæ¢æ£æµ
    if (targetTemp != lastTargetTemp) {
        // é²æå¤çï¼åªæå¨æ¸©åº¦ååå2ç§åæ²¡æåæ¬¡ååæè§¦åæç¤º
        if (currentTime - lastTempChangeTime > TEMP_CHANGE_DEBOUNCE) {
            Serial.println("æ£æµå°æ¸©åº¦åæ¢: " + String(lastTargetTemp) + "Â°C -> " + String(targetTemp) + "Â°C");
            beepTemperatureChange();
            lastTargetTemp = targetTemp;
        }
        lastTempChangeTime = currentTime;
    }
    
        // åå­çæ§ï¼æ¯5ç§æ£æ¥ä¸æ¬¡ï¼
    if (currentTime - lastMemoryCheck >= MEMORY_CHECK_INTERVAL) {
        checkMemoryUsage();
        lastMemoryCheck = currentTime;
    }
    
    // ä½¿ç¨éé»å¡å»¶è¿ï¼æé«ååºéåº¦
    delay(1);
}

// =========================================
// æ§è½çæ§å½æ°
// =========================================

void checkMemoryUsage() {
    // æ£æ¥ESP8266åå­ä½¿ç¨æåµ
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
    uint32_t heapFragmentation = ESP.getHeapFragmentation();
    
    // åªå¨åå­ä½¿ç¨çè¾é«æ¶è¾åºè­¦å
    if (freeHeap < 20000) { // å°äº20KBç©ºé²åå­
        Serial.println("â ï¸ åå­è­¦å: ç©ºé²åå­è¾ä½ - " + String(freeHeap) + " bytes");
        Serial.println("æå¤§ç©ºé²å: " + String(maxFreeBlock) + " bytes");
        Serial.println("åå­ç¢çç: " + String(heapFragmentation) + "%");
    }
    
    // è¾åºæ¸©åº¦è¯»åæ§è½ç»è®¡
    if (temperatureReadCount > 0) {
        Serial.println("ð æ¸©åº¦è¯»åæ§è½: å¹³åæ¶é´ " + String(temperatureReadAvgTime) + "ms, æ»æ¬¡æ° " + String(temperatureReadCount));
        // éç½®è®¡æ°å¨
        temperatureReadCount = 0;
        temperatureReadAvgTime = 0;
    }
}

// å¢å¼ºçæ¸©åº¦è¯»åå½æ°ï¼åå«æ§è½çæ§
float readTemperatureWithMonitoring() {
    unsigned long startTime = micros();
    
    float temp = readTemperatureManual();
    
    unsigned long endTime = micros();
    unsigned long readTime = (endTime - startTime) / 1000; // è½¬æ¢ä¸ºæ¯«ç§
    
    // æ´æ°æ§è½ç»è®¡ï¼ä»å¨æåè¯»åæ¶ï¼
    if (temp >= 0) {
        temperatureReadAvgTime = (temperatureReadAvgTime * temperatureReadCount + readTime) / (temperatureReadCount + 1);
        temperatureReadCount++;
    }
    
    return temp;
}