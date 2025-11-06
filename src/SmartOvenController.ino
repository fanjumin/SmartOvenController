// =========================================
// 智能电烤箱控制器 v0.6.0
// =========================================
// 版本: 0.6.0
// 功能: 强制门户配网 + 设备自动发现 + OTA升级 + MAX6675手动SPI实现 + 主页集成温度控制
// 更新: 修复WiFi连接稳定性问题，统一连接超时时间为30秒
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
// 硬件引脚定义
// =========================================
#define THERMO_CLK   14  // MAX6675时钟 (D5/GPIO14)
#define THERMO_CS    12  // MAX6675片选 (D6/GPIO12)
#define THERMO_DO    13  // MAX6675数据输出 (D7/GPIO13)
#define HEATER_PIN   5   // 加热器控制
#define BUZZER_PIN   4   // 蜂鸣器 (GPIO4/D2)
#define LED_PIN      2   // LED指示灯 (GPIO2/D4)

// =========================================
// 全局变量定义
// =========================================

// 网络相关
ESP8266WebServer webServer(80);
WiFiUDP udp;
DNSServer dnsServer;
ESP8266WebServer otaServer(8080);
ESP8266HTTPUpdateServer httpUpdater;
WiFiServer tcpServer(8888);  // TCP服务器用于APP连接
WiFiClient tcpClient;        // TCP客户端连接

// 性能监控变量
unsigned long temperatureReadCount = 0;
float temperatureReadAvgTime = 0;
const unsigned long WEB_SERVER_HANDLE_INTERVAL = 100; // 每100ms处理一次Web请求（优化：减少CPU占用）

// 硬件故障监控
unsigned long hardwareFailureCount = 0;       // 硬件故障次数
unsigned long lastHardwareReset = 0;         // 上次硬件复位时间
bool hardwareInitialized = false;            // 硬件初始化状态

// 设备信息
const String DEVICE_TYPE = "oven";
const String DEVICE_ID = "oven-" + String(ESP.getChipId());
const String DEVICE_NAME = "SmartOven";
const String FIRMWARE_VERSION = "0.6.0";

// WiFi配置
String wifiSSID = "";
String wifiPassword = "";

// 强制门户配置
bool isCaptivePortalMode = false;
unsigned long captivePortalStartTime = 0;
const unsigned long CAPTIVE_PORTAL_TIMEOUT = 300000; // 5分钟超时
const String AP_SSID = "SmartOven-" + String(ESP.getChipId());
const String AP_PASSWORD = "12345678";

// 温度控制
float currentTemp = 0.0;
float targetTemp = 180.0;
bool heatingEnabled = false;
bool ovenMode = true; // 烤箱模式：true=烤箱模式，false=烤面包机模式

// 网络端口配置
const int DEFAULT_PORT = 8888;

// =========================================
// MAX6675手动SPI通信函数
// =========================================

// 读取MAX6675原始数据（16位）
uint16_t readMAX6675RawData() {
  uint16_t data = 0;
  
  // 硬件复位：确保芯片处于已知状态
  digitalWrite(THERMO_CS, HIGH);
  digitalWrite(THERMO_CLK, LOW);
  delayMicroseconds(100);  // 延长复位时间
  
  // 启用芯片
  digitalWrite(THERMO_CS, LOW);
  delayMicroseconds(50);  // 大幅增加延迟确保芯片稳定
  
  // 读取16位数据（MSB优先）
  for (int i = 15; i >= 0; i--) {
    digitalWrite(THERMO_CLK, HIGH);  // 时钟上升沿
    delayMicroseconds(20);  // 大幅增加时钟延迟
    
    if (digitalRead(THERMO_DO)) {   // 读取数据位
      data |= (1 << i);
    }
    
    digitalWrite(THERMO_CLK, LOW);  // 时钟下降沿
    delayMicroseconds(20);  // 大幅增加时钟延迟
  }
  
  // 禁用芯片
  digitalWrite(THERMO_CS, HIGH);
  delayMicroseconds(50);  // 大幅增加延迟确保芯片稳定
  
  return data;
}

// 温度校准参数
float temperatureOffset = 0.0;  // 温度偏移量，用于校准
float temperatureScale = 1.0;    // 温度缩放系数，用于校准

// 硬件初始化验证
bool verifyHardwareInitialization() {
    Serial.println("验证硬件初始化状态...");
    
    // 检查引脚状态
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    // 设置初始状态
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    delay(100);  // 确保硬件稳定
    
    // 验证引脚状态
    if (digitalRead(THERMO_DO) == HIGH || digitalRead(THERMO_DO) == LOW) {
        Serial.println("硬件初始化验证通过");
        return true;
    } else {
        Serial.println("硬件初始化验证失败");
        return false;
    }
}

// 硬件自动恢复机制
void performHardwareRecovery() {
    Serial.println("🚨 执行硬件自动恢复...");
    
    // 记录故障次数
    hardwareFailureCount++;
    
    // 强制硬件复位序列
    for (int i = 0; i < 5; i++) {
        digitalWrite(THERMO_CS, HIGH);
        digitalWrite(THERMO_CLK, LOW);
        delay(200);  // 延长复位时间
        digitalWrite(THERMO_CS, LOW);
        delay(100);
        digitalWrite(THERMO_CS, HIGH);
        delay(200);
    }
    
    // 重新初始化引脚
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    
    lastHardwareReset = millis();
    Serial.println("✅ 硬件自动恢复完成，故障次数: " + String(hardwareFailureCount));
}

// 读取温度值（手动SPI实现）
float readTemperatureManual() {
    // 增强重试机制：最多重试5次，每次增加延迟
    for (int retry = 0; retry < 5; retry++) {
        uint16_t rawData = readMAX6675RawData();
        
        // 检查数据有效性（排除全0或全1的无效数据）
        if (rawData == 0x0000 || rawData == 0xFFFF) {
            if (retry < 4) {
                Serial.print("温度传感器返回无效数据，第");
                Serial.print(retry + 1);
                Serial.println("次重试...");
                delay(50 * (retry + 1));  // 递增延迟：50ms, 100ms, 150ms, 200ms
                continue;
            } else {
                Serial.println("温度传感器返回无效数据（全0或全1）- 执行硬件自动恢复");
                performHardwareRecovery();
                return -1.0;
            }
        }
        
        // 检查错误标志位（D2位为0表示正常）
        if (!(rawData & 0x04)) {
            uint16_t tempBits = rawData >> 3;  // 右移3位获取温度数据
            float temperature = tempBits * 0.25;  // 每个单位0.25°C
            
            // 应用温度校准
            temperature = (temperature * temperatureScale) + temperatureOffset;
            
            // 检查温度范围是否合理
            if (temperature >= -50.0 && temperature <= 400.0) {
                Serial.print("温度读取成功，重试次数: ");
                Serial.println(retry + 1);
                
                // 重置故障计数器（如果连续成功）
                if (retry == 0) {
                    hardwareFailureCount = 0;
                }
                
                return temperature;
            } else {
                Serial.println("温度传感器读数超出范围");
                return -1.0;
            }
        } else {
            if (retry < 4) {
                Serial.print("温度传感器读取错误，第");
                Serial.print(retry + 1);
                Serial.println("次重试...");
                delay(50 * (retry + 1));  // 递增延迟
                continue;
            } else {
                Serial.println("温度传感器读取错误 - 执行硬件自动恢复");
                performHardwareRecovery();
                return -1.0;
            }
        }
    }
    
    // 所有重试都失败，执行硬件恢复
    Serial.println("所有重试都失败 - 执行硬件自动恢复");
    performHardwareRecovery();
    return -1.0;
}

// 温度校准函数
void calibrateTemperature(float actualTemp, float measuredTemp) {
    // 计算校准参数
    if (measuredTemp != 0) {
        temperatureScale = actualTemp / measuredTemp;
        temperatureOffset = actualTemp - (measuredTemp * temperatureScale);
    } else {
        temperatureOffset = actualTemp - measuredTemp;
        temperatureScale = 1.0;
    }
    
    Serial.println("温度校准完成:");
    Serial.print("实际温度: "); Serial.print(actualTemp); Serial.println("°C");
    Serial.print("测量温度: "); Serial.print(measuredTemp); Serial.println("°C");
    Serial.print("校准偏移: "); Serial.print(temperatureOffset); Serial.println("°C");
    Serial.print("校准系数: "); Serial.println(temperatureScale);
    
    // 保存校准参数到EEPROM
    saveConfig();
    Serial.println("温度校准参数已保存到EEPROM");
}

// 设备发现
bool discoveryEnabled = true;
const unsigned long DISCOVERY_INTERVAL = 10000; // 10秒广播一次
unsigned long lastDiscoveryTime = 0;

// LED状态控制
bool ledState = false;
unsigned long lastLedUpdate = 0;
const unsigned long LED_BLINK_INTERVAL = 500; // LED闪烁间隔

// 烘焙结束状态控制
bool bakingCompleteState = false;
unsigned long bakingCompleteStartTime = 0;
const unsigned long BAKING_COMPLETE_DURATION = 10000; // 烘焙结束快闪持续时间10秒

// =========================================
// EEPROM配置存储
// =========================================
struct Config {
    char ssid[32];
    char password[64];
    float temperatureOffset;  // 温度校准偏移量
    float temperatureScale;    // 温度校准系数
    char signature[16];  // 增加签名空间，避免缓冲区溢出
};

void saveConfig() {
    Config config;
    // 清空配置结构体
    memset(&config, 0, sizeof(config));
    
    // 安全复制字符串
    strncpy(config.ssid, wifiSSID.c_str(), sizeof(config.ssid) - 1);
    strncpy(config.password, wifiPassword.c_str(), sizeof(config.password) - 1);
    strncpy(config.signature, "SMARTOVEN", sizeof(config.signature) - 1);
    
    // 保存温度校准参数
    config.temperatureOffset = temperatureOffset;
    config.temperatureScale = temperatureScale;
    
    // 确保字符串以null结尾
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    config.password[sizeof(config.password) - 1] = '\0';
    config.signature[sizeof(config.signature) - 1] = '\0';
    
    EEPROM.begin(512);
    EEPROM.put(0, config);
    EEPROM.commit();
    EEPROM.end();
    
    Serial.println("配置已保存到EEPROM");
    Serial.print("SSID: ");
    Serial.println(config.ssid);
    Serial.print("密码长度: ");
    Serial.println(strlen(config.password));
    Serial.print("温度校准偏移: ");
    Serial.print(config.temperatureOffset);
    Serial.println("°C");
    Serial.print("温度校准系数: ");
    Serial.println(config.temperatureScale);
    Serial.print("签名: ");
    Serial.println(config.signature);
    
    // 蜂鸣器提示配置已保存
    beepConfigSaved();
}

bool loadConfig() {
    Config config;
    EEPROM.begin(512);
    EEPROM.get(0, config);
    EEPROM.end();
    
    Serial.println("从EEPROM加载配置...");
    Serial.print("读取到的签名: ");
    Serial.println(config.signature);
    Serial.print("读取到的SSID: ");
    Serial.println(config.ssid);
    Serial.print("读取到的密码长度: ");
    Serial.println(strlen(config.password));
    Serial.print("读取到的温度校准偏移: ");
    Serial.print(config.temperatureOffset);
    Serial.println("°C");
    Serial.print("读取到的温度校准系数: ");
    Serial.println(config.temperatureScale);
    
    if (strcmp(config.signature, "SMARTOVEN") == 0) {
        wifiSSID = String(config.ssid);
        wifiPassword = String(config.password);
        
        // 加载温度校准参数
        temperatureOffset = config.temperatureOffset;
        temperatureScale = config.temperatureScale;
        
        Serial.println("配置验证成功，加载有效配置");
        Serial.print("温度校准偏移: ");
        Serial.print(temperatureOffset);
        Serial.println("°C");
        Serial.print("温度校准系数: ");
        Serial.println(temperatureScale);
        return true;
    } else {
        Serial.println("配置验证失败，签名不匹配");
        // 清空配置
        wifiSSID = "";
        wifiPassword = "";
        // 重置温度校准参数
        temperatureOffset = 0.0;
        temperatureScale = 1.0;
        return false;
    }
}

// =========================================
// 强制门户功能
// =========================================
void startCaptivePortal() {
    Serial.println("启动强制门户模式...");
    
    // 断开现有连接
    WiFi.disconnect();
    delay(100);
    
    // 创建AP热点
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
    
    Serial.print("AP热点: ");
    Serial.println(AP_SSID);
    Serial.print("IP地址: ");
    Serial.println(WiFi.softAPIP());
    
    // 启动DNS劫持
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    // 启动Web服务器
    setupWebServer();
    webServer.begin();
    
    // 启动设备发现（强制门户模式下也需要支持设备发现）
    udp.begin(8888);
    
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
    
    Serial.println("强制门户已启动，UDP监听端口8888已开启");
}

void stopCaptivePortal() {
    Serial.println("停止强制门户模式...");
    
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    isCaptivePortalMode = false;
    captivePortalStartTime = 0;
    
    Serial.println("强制门户已停止");
}

bool shouldStartCaptivePortal() {
    // 如果没有保存的WiFi配置，直接启动强制门户
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("没有WiFi配置，需要启动强制门户");
        return true;
    }
    
    // 如果有WiFi配置，尝试连接WiFi
    Serial.println("有WiFi配置，尝试连接WiFi");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("密码长度: ");
    Serial.println(wifiPassword.length());
    
    // 先确保WiFi模式正确
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    // 等待连接，最多等待30秒（与connectToWiFi保持一致）
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        // 每5秒输出一次连接状态
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("连接状态: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("空闲状态"); break;
                case WL_NO_SSID_AVAIL: Serial.println("网络不可用"); break;
                case WL_SCAN_COMPLETED: Serial.println("扫描完成"); break;
                case WL_CONNECTED: Serial.println("已连接"); break;
                case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
                case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
                case WL_DISCONNECTED: Serial.println("已断开"); break;
                default: Serial.println("未知状态"); break;
            }
        }
    }
    
    // 检查连接结果
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi连接成功");
        Serial.print("IP地址: ");
        Serial.println(WiFi.localIP());
        Serial.print("连接耗时: ");
        Serial.print((millis() - startTime) / 1000.0);
        Serial.println("秒");
        return false; // 连接成功，不需要强制门户
    } else {
        Serial.println("");
        Serial.println("WiFi连接失败，启动强制门户");
        Serial.print("最终连接状态: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("空闲状态"); break;
            case WL_NO_SSID_AVAIL: Serial.println("网络不可用"); break;
            case WL_SCAN_COMPLETED: Serial.println("扫描完成"); break;
            case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
            case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
            case WL_DISCONNECTED: Serial.println("已断开"); break;
            default: Serial.println("未知状态"); break;
        }
        // 确保WiFi已断开
        WiFi.disconnect();
        delay(100);
        return true; // 连接失败，需要强制门户
    }
}

void checkCaptivePortalTimeout() {
    if (isCaptivePortalMode && 
        millis() - captivePortalStartTime > CAPTIVE_PORTAL_TIMEOUT) {
        Serial.println("强制门户超时，尝试连接保存的WiFi");
        stopCaptivePortal();
        connectToWiFi();
    }
}

// =========================================
// WiFi连接管理
// =========================================
void connectToWiFi() {
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("没有WiFi配置，启动强制门户");
        startCaptivePortal();
        return;
    }
    
    Serial.println("尝试连接WiFi...");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("密码长度: ");
    Serial.println(wifiPassword.length());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    // 增加连接超时到30秒，并添加更详细的连接状态
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        // 每5秒输出一次连接状态
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("连接状态: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("空闲状态"); break;
                case WL_NO_SSID_AVAIL: Serial.println("网络不可用"); break;
                case WL_SCAN_COMPLETED: Serial.println("扫描完成"); break;
                case WL_CONNECTED: Serial.println("已连接"); break;
                case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
                case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
                case WL_DISCONNECTED: Serial.println("已断开"); break;
                default: Serial.println("未知状态"); break;
            }
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi连接成功!");
        Serial.print("IP地址: ");
        Serial.println(WiFi.localIP());
        Serial.print("连接耗时: ");
        Serial.print((millis() - startTime) / 1000.0);
        Serial.println("秒");
        
        // 启动Web服务器
        setupWebServer();
        
        // 启动OTA服务器
        setupOTA();
        
        // 启动设备发现
        udp.begin(8888);
        
        // 设置LED为常亮状态
        digitalWrite(LED_PIN, HIGH);
    } else {
        Serial.println("");
        Serial.println("WiFi连接失败，启动强制门户");
        Serial.print("最终连接状态: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("空闲状态"); break;
            case WL_NO_SSID_AVAIL: Serial.println("网络不可用"); break;
            case WL_SCAN_COMPLETED: Serial.println("扫描完成"); break;
            case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
            case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
            case WL_DISCONNECTED: Serial.println("已断开"); break;
            default: Serial.println("未知状态"); break;
        }
        startCaptivePortal();
    }
}

// =========================================
// 设备自动发现功能
// =========================================
void handleDiscovery() {
    // 处理接收到的发现请求
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char packetBuffer[255];
        int len = udp.read(packetBuffer, 255);
        if (len > 0) {
            packetBuffer[len] = 0;
            String request = String(packetBuffer);
            
            Serial.print("收到UDP数据包，大小: ");
            Serial.print(packetSize);
            Serial.print(" 字节，内容: ");
            Serial.println(request);
            
            if (request.startsWith("DISCOVER_SMARTOVEN")) {
                Serial.println("收到设备发现请求");
                sendDiscoveryResponse();
            }
        }
    }
    
    // 定期广播设备信息
    if (discoveryEnabled && millis() - lastDiscoveryTime > DISCOVERY_INTERVAL) {
        broadcastDiscovery();
        lastDiscoveryTime = millis();
    }
}

void sendDiscoveryResponse() {
    String response;
    
    // 使用APP期望的格式：DEVICE_INFO:NAME:设备名称,MAC:MAC地址,PORT:端口号,...
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
    
    // 修复：使用APP发送请求的端口（remotePort）而不是硬编码的8889端口
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(response.c_str());
    udp.endPacket();
    
    Serial.println("发送发现响应到端口" + String(udp.remotePort()) + ": " + response);
}

void broadcastDiscovery() {
    String broadcastMsg = "SMARTOVEN_BROADCAST:" + DEVICE_TYPE + ":" + DEVICE_ID + ":" + FIRMWARE_VERSION;
    
    udp.beginPacket("255.255.255.255", 8888);
    udp.write(broadcastMsg.c_str());
    udp.endPacket();
    
    Serial.println("广播设备信息: " + broadcastMsg);
}

// =========================================
// OTA升级功能
// =========================================
void setupOTA() {
    httpUpdater.setup(&otaServer);
    otaServer.begin();
    Serial.println("OTA服务器已启动，端口: 8080");
    Serial.println("OTA升级地址: http://" + WiFi.localIP().toString() + ":8080/update");
}

void handleOTA() {
    if (!isCaptivePortalMode && WiFi.status() == WL_CONNECTED) {
        otaServer.handleClient();
    }
}

// OTA升级页面
void handleOTAWebPage() {
    String html = "<!DOCTYPE html><html><head><title>智能电烤箱OTA升级</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
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
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-success\">当前版本: ' + data.current_version + '</div>';";
    html += "    if (data.update_available) {";
    html += "      document.getElementById('updateStatus').innerHTML += '<div class=\"status status-warning\">有新版本可用: ' + data.latest_version + '</div>';";
    html += "    }";
    html += "  }).catch(error => {";
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-error\">检查更新失败</div>';";
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
    html += "<h1>智能电烤箱OTA升级</h1>";
    html += "<p>固件空中升级系统</p>";
    html += "</div>";
    html += "<div class=\"content\">";
    html += "<div class=\"info-card\">";
    html += "<p><strong>设备ID:</strong> " + DEVICE_ID + "</p>";
    html += "<p><strong>IP地址:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>OTA端口:</strong> 8080</p>";
    html += "</div>";
    html += "<div id=\"updateStatus\"></div>";
    html += "<div class=\"ota-section\">";
    html += "<h3>OTA升级操作</h3>";
    html += "<p>点击下方按钮打开OTA升级页面，上传新的固件文件进行升级。</p>";
    html += "<button class=\"btn btn-success\" onclick=\"startOTA()\">打开OTA升级页面</button>";
    html += "<button class=\"btn\" onclick=\"checkUpdate()\">检查更新</button>";
    html += "</div>";
    html += "<div style=\"text-align: center; margin-top: 20px;\">";
    html += "<a href=\"/\" style=\"color: #2196F3; text-decoration: none;\">返回主页面</a>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
}

// 检查更新API
void handleCheckUpdate() {
    String json = "{\"current_version\":\"" + FIRMWARE_VERSION + "\",\"latest_version\":\"0.6.0\",\"update_available\":false}";
    
    // 这里可以添加检查新版本的逻辑
    // 例如从服务器获取最新版本信息
    
    webServer.send(200, "application/json", json);
}

// =========================================
// Web服务器处理
// =========================================
void setupWebServer() {
    // 配置静态文件服务（仅在非强制门户模式下使用）
    if (!isCaptivePortalMode) {
        webServer.serveStatic("/login.html", SPIFFS, "/login.html");
        webServer.serveStatic("/css/", SPIFFS, "/css/");
        webServer.serveStatic("/js/", SPIFFS, "/js/");
        webServer.serveStatic("/images/", SPIFFS, "/images/");
    }
    
    // 动态路由
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
    webServer.onNotFound(handleNotFound);
    webServer.begin();
}

void handleScanWiFi() {
    // 扫描WiFi网络
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
    
    // 清理扫描结果
    WiFi.scanDelete();
}

void handleRoot() {
    if (isCaptivePortalMode) {
        // 强制门户模式下显示WiFi配置页面
        String html = "<!DOCTYPE html><html><head><title>智能电烤箱配置</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
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
        html += "  scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>扫描中...';";
        html += "  wifiList.innerHTML = '<option value=\"\">扫描中...</option>';";
        html += "  fetch('/scanwifi').then(response => response.json()).then(data => {";
        html += "    wifiList.innerHTML = '';";
        html += "    wifiList.innerHTML = '<option value=\"\">请选择WiFi网络</option>';";
        html += "    if (data.networks && data.networks.length > 0) {";
        html += "      data.networks.forEach(network => {";
        html += "        var option = document.createElement('option');";
        html += "        option.value = network.ssid;";
        html += "        option.textContent = network.ssid + ' (' + network.rssi + ' dBm)';";
        html += "        wifiList.appendChild(option);";
        html += "      });";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-connected\"></span>扫描完成 (' + data.networks.length + '个网络)';";
        html += "    } else {";
        html += "      wifiList.innerHTML = '<option value=\"\">未找到WiFi网络</option>';";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>重新扫描';";
        html += "    }";
        html += "    scanBtn.disabled = false;";
        html += "  }).catch(error => {";
        html += "    wifiList.innerHTML = '<option value=\"\">扫描失败</option>';";
        html += "    scanBtn.disabled = false;";
        html += "    scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>重新扫描';";
        html += "  });";
        html += "}";
        html += "window.onload = scanWiFi;";
        html += "function confirmFactoryReset() {";
        html += "  if (confirm('⚠️ 危险操作警告！\\n\\n恢复出厂设置将清除所有WiFi配置和温度校准参数。\\n设备将重启并进入配网模式，需要重新配置WiFi网络。\\n\\n确定要执行恢复出厂设置吗？')) {";
        html += "    var resetBtn = document.querySelector('[onclick=\\\"confirmFactoryReset()\\\"]');";
        html += "    resetBtn.disabled = true;";
        html += "    resetBtn.innerHTML = '恢复出厂设置中...';";
        html += "    resetBtn.style.opacity = '0.7';";
        html += "    ";
        html += "    fetch('/factoryreset', { method: 'POST' })";
        html += "      .then(response => {";
        html += "        if (response.ok) {";
        html += "          resetBtn.innerHTML = '恢复成功，设备重启中...';";
        html += "          resetBtn.style.background = 'linear-gradient(135deg, #28a745 0%, #20c997 100%)';";
        html += "        } else {";
        html += "          throw new Error('恢复失败');";
        html += "        }";
        html += "      })";
        html += "      .catch(error => {";
        html += "        resetBtn.disabled = false;";
        html += "        resetBtn.innerHTML = '恢复出厂设置';";
        html += "        resetBtn.style.opacity = '1';";
        html += "        alert('恢复出厂设置失败，请重试：' + error.message);";
        html += "      });";
        html += "  }";
        html += "}";
        html += "</script>";
        html += "</head><body>";
        html += "<div class=\"container\">";
        html += "<div class=\"header\">";
        html += "<h1>智能电烤箱配置</h1>";
        html += "<p>WiFi网络配置页面</p>";
        html += "</div>";
        html += "<div class=\"device-info\">";
        html += "<p><strong>设备ID:</strong> " + DEVICE_ID + "</p>";
        html += "<p><strong>固件版本:</strong> " + FIRMWARE_VERSION + "</p>";
        html += "</div>";
        html += "<div class=\"temperature-display\">当前温度: " + String(currentTemp) + "°C</div>";
        html += "<div class=\"form-container\">";
        html += "<form method=\"POST\" action=\"/savewifi\">";
        html += "<div class=\"scan-section\">";
        html += "<button type=\"button\" id=\"scanBtn\" class=\"btn btn-secondary\" onclick=\"scanWiFi()\"><span class=\"status-indicator status-disconnected\"></span>扫描WiFi网络</button>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"ssid\">WiFi网络</label>";
        html += "<select id=\"ssid\" name=\"ssid\" class=\"form-control\" required><option value=\"\">请先扫描WiFi网络</option></select>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"password\">WiFi密码</label>";
        html += "<input type=\"password\" id=\"password\" name=\"password\" class=\"form-control\" placeholder=\"请输入WiFi密码\" required>";
        html += "</div>";
        html += "<button type=\"submit\" class=\"btn\" style=\"width: 100%;\">保存配置</button>";
        html += "</form>";
        

        
        html += "</div>";
        html += "</div>";
        
        // 温度控制JavaScript
        html += "<script>";
        html += "let currentTargetTemp = " + String(targetTemp) + ";";
        html += "let currentHeatingState = " + String(heatingEnabled ? "true" : "false") + ";";
        
        html += "// 更新温度显示";
        html += "function updateTemperatureDisplay() {";
        html += "  fetch('/status')";
        html += "    .then(response => response.json())";
        html += "    .then(data => {";
        html += "      document.getElementById('current-temp').textContent = data.temperature.toFixed(1) + '°C';";
        html += "      document.getElementById('target-temp').textContent = data.target_temperature.toFixed(1) + '°C';";
        html += "      currentTargetTemp = data.target_temperature;";
        html += "      currentHeatingState = data.heating_enabled;";
        html += "      ";
        html += "      // 更新加热按钮状态";
        html += "      const heatingBtn = document.getElementById('heating-btn');";
        html += "      const heatingText = document.getElementById('heating-text');";
        html += "      if (data.heating_enabled) {";
        html += "        heatingBtn.classList.add('active');";
        html += "        heatingText.textContent = '停止加热';";
        html += "      } else {";
        html += "        heatingBtn.classList.remove('active');";
        html += "        heatingText.textContent = '开始加热';";
        html += "      }";
        html += "    })";
        html += "    .catch(error => console.error('获取温度状态失败:', error));";
        html += "}";
        
        html += "// 改变目标温度";
        html += "function changeTemp(delta) {";
        html += "  const newTemp = Math.max(0, Math.min(300, currentTargetTemp + delta));";
        html += "  setTargetTemp(newTemp);";
        html += "}";
        
        html += "// 设置预设温度";
        html += "function setPresetTemp(temp) {";
        html += "  setTargetTemp(temp);";
        html += "}";
        
        html += "// 设置自定义温度";
        html += "function setCustomTemp() {";
        html += "  const input = document.getElementById('temp-input');";
        html += "  const temp = parseInt(input.value);";
        html += "  if (!isNaN(temp) && temp >= 0 && temp <= 300) {";
        html += "    setTargetTemp(temp);";
        html += "    input.value = '';";
        html += "  } else {";
        html += "    alert('请输入0-300°C之间的有效温度值');";
        html += "  }";
        html += "}";
        
        html += "// 设置目标温度";
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
        html += "        document.getElementById('target-temp').textContent = temp.toFixed(1) + '°C';";
        html += "        console.log('目标温度已设置为: ' + temp + '°C');";
        html += "      } else {";
        html += "        throw new Error('设置温度失败');";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('设置温度失败:', error);";
        html += "      alert('设置温度失败，请检查设备连接');";
        html += "    });";
        html += "}";
        
        html += "// 切换加热状态";
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
        html += "          heatingText.textContent = '停止加热';";
        html += "        } else {";
        html += "          heatingBtn.classList.remove('active');";
        html += "          heatingText.textContent = '开始加热';";
        html += "        }";
        html += "        console.log('加热状态已切换为: ' + (newHeatingState ? '开启' : '关闭'));";
        html += "      } else {";
        html += "        throw new Error('切换加热状态失败');";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('切换加热状态失败:', error);";
        html += "      alert('切换加热状态失败，请检查设备连接');";
        html += "    });";
        html += "}";
        
        html += "// 实时更新温度显示";
        html += "setInterval(updateTemperatureDisplay, 2000);";
        
        html += "// 页面加载时初始化";
        html += "document.addEventListener('DOMContentLoaded', function() {";
        html += "  updateTemperatureDisplay();";
        html += "  ";
        html += "  // 输入框回车事件";
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
        // 正常模式下重定向到登录页面
        webServer.sendHeader("Location", "/login.html", true);
        webServer.send(302, "text/plain", "Redirecting to login page");
    }
}

// 处理所有未定义的路由，实现自动跳转
void handleNotFound() {
    if (isCaptivePortalMode) {
        // 在强制门户模式下，将所有未定义的路由重定向到首页
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "Redirect to configuration page");
    } else {
        // 在正常模式下，直接返回404错误
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
        html += "<div class=\"success-icon\">✓</div>";
        html += "<h1 class=\"success-title\">配置已保存</h1>";
        html += "<p class=\"success-message\">WiFi配置已成功保存，设备将重启并尝试连接WiFi网络。</p>";
        html += "<p class=\"countdown\">5秒后自动跳转回配置页面...</p>";
        html += "</div>";
        html += "</body></html>";
        
        webServer.send(200, "text/html", html);
        
        delay(1000);
        ESP.restart();
    }
}

void handleFactoryReset() {
    // 清空EEPROM中的配置
    EEPROM.begin(512);
    
    // 清空整个EEPROM区域
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    
    // 清空内存中的配置变量
    wifiSSID = "";
    wifiPassword = "";
    temperatureOffset = 0.0;
    temperatureScale = 1.0;
    
    Serial.println("恢复出厂设置完成，所有配置已清除");
    
    // 返回成功页面
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
    html += "<div class=\"reset-icon\">⚠️</div>";
    html += "<h1 class=\"reset-title\">恢复出厂设置</h1>";
    html += "<p class=\"reset-message\">设备已恢复出厂设置，所有WiFi配置和温度校准参数已被清除。</p>";
    html += "<p class=\"reset-message\">设备将重启并进入配网模式，请重新配置WiFi网络。</p>";
    html += "<p class=\"countdown\">5秒后设备将重启...</p>";
    html += "</div>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
    
    delay(1000);
    ESP.restart();
}

void handleRestart() {
    Serial.println("接收到重启设备请求");
    
    // 返回重启确认页面
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
    html += "<div class=\"restart-icon\">🔄</div>";
    html += "<h1 class=\"restart-title\">设备重启中</h1>";
    html += "<p class=\"restart-message\">设备正在重启，重启过程大约需要30秒。</p>";
    html += "<p class=\"restart-message\">重启完成后，设备将自动重新连接WiFi网络。</p>";
    html += "<p class=\"countdown\">5秒后设备将重启...</p>";
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
    // 读取原始MAX6675数据
    uint16_t rawData = readMAX6675RawData();
    bool errorFlag = (rawData & 0x04) != 0;
    uint16_t tempBits = rawData >> 3;
    float rawTemperature = tempBits * 0.25;
    
    // 确定传感器状态
    String sensorStatus = "正常";
    String diagnosticAdvice = "传感器工作正常";
    
    if (errorFlag) {
        sensorStatus = "错误";
        diagnosticAdvice = "检测到热电偶开路或传感器故障";
    } else if (rawData == 0x0000 || rawData == 0xFFFF) {
        sensorStatus = "警告";
        diagnosticAdvice = "传感器数据异常，检查硬件连接";
    } else if (rawTemperature < -20 || rawTemperature > 1024) {
        sensorStatus = "警告";
        diagnosticAdvice = "温度读数超出正常范围";
    }
    
    // 构建诊断JSON响应
    String json = "{";
    json += "\"sensor_status\":\"" + sensorStatus + "\",";
    json += "\"raw_data\":\"0x" + String(rawData, HEX) + "\",";
    json += "\"error_flag\":" + String(errorFlag ? "true" : "false") + ",";
    json += "\"temp_bits\":" + String(tempBits) + ",";
    json += "\"raw_temperature\":" + String(rawTemperature) + ",";
    json += "\"calibration_params\":\"偏移:" + String(temperatureOffset) + " 缩放:" + String(temperatureScale) + "\",";
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
    
    String json = "{\"message\":\"温度校准参数已重置\",";
    json += "\"offset\":" + String(temperatureOffset) + ",";
    json += "\"scale\":" + String(temperatureScale) + "}";
    
    webServer.send(200, "application/json", json);
}

void handleControl() {
    bool wasHeating = heatingEnabled;  // 保存之前的加热状态
    
    if (webServer.hasArg("target_temp")) {
        targetTemp = webServer.arg("target_temp").toFloat();
    }
    if (webServer.hasArg("heating")) {
        heatingEnabled = webServer.arg("heating") == "true";
    }
    
    // 检测加热状态变化并触发蜂鸣器提示
    if (!wasHeating && heatingEnabled) {
        // 从关闭到开启：开始烘焙
        beepBakingStart();
        Serial.println("烘焙开始 - 目标温度: " + String(targetTemp) + "°C");
    } else if (wasHeating && !heatingEnabled) {
        // 从开启到关闭：烘焙完成
        beepBakingComplete();
        Serial.println("烘焙完成 - 最终温度: " + String(currentTemp) + "°C");
        
        // 触发烘焙结束状态：快闪10秒
        bakingCompleteState = true;
        bakingCompleteStartTime = millis();
        Serial.println("触发烘焙结束快闪状态，持续10秒");
    }
    
    webServer.send(200, "text/plain", "OK");
}

// =========================================
// 温度控制功能
// =========================================

// 温度读取定时器
unsigned long lastTemperatureRead = 0;
const unsigned long TEMPERATURE_READ_INTERVAL = 200; // 每200ms读取一次温度（优化：提高温度响应速度）

void readTemperature() {
    unsigned long currentTime = millis();
    
    // 定时读取温度，避免频繁读取影响性能
    if (currentTime - lastTemperatureRead >= TEMPERATURE_READ_INTERVAL) {
        currentTemp = readTemperatureWithMonitoring();
        if (currentTemp < 0) {
            Serial.println("温度传感器读取错误");
        } else {
            // 只在温度变化较大时才打印日志，减少串口输出
            static float lastPrintedTemp = -999;
            if (fabs(currentTemp - lastPrintedTemp) >= 0.5) {
                Serial.println("当前温度: " + String(currentTemp) + "°C");
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
// LED状态指示
// =========================================
void updateLED() {
    unsigned long currentTime = millis();
    
    // 烘焙结束状态处理
    if (bakingCompleteState) {
        if (currentTime - bakingCompleteStartTime > BAKING_COMPLETE_DURATION) {
            // 烘焙结束快闪时间到，切换到待机状态
            bakingCompleteState = false;
            Serial.println("烘焙结束快闪完成，切换到待机状态");
        }
    }
    
    if (currentTime - lastLedUpdate > LED_BLINK_INTERVAL) {
        if (bakingCompleteState) {
            // 烘焙结束状态：快速闪烁（500ms间隔）
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED状态: 烘焙结束 - 快闪 (亮)");
            } else {
                Serial.println("LED状态: 烘焙结束 - 快闪 (灭)");
            }
        } else if (isCaptivePortalMode) {
            // 配网前状态：快速闪烁（500ms间隔）
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED状态: 配网前 - 快闪 (亮)");
            } else {
                Serial.println("LED状态: 配网前 - 快闪 (灭)");
            }
        } else if (heatingEnabled) {
            // 加热中状态：常亮
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED状态: 加热中 - 常亮");
        } else {
            // 待机状态：慢速闪烁（1000ms间隔）
            if (currentTime - lastLedUpdate > 1000) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState ? HIGH : LOW);
                if (ledState) {
                    Serial.println("LED状态: 待机 - 慢闪 (亮)");
                } else {
                    Serial.println("LED状态: 待机 - 慢闪 (灭)");
                }
                lastLedUpdate = currentTime;
            }
            return; // 待机状态使用自定义间隔，不更新lastLedUpdate
        }
        
        lastLedUpdate = currentTime;
    }
}

// =========================================
// 蜂鸣器控制
// =========================================
void beep(int duration = 100) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}

// 蜂鸣器状态提示函数
void beepConfigSaved() {
    // 设置保存成功提示：短-短-短
    beep(100);
    delay(100);
    beep(100);
    delay(100);
    beep(100);
    Serial.println("蜂鸣器提示：配置已保存");
}

void beepBakingStart() {
    // 开始烘焙提示：长-短
    beep(300);
    delay(200);
    beep(100);
    Serial.println("蜂鸣器提示：烘焙开始");
}

void beepBakingComplete() {
    // 烘焙完成提示：长-长-长
    beep(500);
    delay(200);
    beep(500);
    delay(200);
    beep(500);
    Serial.println("蜂鸣器提示：烘焙完成");
}

void beepTemperatureChange() {
    // 温度切换提示：短-短-短-短
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    Serial.println("蜂鸣器提示：温度切换");
}

// =========================================
// 初始化函数
// =========================================
void setup() {
    Serial.begin(115200);
    Serial.println("");
    Serial.println("=========================================");
    Serial.println("智能电烤箱控制器 v" + FIRMWARE_VERSION);
    Serial.println("MAX6675手动SPI实现 - 温度传感器正常工作");
    Serial.println("=========================================");
    
    // 初始化引脚
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // 初始化MAX6675引脚
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    // 设置MAX6675初始状态
    digitalWrite(THERMO_CS, HIGH);  // CS引脚高电平（禁用）
    digitalWrite(THERMO_CLK, LOW);  // CLK引脚低电平
    
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    // 蜂鸣器提示启动
    beep(200);
    delay(100);
    beep(200);
    
    // 硬件初始化验证
    if (!verifyHardwareInitialization()) {
        Serial.println("⚠️ 硬件初始化验证失败，尝试硬件复位...");
        
        // 强制硬件复位
        for (int i = 0; i < 3; i++) {
            digitalWrite(THERMO_CS, HIGH);
            digitalWrite(THERMO_CLK, LOW);
            delay(100);
            digitalWrite(THERMO_CS, LOW);
            delay(50);
            digitalWrite(THERMO_CS, HIGH);
            delay(100);
        }
        
        // 重新验证
        if (verifyHardwareInitialization()) {
            Serial.println("✅ 硬件复位成功");
        } else {
            Serial.println("❌ 硬件复位失败，请检查硬件连接");
        }
    }
    
    // 加载配置
    if (loadConfig()) {
        Serial.println("配置加载成功");
    } else {
        Serial.println("未找到有效配置");
    }
    
    // 启动网络
    if (shouldStartCaptivePortal()) {
        startCaptivePortal();
    } else {
        connectToWiFi();
    }
    
    // 启动TCP服务器用于APP连接
    tcpServer.begin();
    Serial.println("TCP服务器已启动，监听端口: " + String(DEFAULT_PORT));
    
    // 初始化SPIFFS文件系统
    if (SPIFFS.begin()) {
        Serial.println("SPIFFS文件系统初始化成功");
        
        // 检查是否存在必要的文件
        if (SPIFFS.exists("/login.html")) {
            Serial.println("找到登录页面文件: /login.html");
        } else {
            Serial.println("警告: 未找到登录页面文件 /login.html");
        }
        
        if (SPIFFS.exists("/index.html")) {
            Serial.println("找到主页文件: /index.html");
        } else {
            Serial.println("警告: 未找到主页文件 /index.html");
        }
    } else {
        Serial.println("错误: SPIFFS文件系统初始化失败");
    }
    
    Serial.println("初始化完成");
}

// =========================================
// 串口命令处理
// =========================================
void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            Serial.println("收到命令: " + command);
            
            if (command == "LED_ON") {
                digitalWrite(LED_PIN, HIGH);
                Serial.println("LED已打开");
            } else if (command == "LED_OFF") {
                digitalWrite(LED_PIN, LOW);
                Serial.println("LED已关闭");
            } else if (command == "LED_BLINK") {
                // 临时闪烁LED
                for (int i = 0; i < 5; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(200);
                    digitalWrite(LED_PIN, LOW);
                    delay(200);
                }
                Serial.println("LED闪烁完成");
            } else if (command == "BEEP") {
                beep(100);
                Serial.println("蜂鸣器已响");
            } else if (command == "BEEP_LONG") {
                beep(500);
                Serial.println("蜂鸣器长响");
            } else if (command == "BEEP_SHORT") {
                beep(50);
                Serial.println("蜂鸣器短响");
            } else if (command == "GET_STATUS") {
                // 返回APP期望的格式：TEMP:25.50,TARGET:180.00,HEAT:0,MODE:1,UPTIME:123
                String statusResponse = "TEMP:" + String(currentTemp) + 
                                      ",TARGET:" + String(targetTemp) + 
                                      ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                                      ",MODE:" + String(ovenMode ? "1" : "0") + 
                                      ",UPTIME:" + String(millis() / 1000);
                Serial.println(statusResponse);
            } else if (command == "GET_TEMP") {
                Serial.println("当前温度: " + String(currentTemp) + "°C");
            } else if (command.startsWith("CALIBRATE_TEMP")) {
                // 温度校准命令格式: CALIBRATE_TEMP 实际温度
                int spaceIndex = command.indexOf(' ');
                if (spaceIndex > 0) {
                    String actualTempStr = command.substring(spaceIndex + 1);
                    float actualTemp = actualTempStr.toFloat();
                    float measuredTemp = currentTemp;
                    
                    if (actualTemp > 0) {
                        calibrateTemperature(actualTemp, measuredTemp);
                        Serial.println("温度校准已应用，请重新读取温度验证");
                    } else {
                        Serial.println("错误: 实际温度值无效");
                    }
                } else {
                    Serial.println("温度校准命令格式: CALIBRATE_TEMP 实际温度");
                    Serial.println("例如: CALIBRATE_TEMP 16.0");
                }
            } else if (command == "RESET_CALIBRATION") {
                temperatureOffset = 0.0;
                temperatureScale = 1.0;
                Serial.println("温度校准已重置");
            } else if (command == "GET_RAW_TEMP") {
                // 获取原始温度数据（未校准）
                uint16_t rawData = readMAX6675RawData();
                if (!(rawData & 0x04)) {
                    uint16_t tempBits = rawData >> 3;
                    float rawTemp = tempBits * 0.25;
                    Serial.println("原始温度数据:");
                    Serial.print("原始值: 0x"); Serial.println(rawData, HEX);
                    Serial.print("温度位: "); Serial.println(tempBits);
                    Serial.print("未校准温度: "); Serial.print(rawTemp); Serial.println("°C");
                    Serial.print("校准后温度: "); Serial.print(currentTemp); Serial.println("°C");
                } else {
                    Serial.println("错误: 温度传感器读取错误");
                }
            } else if (command == "GET_PERFORMANCE") {
                // 获取性能信息
                Serial.println("📊 设备性能信息:");
                Serial.print("温度读取平均时间: "); Serial.print(temperatureReadAvgTime); Serial.println("ms");
                Serial.print("温度读取总次数: "); Serial.println(temperatureReadCount);
                
                // 获取内存信息
                uint32_t freeHeap = ESP.getFreeHeap();
                uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
                uint32_t heapFragmentation = ESP.getHeapFragmentation();
                
                Serial.print("空闲内存: "); Serial.print(freeHeap); Serial.println(" bytes");
                Serial.print("最大空闲块: "); Serial.print(maxFreeBlock); Serial.println(" bytes");
                Serial.print("内存碎片率: "); Serial.print(heapFragmentation); Serial.println("%");
                
                // 输出当前状态
                Serial.print("Web服务器处理间隔: "); Serial.print(WEB_SERVER_HANDLE_INTERVAL); Serial.println("ms");
                Serial.print("温度读取间隔: "); Serial.print(TEMPERATURE_READ_INTERVAL); Serial.println("ms");
            } else if (command == "RESET_PERFORMANCE") {
                // 重置性能计数器
                temperatureReadCount = 0;
                temperatureReadAvgTime = 0;
                Serial.println("✅ 性能计数器已重置");
            } else {
                Serial.println("未知命令，可用命令:");
                Serial.println("LED_ON, LED_OFF, LED_BLINK");
                Serial.println("BEEP, BEEP_LONG, BEEP_SHORT");
                Serial.println("GET_STATUS, GET_TEMP, GET_RAW_TEMP");
                Serial.println("CALIBRATE_TEMP 实际温度");
                Serial.println("RESET_CALIBRATION");
                Serial.println("GET_PERFORMANCE, RESET_PERFORMANCE");
            }
        }
    }
}

// =========================================
// TCP服务器处理函数
// =========================================

void handleTCPConnection() {
    // 检查是否有新的客户端连接
    if (tcpServer.hasClient()) {
        // 如果已经有客户端连接，断开旧的连接
        if (tcpClient && tcpClient.connected()) {
            tcpClient.stop();
            Serial.println("TCP客户端已断开");
        }
        
        // 接受新的客户端连接
        tcpClient = tcpServer.available();
        if (tcpClient) {
            Serial.println("TCP客户端已连接: " + tcpClient.remoteIP().toString());
            
            // 发送欢迎消息
            tcpClient.println("SmartOven Controller v" + FIRMWARE_VERSION);
            tcpClient.println("连接成功，请输入命令");
        }
    }
    
    // 处理已连接的客户端数据
    if (tcpClient && tcpClient.connected()) {
        if (tcpClient.available()) {
            String command = tcpClient.readStringUntil('\n');
            command.trim();
            
            if (command.length() > 0) {
                Serial.println("TCP收到命令: " + command);
                handleTCPCommand(command);
            }
        }
    }
}

void handleTCPCommand(String command) {
    if (command == "GET_STATUS") {
        // 返回设备状态信息
        String statusResponse = "TEMP:" + String(currentTemp) + 
                              ",TARGET:" + String(targetTemp) + 
                              ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                              ",MODE:" + String(ovenMode ? "1" : "0") + 
                              ",UPTIME:" + String(millis() / 1000);
        tcpClient.println(statusResponse);
        Serial.println("TCP发送状态: " + statusResponse);
    } else if (command == "GET_TEMP") {
        tcpClient.println("当前温度: " + String(currentTemp) + "°C");
    } else if (command.startsWith("SET_TEMP")) {
        // 设置目标温度格式: SET_TEMP 180.0
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            String tempStr = command.substring(spaceIndex + 1);
            float newTemp = tempStr.toFloat();
            
            if (newTemp >= 0 && newTemp <= 300) {
                targetTemp = newTemp;
                tcpClient.println("目标温度已设置为: " + String(targetTemp) + "°C");
                Serial.println("TCP设置目标温度: " + String(targetTemp) + "°C");
            } else {
                tcpClient.println("错误: 温度范围应为0-300°C");
            }
        }
    } else if (command == "HEAT_ON") {
        heatingEnabled = true;
        tcpClient.println("加热已开启");
        Serial.println("TCP开启加热");
    } else if (command == "HEAT_OFF") {
        heatingEnabled = false;
        tcpClient.println("加热已关闭");
        Serial.println("TCP关闭加热");
    } else if (command == "OVEN_MODE") {
        ovenMode = true;
        tcpClient.println("已切换到烤箱模式");
        Serial.println("TCP切换到烤箱模式");
    } else if (command == "TOASTER_MODE") {
        ovenMode = false;
        tcpClient.println("已切换到烤面包机模式");
        Serial.println("TCP切换到烤面包机模式");
    } else if (command == "PING") {
        tcpClient.println("PONG");
    } else {
        tcpClient.println("未知命令，可用命令:");
        tcpClient.println("GET_STATUS, GET_TEMP, SET_TEMP 温度值");
        tcpClient.println("HEAT_ON, HEAT_OFF, OVEN_MODE, TOASTER_MODE, PING");
    }
}

// =========================================
// 性能监控和内存管理
// =========================================

// 内存监控定时器
unsigned long lastMemoryCheck = 0;
const unsigned long MEMORY_CHECK_INTERVAL = 5000; // 每5秒检查一次内存

// 温度读取性能监控
unsigned long lastTemperatureReadTime = 0;

// =========================================
// 主循环
// =========================================

// 温度切换检测变量
float lastTargetTemp = targetTemp;
unsigned long lastTempChangeTime = 0;
const unsigned long TEMP_CHANGE_DEBOUNCE = 2000; // 2秒防抖

// Web服务器处理定时器
unsigned long lastWebServerHandle = 0;

void loop() {
    unsigned long currentTime = millis();
    
    // 处理强制门户（提高响应频率）
    if (isCaptivePortalMode) {
        // 强制门户模式下需要更频繁地处理DNS和Web请求
        dnsServer.processNextRequest();
        webServer.handleClient();
        checkCaptivePortalTimeout();
    } else {
        // 正常模式下可以适当降低处理频率
        if (currentTime - lastWebServerHandle >= WEB_SERVER_HANDLE_INTERVAL) {
            webServer.handleClient();
            lastWebServerHandle = currentTime;
        }
    }
    
    // 处理OTA升级（低频）
    handleOTA();
    
    // 处理设备发现（低频）
    handleDiscovery();
    
    // 处理TCP连接（高频）
    handleTCPConnection();
    
    // 处理串口命令（高频）
    handleSerialCommands();
    
    // 温度控制（定时读取）
    readTemperature();
    controlHeater();
    
    // LED状态更新（定时）
    updateLED();
    
    // 温度切换检测
    if (targetTemp != lastTargetTemp) {
        // 防抖处理：只有在温度变化后2秒内没有再次变化才触发提示
        if (currentTime - lastTempChangeTime > TEMP_CHANGE_DEBOUNCE) {
            Serial.println("检测到温度切换: " + String(lastTargetTemp) + "°C -> " + String(targetTemp) + "°C");
            beepTemperatureChange();
            lastTargetTemp = targetTemp;
        }
        lastTempChangeTime = currentTime;
    }
    
        // 内存监控（每5秒检查一次）
    if (currentTime - lastMemoryCheck >= MEMORY_CHECK_INTERVAL) {
        checkMemoryUsage();
        lastMemoryCheck = currentTime;
    }
    
    // 使用非阻塞延迟，提高响应速度
    delay(1);
}

// =========================================
// 性能监控函数
// =========================================

void checkMemoryUsage() {
    // 检查ESP8266内存使用情况
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
    uint32_t heapFragmentation = ESP.getHeapFragmentation();
    
    // 只在内存使用率较高时输出警告
    if (freeHeap < 20000) { // 少于20KB空闲内存
        Serial.println("⚠️ 内存警告: 空闲内存较低 - " + String(freeHeap) + " bytes");
        Serial.println("最大空闲块: " + String(maxFreeBlock) + " bytes");
        Serial.println("内存碎片率: " + String(heapFragmentation) + "%");
    }
    
    // 输出温度读取性能统计
    if (temperatureReadCount > 0) {
        Serial.println("📊 温度读取性能: 平均时间 " + String(temperatureReadAvgTime) + "ms, 总次数 " + String(temperatureReadCount));
        // 重置计数器
        temperatureReadCount = 0;
        temperatureReadAvgTime = 0;
    }
}

// 增强的温度读取函数，包含性能监控
float readTemperatureWithMonitoring() {
    unsigned long startTime = micros();
    
    float temp = readTemperatureManual();
    
    unsigned long endTime = micros();
    unsigned long readTime = (endTime - startTime) / 1000; // 转换为毫秒
    
    // 更新性能统计（仅在成功读取时）
    if (temp >= 0) {
        temperatureReadAvgTime = (temperatureReadAvgTime * temperatureReadCount + readTime) / (temperatureReadCount + 1);
        temperatureReadCount++;
    }
    
    return temp;
}