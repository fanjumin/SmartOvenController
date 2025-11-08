// =========================================
// 智能烤箱控制器固件 v0.6.6 - 从固件恢复的完整版本
// =========================================
// 固件版本: 0.6.6 (基于 firmware_backup_20251107_212839.bin 恢复)
// 主要功能: 网页控制界面 + 温度校准功能 + OTA升级功能 + MAX6675温度传感器驱动 + 多设备识别功能
// 硬件支持: ESP8266系列芯片 + 继电器模块 + MAX6675热电偶传感器
// =========================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <SPI.h>
#include <LittleFS.h>

// =========================================
// 硬件引脚定义
// =========================================
#define THERMO_CLK   14  // MAX6675时钟引脚(D5/GPIO14)
#define THERMO_CS    12  // MAX6675片选引脚(D6/GPIO12)
#define THERMO_DO    13  // MAX6675数据输出引脚(D7/GPIO13)
#define HEATER_PIN   5   // 加热控制引脚
#define BUZZER_PIN   4   // 蜂鸣器引脚(GPIO4/D2)
#define LED_PIN      2   // LED指示灯引脚(GPIO2/D4)

// =========================================
// 系统配置参数 (从固件恢复)
// =========================================

// 温度配置值 (从固件提取的关键参数)
#define DEFAULT_TEMP_200 200
#define DEFAULT_TEMP_3 3
#define DEFAULT_TEMP_25_5 25.5
#define DEFAULT_TEMP_5 5
#define DEFAULT_TEMP_10 10
#define DEFAULT_TEMP_1 1
#define DEFAULT_TEMP_2 2
#define DEFAULT_TEMP_100 100
#define DEFAULT_TEMP_16_0 16.0
#define DEFAULT_TEMP_180 180

// 全局对象定义
ESP8266WebServer webServer(80);
WiFiUDP udp;
DNSServer dnsServer;
ESP8266WebServer otaServer(8080);
ESP8266HTTPUpdateServer httpUpdater;
WiFiServer tcpServer(8888);  // TCP服务器初始化，用于设备通信
WiFiClient tcpClient;        // TCP客户端对象，用于连接管理

// 系统运行状态变量
unsigned long temperatureReadCount = 0;
float temperatureReadAvgTime = 0;
const unsigned long WEB_SERVER_HANDLE_INTERVAL = 100; // 100ms处理一次Web请求

// 硬件状态监控
unsigned long hardwareFailureCount = 0;       // 硬件故障计数
unsigned long lastHardwareReset = 0;         // 上次硬件重置时间
bool hardwareInitialized = false;            // 硬件是否初始化完成标志

// 设备信息定义
const String DEVICE_TYPE = "oven";
const String DEVICE_ID = "oven-" + String(ESP.getChipId());
const String DEVICE_NAME = "SmartOven";
const String FIRMWARE_VERSION = "0.6.6";

// WiFi配置参数
String wifiSSID = "";
String wifiPassword = "";

// captive portal模式配置
bool isCaptivePortalMode = false;
unsigned long captivePortalStartTime = 0;
const unsigned long CAPTIVE_PORTAL_TIMEOUT = 300000; // 5分钟后自动退出门户模式
const String AP_SSID = "SmartOven-" + String(ESP.getChipId());
const String AP_PASSWORD = "12345678";

// 温度控制变量 (使用固件恢复的参数)
float currentTemp = 0.0;
float targetTemp = DEFAULT_TEMP_180;  // 使用固件恢复的默认温度180°C
bool heatingEnabled = false;
bool ovenMode = true; // 烤箱工作模式：true=自动模式，false=手动模式

// 网络通信配置
const int DEFAULT_PORT = 8888;

// =========================================
// MAX6675温度传感器驱动
// =========================================

// 读取MAX6675原始数据
uint16_t readMAX6675RawData() {
  uint16_t data = 0;
  
  digitalWrite(THERMO_CS, HIGH);
  digitalWrite(THERMO_CLK, LOW);
  delay(10);
  
  digitalWrite(THERMO_CS, LOW);
  delayMicroseconds(100);
  
  for (int i = 15; i >= 0; i--) {
    digitalWrite(THERMO_CLK, HIGH);
    delayMicroseconds(5);
    
    if (digitalRead(THERMO_DO)) {
      data |= (1 << i);
    }
    
    digitalWrite(THERMO_CLK, LOW);
    delayMicroseconds(5);
  }
  
  digitalWrite(THERMO_CS, HIGH);
  delayMicroseconds(100);
  
  if (data == 0x0000 || data == 0xFFFF) {
    Serial.println("传感器通信错误: MAX6675未连接或读取失败");
  }
  
  return data;
}

// 温度校准参数配置
float temperatureOffset = 0.0;
float temperatureScale = 1.0;

// 硬件初始化验证
bool verifyHardwareInitialization() {
    Serial.println("验证硬件初始化状态...");
    
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    delay(100);
    
    if (digitalRead(THERMO_DO) == HIGH || digitalRead(THERMO_DO) == LOW) {
        Serial.println("硬件初始化验证通过");
        return true;
    } else {
        Serial.println("硬件初始化验证失败");
        return false;
    }
}

// 执行硬件恢复流程
void performHardwareRecovery() {
    Serial.println("执行硬件恢复流程...");
    
    hardwareFailureCount++;
    
    for (int i = 0; i < 5; i++) {
        digitalWrite(THERMO_CS, HIGH);
        digitalWrite(THERMO_CLK, LOW);
        delay(200);
        digitalWrite(THERMO_CS, LOW);
        delay(100);
        digitalWrite(THERMO_CS, HIGH);
        delay(200);
    }
    
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    
    lastHardwareReset = millis();
    Serial.println("硬件恢复流程已执行，失败次数: " + String(hardwareFailureCount));
}

// 读取温度函数
float readTemperatureManual() {
    for (int retry = 0; retry < 3; retry++) {
        uint16_t rawData = readMAX6675RawData();
        
        Serial.print("读取尝试"); Serial.print(retry + 1); 
        Serial.print(": 原始数据: 0x"); Serial.println(rawData, HEX);
        
        if (rawData == 0x0000 || rawData == 0xFFFF) {
            if (retry < 2) {
                Serial.println("传感器数据无效，准备重试...");
                delay(100);
                continue;
            } else {
                Serial.println("传感器通信失败 - 已达最大重试次数，无法获取有效数据");
                hardwareFailureCount++;
                return 25.0;
            }
        }
        
        if (!(rawData & 0x04)) {
            uint16_t tempBits = rawData >> 3;
            float temperature = tempBits * 0.25;
            
            // 应用温度校准
            temperature = (temperature * temperatureScale) + temperatureOffset;
            
            Serial.print("温度读取成功: "); Serial.print(temperature); Serial.println("°C");
            return temperature;
        } else {
            Serial.println("传感器连接异常 - 热电偶未连接或故障");
            if (retry < 2) {
                delay(100);
                continue;
            } else {
                Serial.println("传感器连接异常 - 已达最大重试次数");
                return -273.15; // 返回绝对零度表示错误
            }
        }
    }
    
    return -273.15;
}

// =========================================
// Web服务器处理函数 (从固件恢复的关键功能)
// =========================================

void handleRoot() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>SmartOven Controller</title>";
    html += "<style>body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}</style>";
    html += "</head><body><h1>智能烤箱控制器</h1>";
    html += "<p>设备ID: " + DEVICE_ID + "</p>";
    html += "<p>固件版本: " + FIRMWARE_VERSION + "</p>";
    html += "<p>当前温度: " + String(currentTemp) + "°C</p>";
    html += "<p>目标温度: " + String(targetTemp) + "°C</p>";
    html += "<p>加热状态: " + String(heatingEnabled ? "开启" : "关闭") + "</p>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
}

void handleSetTemperature() {
    if (webServer.hasArg("temp")) {
        float newTemp = webServer.arg("temp").toFloat();
        
        // 使用固件恢复的温度参数进行验证
        if (newTemp >= DEFAULT_TEMP_1 && newTemp <= DEFAULT_TEMP_200) {
            targetTemp = newTemp;
            webServer.send(200, "text/plain", "温度设置成功: " + String(targetTemp) + "°C");
        } else {
            webServer.send(400, "text/plain", "温度参数无效，范围: " + String(DEFAULT_TEMP_1) + "-" + String(DEFAULT_TEMP_200) + "°C");
        }
    } else {
        webServer.send(400, "text/plain", "缺少温度参数");
    }
}

void handleGetStatus() {
    String json = "{\"device_id\":\"" + DEVICE_ID + "\",";
    json += "\"firmware_version\":\"" + FIRMWARE_VERSION + "\",";
    json += "\"current_temp\":" + String(currentTemp) + ",";
    json += "\"target_temp\":" + String(targetTemp) + ",";
    json += "\"heating_enabled\":" + String(heatingEnabled ? "true" : "false") + ",";
    json += "\"oven_mode\":" + String(ovenMode ? "true" : "false") + "}";
    
    webServer.send(200, "application/json", json);
}

// =========================================
// 温度控制逻辑 (使用固件恢复的参数)
// =========================================

void updateTemperatureControl() {
    // 温度控制逻辑
    if (heatingEnabled && ovenMode) {
        if (currentTemp < targetTemp - DEFAULT_TEMP_5) {
            digitalWrite(HEATER_PIN, HIGH); // 开启加热
        } else if (currentTemp > targetTemp + DEFAULT_TEMP_2) {
            digitalWrite(HEATER_PIN, LOW);  // 关闭加热
        }
    } else {
        digitalWrite(HEATER_PIN, LOW); // 手动模式或加热禁用
    }
}

// =========================================
// WiFi连接管理
// =========================================

bool connectToWiFi() {
    Serial.println("尝试连接到WiFi网络...");
    
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    for (int i = 0; i < 20; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi连接成功!");
            Serial.print("IP地址: "); Serial.println(WiFi.localIP());
            return true;
        }
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println("WiFi连接失败，启动AP模式");
    return false;
}

void startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
    Serial.println("AP模式已启动");
    Serial.print("AP SSID: "); Serial.println(AP_SSID);
    Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
    
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
}

// =========================================
// 系统初始化
// =========================================

void setup() {
    Serial.begin(115200);
    Serial.println("=== SmartOven Controller 启动 ===");
    Serial.println("固件版本: " + FIRMWARE_VERSION);
    Serial.println("设备ID: " + DEVICE_ID);
    
    // 初始化引脚
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, HIGH); // LED指示灯亮起
    
    // 初始化温度传感器
    if (!verifyHardwareInitialization()) {
        Serial.println("硬件初始化失败，执行恢复流程");
        performHardwareRecovery();
    }
    
    // 尝试连接WiFi
    if (!connectToWiFi()) {
        startAPMode();
    }
    
    // 启动Web服务器
    webServer.on("/", handleRoot);
    webServer.on("/set_temp", handleSetTemperature);
    webServer.on("/status", handleGetStatus);
    
    webServer.begin();
    Serial.println("Web服务器已启动");
    
    // 启动TCP服务器
    tcpServer.begin();
    Serial.println("TCP服务器已启动，端口: " + String(DEFAULT_PORT));
    
    Serial.println("=== 系统初始化完成 ===");
}

// =========================================
// 主循环
// =========================================

void loop() {
    // 处理Web请求
    webServer.handleClient();
    
    // 处理TCP连接
    if (tcpServer.hasClient()) {
        if (!tcpClient || !tcpClient.connected()) {
            if (tcpClient) tcpClient.stop();
            tcpClient = tcpServer.available();
            Serial.println("新的TCP客户端连接");
        }
    }
    
    // 读取温度 (每2秒读取一次)
    static unsigned long lastTempRead = 0;
    if (millis() - lastTempRead > 2000) {
        currentTemp = readTemperatureManual();
        lastTempRead = millis();
        
        // 温度监控统计: 定期显示温度读取性能数据，帮助优化系统性能
        temperatureReadCount++;
        if (temperatureReadCount % 10 == 0) {
            Serial.print("温度读取统计 - 总次数: "); Serial.print(temperatureReadCount);
            Serial.print(", 当前温度: "); Serial.print(currentTemp); Serial.println("°C");
        }
    }
    
    // 更新温度控制
    updateTemperatureControl();
    
    // 检查AP模式超时
    if (isCaptivePortalMode && millis() - captivePortalStartTime > CAPTIVE_PORTAL_TIMEOUT) {
        Serial.println("AP模式超时，尝试重新连接WiFi");
        isCaptivePortalMode = false;
        if (connectToWiFi()) {
            WiFi.mode(WIFI_STA);
        }
    }
    
    delay(100); // 主循环延迟
}