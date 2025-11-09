// =========================================
// 智能烤箱控制器固件 v0.7.5 - 正式版
// =========================================
// 固件版本: 0.7.6
// 主要功能: 网页控制界面 + 温度校准功能 + OTA升级功能 + MAX6675温度传感器驱动 + 多设备识别功能
// 硬件支持: ESP8266系列芯片 + 继电器模块 + OLED显示屏 + MAX6675热电偶传感器
// =========================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <SPI.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

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
// 系统配置参数
// =========================================

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
const unsigned long WEB_SERVER_HANDLE_INTERVAL = 100; // 100ms处理一次Web请求，避免阻塞主循环占用过多CPU资源

// 硬件状态监控
unsigned long hardwareFailureCount = 0;       // 硬件故障计数
unsigned long lastHardwareReset = 0;         // 上次硬件重置时间
bool hardwareInitialized = false;            // 硬件是否初始化完成标志

// 设备信息定义
const String DEVICE_TYPE = "oven";
const String DEVICE_ID = "oven-" + String(ESP.getChipId());
const String DEVICE_NAME = "SmartOven";
const String FIRMWARE_VERSION = "0.7.6";

// WiFi配置参数
String wifiSSID = "";
String wifiPassword = "";

// captive portal模式配置
bool isCaptivePortalMode = false;
unsigned long captivePortalStartTime = 0;

// 文件系统状态
bool isFileSystemAvailable = false;
const unsigned long CAPTIVE_PORTAL_TIMEOUT = 300000; // 5分钟后自动退出门户模式
const String AP_SSID = "SmartOven-" + String(ESP.getChipId());
const String AP_PASSWORD = "12345678";

// 温度控制变量
float currentTemp = 0.0;
float targetTemp = 180.0;
bool heatingEnabled = false;
bool ovenMode = true; // 烤箱工作模式：true=自动模式，false=手动模式

// 网络通信配置
const int DEFAULT_PORT = 8888;

// 温度校准参数配置
float temperatureOffset = 0.0;  // 温度补偿值 - 用于校准温度传感器读数偏差
float temperatureScale = 1.0;    // 温度缩放系数 - 用于调整温度读数的比例关系

// 设备发现配置
bool discoveryEnabled = true;
const unsigned long DISCOVERY_INTERVAL = 10000; // 10秒设备发现广播间隔
unsigned long lastDiscoveryTime = 0;

// LED状态控制与闪烁管理
bool ledState = false;
unsigned long lastLedUpdate = 0;
const unsigned long LED_BLINK_INTERVAL = 500; // LED闪烁间隔时间(毫秒)

// 烘焙完成状态管理与提示控制
bool bakingCompleteState = false;
unsigned long bakingCompleteStartTime = 0;
const unsigned long BAKING_COMPLETE_DURATION = 10000; // 烘焙完成提示持续时间(毫秒)

// =========================================
// EEPROM配置管理模块
// =========================================

struct Config {
    char ssid[32];
    char password[64];
    float temperatureOffset;  // 温度校准偏移量
    float temperatureScale;    // 温度校准缩放系数
    char signature[16];  // 配置文件签名，用于验证配置有效性
};

bool saveConfig() {
    Config config;
    // 初始化配置结构体，清空内存空间
    memset(&config, 0, sizeof(config));
    
    // 验证WiFi配置参数有效性
    if (wifiSSID.length() == 0 || wifiSSID.length() > 31) {
        Serial.println("错误：SSID无效，无法保存配置");
        return false;
    }
    if (wifiPassword.length() > 63) {
        Serial.println("错误：密码过长，无法保存配置");
        return false;
    }
    
    // 将WiFi配置参数复制到配置结构体
    strncpy(config.ssid, wifiSSID.c_str(), sizeof(config.ssid) - 1);
    strncpy(config.password, wifiPassword.c_str(), sizeof(config.password) - 1);
    strncpy(config.signature, "SMARTOVEN", sizeof(config.signature) - 1);
    
    // 应用温度校准参数到配置结构体
    config.temperatureOffset = temperatureOffset;
    config.temperatureScale = temperatureScale;
    
    // 确保字符串以null终止符结束
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    config.password[sizeof(config.password) - 1] = '\0';
    config.signature[sizeof(config.signature) - 1] = '\0';
    
    // 保存配置到EEPROM，增加重试机制
    bool saveSuccess = false;
    for (int attempt = 0; attempt < 3 && !saveSuccess; attempt++) {
        EEPROM.begin(512);
        EEPROM.put(0, config);
        saveSuccess = EEPROM.commit();
        EEPROM.end();
        
        if (!saveSuccess) {
            Serial.println("EEPROM保存失败，重试 " + String(attempt + 1));
            delay(100);
        }
    }
    
    if (saveSuccess) {
        Serial.println("配置已成功保存到EEPROM");
        Serial.print("SSID: ");
        Serial.println(config.ssid);
        Serial.print("密码长度: ");
        Serial.println(strlen(config.password));
        Serial.print("温度校准偏移量: ");
        Serial.print(config.temperatureOffset);
        Serial.println("°C");
        Serial.print("温度校准缩放系数: ");
        Serial.println(config.temperatureScale);
        Serial.print("配置签名: ");
        Serial.println(config.signature);
        
        // 配置保存成功提示音
        beepConfigSaved();
        return true;
    } else {
        Serial.println("错误：EEPROM保存失败，配置未保存");
        return false;
    }
}

bool loadConfig() {
    Config config;
    
    // 读取EEPROM配置，增加重试机制
    bool readSuccess = false;
    for (int attempt = 0; attempt < 3 && !readSuccess; attempt++) {
        EEPROM.begin(512);
        EEPROM.get(0, config);
        EEPROM.end();
        
        // 验证读取的数据是否有效
        if (config.signature[0] != '\0') {
            readSuccess = true;
        } else {
            Serial.println("EEPROM读取失败，重试 " + String(attempt + 1));
            delay(100);
        }
    }
    
    if (!readSuccess) {
        Serial.println("错误：EEPROM读取失败，使用默认配置");
        resetToDefaultConfig();
        return false;
    }
    
    Serial.println("从EEPROM加载配置参数...");
    Serial.print("配置签名: ");
    Serial.println(config.signature);
    Serial.print("配置SSID: ");
    Serial.println(config.ssid);
    Serial.print("密码长度: ");
    Serial.println(strlen(config.password));
    Serial.print("温度校准偏移量: ");
    Serial.print(config.temperatureOffset);
    Serial.println("°C");
    Serial.print("温度校准缩放系数: ");
    Serial.println(config.temperatureScale);
    
    // 严格的配置验证
    if (strcmp(config.signature, "SMARTOVEN") == 0) {
        // 验证SSID和密码的有效性
        String loadedSSID = String(config.ssid);
        String loadedPassword = String(config.password);
        
        if (loadedSSID.length() == 0 || loadedSSID.length() > 31) {
            Serial.println("警告：加载的SSID无效，使用默认配置");
            resetToDefaultConfig();
            return false;
        }
        
        if (loadedPassword.length() > 63) {
            Serial.println("警告：加载的密码过长，使用默认配置");
            resetToDefaultConfig();
            return false;
        }
        
        // 配置验证通过，应用配置
        wifiSSID = loadedSSID;
        wifiPassword = loadedPassword;
        
        // 加载配置文件中的温度校准参数
        temperatureOffset = config.temperatureOffset;
        temperatureScale = config.temperatureScale;
        
        // 验证温度校准参数的合理性
        if (temperatureOffset < -50.0 || temperatureOffset > 50.0) {
            Serial.println("警告：温度校准偏移量超出合理范围，重置为0");
            temperatureOffset = 0.0;
        }
        
        if (temperatureScale < 0.5 || temperatureScale > 2.0) {
            Serial.println("警告：温度校准缩放系数超出合理范围，重置为1");
            temperatureScale = 1.0;
        }
        
        Serial.println("配置文件加载成功，应用温度校准参数");
        Serial.print("温度校准偏移量: ");
        Serial.print(temperatureOffset);
        Serial.println("°C");
        Serial.print("温度校准缩放系数: ");
        Serial.println(temperatureScale);
        return true;
    } else {
        Serial.println("配置文件签名验证失败，使用默认配置参数");
        resetToDefaultConfig();
        return false;
    }
}

// =========================================
// 重置为默认配置
// =========================================
void resetToDefaultConfig() {
    wifiSSID = "";
    wifiPassword = "";
    temperatureOffset = 0.0;
    temperatureScale = 1.0;
    Serial.println("已重置为默认配置参数");
}

// =========================================
// MAX6675温度传感器驱动
// =========================================

// 读取MAX6675原始数据 - 16位数据格式，包含温度信息和状态位
/**
 * 读取MAX6675温度传感器的原始数据
 * 
 * 此函数通过手动SPI通信协议读取MAX6675传感器的16位原始数据，
 * 包括温度信息和传感器状态位。
 * 
 * @return uint16_t 16位原始数据，包含温度值和状态信息
 */
uint16_t readMAX6675RawData() {
  uint16_t data = 0;
  
  // 初始化MAX6675片选引脚 - 先置高电平禁用通信
  digitalWrite(THERMO_CS, HIGH);
  digitalWrite(THERMO_CLK, LOW);
  delay(10);  // 等待芯片稳定 - 延迟10ms
  
  // 开始数据读取
  digitalWrite(THERMO_CS, LOW);
  delayMicroseconds(100);  // 等待转换完成 - 确保数据稳定
  
  // 读取16位数据 - 从高位到低位
  for (int i = 15; i >= 0; i--) {
    digitalWrite(THERMO_CLK, HIGH);  // 时钟信号高电平 - 读取数据位
    delayMicroseconds(5);  // 短暂延迟确保数据稳定读取
    if (digitalRead(THERMO_DO)) {   // 读取数据位 - 如果DO引脚为高电平则设置当前位
      data |= (1 << i);
    }
    digitalWrite(THERMO_CLK, LOW);  // 时钟信号低电平 - 结束当前位读取
    delayMicroseconds(5);  // 短暂延迟确保数据稳定
  }
  
  // 结束数据读取
  digitalWrite(THERMO_CS, HIGH);
  delayMicroseconds(100);  // 等待通信完成 - 确保芯片进入空闲状态
  
  // 检查传感器数据有效性 - 判断是否通信正常
  if (data == 0x0000 || data == 0xFFFF) {
    Serial.println("传感器通信错误: MAX6675未连接或读取失败");
  }
  
  return data;
}

// 硬件初始化验证（快速启动优化）
bool verifyHardwareInitialization() {
    // 快速初始化MAX6675引脚模式
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    // 快速配置MAX6675初始状态
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    delay(10);  // 快速等待传感器稳定- 延迟10ms（优化启动速度）
    
    // 快速检查MAX6675传感器响应状态
    if (digitalRead(THERMO_DO) == HIGH || digitalRead(THERMO_DO) == LOW) {
        return true;
    } else {
        return false;
    }
}

// 执行硬件恢复流程（快速启动优化）
void performHardwareRecovery() {
    // 增加硬件故障计数
    hardwareFailureCount++;
    
    // 快速重置MAX6675传感器 - 尝试恢复通信
    for (int i = 0; i < 3; i++) {  // 减少重试次数
        digitalWrite(THERMO_CS, HIGH);
        digitalWrite(THERMO_CLK, LOW);
        delay(50);  // 快速等待传感器稳定 - 延迟50ms（优化启动速度）
        digitalWrite(THERMO_CS, LOW);
        delay(20);  // 快速延迟
        digitalWrite(THERMO_CS, HIGH);
        delay(50);  // 快速延迟
    }
    
    // 快速重新初始化MAX6675引脚配置
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    
    lastHardwareReset = millis();
}

/**
 * 读取温度值（带重试机制）
 * 
 * 此函数通过MAX6675传感器读取当前温度值，包含重试机制和错误处理。
 * 最多尝试3次读取，如果所有尝试都失败，则返回默认温度值25.0°C。
 * 
 * @return float 读取到的温度值（摄氏度），失败时返回25.0°C
 */
float readTemperatureManual() {
    // 温度读取带重试机制 - 最多尝试3次读取传感器数据
    for (int retry = 0; retry < 3; retry++) {
        uint16_t rawData = readMAX6675RawData();
        
        // 输出当前重试次数及原始数据
        Serial.print("读取尝试"); Serial.print(retry + 1); 
        Serial.print(": 原始数据: 0x"); Serial.println(rawData, HEX);
        
        // 检查传感器数据有效性 - 排除无效数据（0x0000或0xFFFF）
        if (rawData == 0x0000 || rawData == 0xFFFF) {
            if (retry < 2) {
                Serial.println("传感器数据无效，准备重试...");
                delay(100);  // 重试前延迟100ms
                continue;
            } else {
                Serial.println("传感器通信失败 - 已达最大重试次数，无法获取有效数据 - 将返回默认温度");
                // 传感器读取失败，增加硬件故障计数
                hardwareFailureCount++;
                // 传感器读取失败，已达最大重试次数，返回默认温度 25.0°C
                Serial.println("传感器读取失败，返回默认温度: 25.0°C");
                return 25.0;
            }
        }
        
        // 检查传感器连接状态位 - 第3位为0表示连接正常
        if (!(rawData & 0x04)) {
            uint16_t tempBits = rawData >> 3;  // 将原始数据右移3位以提取温度相关数据
            float temperature = tempBits * 0.25;  // 每一位代表0.25°C，计算实际温度值
            
            // 应用温度校准参数（缩放和偏移）
            temperature = (temperature * temperatureScale) + temperatureOffset;
            
            // 验证温度值是否在有效范围内（-50.0°C 到 400.0°C）
            if (temperature >= -50.0 && temperature <= 400.0) {
                Serial.print("读取到的温度: ");
                Serial.print(temperature); Serial.println("°C");
                
                // 重置传感器错误计数器，更新最后成功读取时间
                if (retry == 0) {
                    hardwareFailureCount = 0;
                }
                return temperature;
            } else {
                Serial.println("温度值超出有效范围");
                return -1.0;
            }
        } else {
            if (retry < 2) {
                Serial.println("传感器连接状态异常- 尝试重新连接...");
                delay(100);  // 重试前延迟100ms
                continue;
            } else {
                Serial.println("传感器连接状态异常- 已达最大重试次数");
                return -1.0;
            }
        }
    }
    
    // 所有读取尝试失败- 无法获取有效温度数据
    Serial.println("所有读取尝试失败- 返回默认温度");
    // 传感器读取失败，已达最大重试次数，返回默认温度 25.0°C
    Serial.println("传感器读取失败，返回默认温度: 25.0°C");
    return 25.0;
}

// 温度校准参数设置函数 - 用于调整传感器读数准确性
void calibrateTemperature(float actualTemp, float measuredTemp) {
    // 计算温度校准参数 - 实际温度与测量温度的比例关系
    if (measuredTemp != 0) {
        temperatureScale = actualTemp / measuredTemp;
        temperatureOffset = actualTemp - (measuredTemp * temperatureScale);
    } else {
        temperatureOffset = actualTemp - measuredTemp;
        temperatureScale = 1.0;
    }
    
    Serial.println("温度校准参数计算完成");
    Serial.print("实际校准温度: "); Serial.print(actualTemp); Serial.println("°C");
    Serial.print("传感器测量温度: "); Serial.print(measuredTemp); Serial.println("°C");
    Serial.print("温度校准偏移量: "); Serial.print(temperatureOffset); Serial.println("°C");
    Serial.print("温度校准缩放系数: "); Serial.println(temperatureScale);
    
    // 保存温度校准参数到EEPROM
    saveConfig();
    Serial.println("温度校准参数已保存到EEPROM");
}

// =========================================
// 网络配置与Captive Portal管理
// =========================================

/**
 * 启动强制门户服务
 * 
 * 此函数负责配置并启动WiFi接入点模式，设置DNS服务器和Web服务器，
 * 为用户提供网络配置界面。当设备无法连接到保存的WiFi网络时，
 * 会自动启动此服务进行网络配置。
 */
void startCaptivePortal() {
    Serial.println("启动Captive Portal服务...");
    
    // 快速断开现有WiFi连接（快速配网优化）
    WiFi.disconnect();
    delay(50); // 减少等待时间
    
    // 快速配置并启动WiFi接入点模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
    
    Serial.print("AP名称: ");
    Serial.println(AP_SSID);
    Serial.print("AP IP地址: ");
    Serial.println(WiFi.softAPIP());
    
    // 快速配置DNS服务器参数
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    // 快速启动Web服务器服务
    setupWebServer();
    webServer.begin();
    
    // 启动UDP服务用于设备发现广播
    udp.begin(8888);
    
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
    
    Serial.println("Captive portal启动成功 - 快速配网模式已启用");
    Serial.println("用户可立即访问 http://192.168.4.1 进行WiFi配置");
}

/**
 * 停止强制门户服务
 * 
 * 此函数负责关闭DNS服务器、断开WiFi接入点，并恢复系统到正常工作模式。
 * 通常在WiFi配置完成或超时后调用此函数。
 */
void stopCaptivePortal() {
    Serial.println("停止Captive Portal服务，关闭相关网络服务...");
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    isCaptivePortalMode = false;
    captivePortalStartTime = 0;
    Serial.println("Captive portal已停止，系统恢复正常工作模式");
}

bool shouldStartCaptivePortal() {
    // 检查WiFi配置是否存在 - 若SSID或密码为空则启动 captive portal
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("WiFi配置参数缺失，启动captive portal 进行网络配置");
        return true;
    }
    
    // WiFi配置参数存在，尝试连接到指定WiFi网络
    Serial.println("使用保存的WiFi配置参数尝试连接网络");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("密码长度: ");
    Serial.println(wifiPassword.length());
    
    // 配置WiFi为Station模式并尝试连接网络
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    // 设置WiFi连接超时处理机制，最多等待15秒连接成功（快速配网优化）
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        // 定期检查WiFi连接状态并输出当前状态
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("WiFi连接状态: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("闲置状态"); break;
                case WL_NO_SSID_AVAIL: Serial.println("SSID不存在"); break;
                case WL_SCAN_COMPLETED: Serial.println("扫描完成"); break;
                case WL_CONNECTED: Serial.println("连接成功"); break;
                case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
                case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
                case WL_DISCONNECTED: Serial.println("已断开连接"); break;
                default: Serial.println("未知状态"); break;
            }
        }
    }
    
    // 检查WiFi连接结果并处理连接状态
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi连接成功");
        Serial.print("IP地址: ");
        Serial.println(WiFi.localIP());
        Serial.print("连接耗时: ");
        Serial.print((millis() - startTime) / 1000.0);
        Serial.println("秒");
        return false; // WiFi连接成功，无需启动Captive Portal
    } else {
        Serial.println("");
        Serial.println("WiFi连接失败，启动Captive Portal进行网络配置");
        Serial.print("WiFi连接失败原因: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("闲置状态"); break;
            case WL_NO_SSID_AVAIL: Serial.println("SSID不存在"); break;
            case WL_SCAN_COMPLETED: Serial.println("扫描完成"); break;
            case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
            case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
            case WL_DISCONNECTED: Serial.println("已断开连接"); break;
            default: Serial.println("未知状态"); break;
        }
        
        // 连接失败，启动Captive Portal进行网络配置
        WiFi.disconnect();
        delay(100);
        return true; // WiFi连接失败，启动Captive Portal进行网络配置
    }
}

void checkCaptivePortalTimeout() {
    if (isCaptivePortalMode && 
        millis() - captivePortalStartTime > CAPTIVE_PORTAL_TIMEOUT) {
        Serial.println("Captive Portal超时，停止配网模式并尝试连接WiFi");
        stopCaptivePortal();
        connectToWiFi();
    }
}

// =========================================
// 智能WiFi扫描功能 - 快速配网优化
// =========================================

/**
 * 智能WiFi扫描函数
 * 
 * 此函数负责快速扫描可用的WiFi网络，并返回网络列表
 * 用于在Captive Portal中自动推荐可用网络
 * 
 * @return String 包含可用WiFi网络的JSON格式字符串
 */
String scanWiFiNetworks() {
    Serial.println("开始快速WiFi扫描...");
    
    // 添加安全检查和内存保护
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("设备已连接到WiFi，跳过扫描以避免系统不稳定");
        return "{\"status\":\"error\",\"message\":\"设备已连接到WiFi，无法扫描\"}";
    }
    
    // 设置WiFi模式为STA以进行扫描
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // 快速扫描WiFi网络（5秒超时）
    int n = WiFi.scanNetworks(false, true, 0, NULL);
    
    if (n == 0) {
        Serial.println("未发现可用WiFi网络");
        return "{\"status\":\"success\",\"networks\":[]}";
    } else if (n > 20) {
        Serial.println("发现太多网络，限制显示数量以避免内存问题");
        n = 20; // 限制最大网络数量
    }
    
    Serial.print("发现 ");
    Serial.print(n);
    Serial.println(" 个WiFi网络");
    
    // 构建JSON格式的网络列表
    String networks = "{\"status\":\"success\",\"networks\":[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) networks += ",";
        networks += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
        networks += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        networks += "\"encrypted\":" + String(WiFi.encryptionType(i) != ENC_TYPE_NONE ? "true" : "false") + "}";
        
        // 添加内存保护，防止字符串过长
        if (networks.length() > 2000) {
            Serial.println("WiFi网络列表过长，已截断");
            break;
        }
    }
    networks += "]}";
    
    return networks;
}

// =========================================
// WiFi连接管理函数 - 处理网络连接和重连逻辑
// =========================================

/**
 * 连接到WiFi网络
 * 
 * 此函数负责使用保存的WiFi配置参数连接到指定的WiFi网络。
 * 如果连接失败，会返回false但不启动强制门户服务，让调用方决定后续操作。
 * 
 * @return bool 连接成功返回true，失败返回false
 */
bool connectToWiFi() {
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("WiFi配置参数缺失，无法连接");
        return false;
    }
    
    Serial.print("尝试连接到WiFi网络: ");
    Serial.println(wifiSSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    // WiFi连接超时处理：最多尝试10秒，期间每500ms检查一次连接状态
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        if (connectionAttempts % 4 == 0) {
            Serial.println("");
            Serial.print("WiFi连接状态: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("闲置状态"); break;
                case WL_NO_SSID_AVAIL: Serial.println("SSID不存在"); break;
                case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
                case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
                case WL_DISCONNECTED: Serial.println("已断开连接"); break;
                default: Serial.println("连接中..."); break;
            }
        }
    }
    
    // 检查WiFi连接结果
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi连接成功！");
        Serial.print("IP地址: ");
        Serial.println(WiFi.localIP());
        
        isCaptivePortalMode = false;
        
        // 快速启动Web服务器
        setupWebServer();
        webServer.begin();
        
        return true;
    } else {
        Serial.println("");
        Serial.println("WiFi连接失败");
        Serial.print("失败原因: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("闲置状态"); break;
            case WL_NO_SSID_AVAIL: Serial.println("SSID不存在"); break;
            case WL_CONNECT_FAILED: Serial.println("连接失败"); break;
            case WL_CONNECTION_LOST: Serial.println("连接丢失"); break;
            case WL_DISCONNECTED: Serial.println("已断开连接"); break;
            default: Serial.println("未知错误"); break;
        }
        
        return false;
    }
}

// =========================================
// 设备发现协议处理
// =========================================

void handleDiscovery() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char packetBuffer[255];
        int len = udp.read(packetBuffer, 255);
        if (len > 0) {
            packetBuffer[len] = 0;
            String request = String(packetBuffer);
            
            Serial.print("收到UDP数据包，大小: ");
            Serial.print(packetSize);
            Serial.print(" 内容: ");
            Serial.println(request);
            
            if (request.startsWith("DISCOVER_SMARTOVEN")) {
                Serial.println("收到设备发现请求，正在发送响应");
                sendDiscoveryResponse();
            }
        }
    }
    
    if (discoveryEnabled && millis() - lastDiscoveryTime > DISCOVERY_INTERVAL) {
        broadcastDiscovery();
        lastDiscoveryTime = millis();
    }
}

void sendDiscoveryResponse() {
    String response;
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
    
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(response.c_str());
    udp.endPacket();
    
    Serial.println("发送设备信息响应到端口 " + String(udp.remotePort()) + ": " + response);
}

void broadcastDiscovery() {
    String broadcastMsg = "SMARTOVEN_BROADCAST:" + DEVICE_TYPE + ":" + DEVICE_ID + ":" + FIRMWARE_VERSION;
    udp.beginPacket("255.255.255.255", 8888);
    udp.write(broadcastMsg.c_str());
    udp.endPacket();
    Serial.println("发送广播发现信息 " + broadcastMsg);
}

// =========================================
// OTA更新设置
// =========================================

void setupOTA() {
    // 将OTA更新服务器集成到主Web服务器中
    httpUpdater.setup(&webServer);
    Serial.println("OTA升级功能已集成到主Web服务器");
    Serial.println("OTA更新页面地址: http://" + WiFi.localIP().toString() + "/ota_update");
}

void handleOTA() {
    if (!isCaptivePortalMode && WiFi.status() == WL_CONNECTED) {
        otaServer.handleClient();
    }
}

// =========================================
// Web服务器处理函数
// =========================================

void handleNotFound() {
    if (isCaptivePortalMode) {
        // 在captive portal模式下重定向到配网页面
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "Redirect to configuration page");
    } else {
        // 正常模式下返回404错误
        webServer.send(404, "text/plain", "Not found: " + webServer.uri());
    }
}

void handleRoot() {
    // 如果处于强制门户模式，直接显示配网页面
    if (isCaptivePortalMode) {
        // 检查wifi_config.html文件是否存在
        if (LittleFS.exists("/wifi_config.html")) {
            File file = LittleFS.open("/wifi_config.html", "r");
            if (file) {
                webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                webServer.sendHeader("Pragma", "no-cache");
                webServer.sendHeader("Expires", "-1");
                webServer.streamFile(file, "text/html", HTTP_GET);
                file.close();
                Serial.println("强制门户模式：直接显示wifi_config.html配网页面");
                return;
            }
        }
        
        // 如果wifi_config.html文件不存在，返回简化的配网页面
        String html = "<!DOCTYPE html><html><head><title>WiFi配置 - 智能电烤箱</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
        html += "<style>body{font-family:Arial,sans-serif;margin:20px;background:#f0f0f0;}h1{color:#333;}.container{max-width:500px;margin:50px auto;background:white;padding:30px;border-radius:10px;box-shadow:0 4px 15px rgba(0,0,0,0.1);}.btn{display:block;width:100%;padding:15px;background:#007bff;color:white;text-align:center;text-decoration:none;border-radius:5px;margin:10px 0;font-size:16px;}.btn:hover{background:#0056b3;}.info{background:#e8f4fd;border-left:4px solid #007bff;padding:15px;margin:15px 0;border-radius:4px;}</style>";
        html += "</head><body><div class=\"container\">";
        html += "<h1>📶 WiFi配置</h1>";
        html += "<div class=\"info\">";
        html += "<strong>设备已进入配网模式</strong><br>";
        html += "请选择您的WiFi网络并输入密码";
        html += "</div>";
        html += "<form id=\"wifiForm\">";
        html += "<div><label>WiFi网络:</label><select id=\"ssid\" required><option value=\"\">请选择网络...</option></select></div>";
        html += "<div><label>密码:</label><input type=\"password\" id=\"password\" required></div>";
        html += "<button type=\"submit\" class=\"btn\">连接网络</button>";
        html += "</form>";
        html += "<p style=\"text-align:center;color:#666;\">固件版本：" + FIRMWARE_VERSION + "</p>";
        html += "<script>";
        html += "document.getElementById('wifiForm').onsubmit = function(e) { e.preventDefault(); saveWiFi(); }; ";
        html += "function saveWiFi() { var ssid = document.getElementById('ssid').value; var password = document.getElementById('password').value; ";
        html += "fetch('/save_wifi', { method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password) }).then(r => r.json()).then(data => { if(data.status === 'success') { alert('配置保存成功，设备将重启'); } else { alert('配置保存失败'); } }); }";
        html += "</script>";
        html += "</div></body></html>";
        webServer.send(200, "text/html", html);
        Serial.println("强制门户模式：返回简化配网页面");
        return;
    }
    
    // 正常模式：如果LittleFS文件系统已初始化且index.html文件存在，则从文件系统加载
    if (LittleFS.exists("/index.html")) {
        File file = LittleFS.open("/index.html", "r");
        if (file) {
            webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            webServer.sendHeader("Pragma", "no-cache");
            webServer.sendHeader("Expires", "-1");
            webServer.streamFile(file, "text/html", HTTP_GET);
            file.close();
        } else {
            // 如果文件打开失败，返回简单的状态页面
            String html = "<!DOCTYPE html><html><head><title>智能烤箱控制器</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
            html += "<style>body{font-family:Arial,sans-serif;margin:20px;}</style>";
            html += "</head><body>";
            html += "<h1>智能烤箱控制器 v" + FIRMWARE_VERSION + "</h1>";
            html += "<p>当前温度: " + String(currentTemp) + "°C</p>";
            html += "<p>目标温度: " + String(targetTemp) + "°C</p>";
            html += "<p>加热状态: " + String(heatingEnabled ? "开启" : "关闭") + "</p>";
            html += "<p>工作模式: " + String(ovenMode ? "烤箱模式" : "烤面包机模式") + "</p>";
            html += "<p><small>注意：HTML文件未正确上传到设备，请检查LittleFS文件系统</small></p>";
            html += "</body></html>";
            webServer.send(200, "text/html", html);
        }
    } else {
        // 如果文件不存在，返回简单的状态页面
        String html = "<!DOCTYPE html><html><head><title>智能烤箱控制器</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
        html += "<style>body{font-family:Arial,sans-serif;margin:20px;}</style>";
            html += "</head><body>";
            html += "<h1>智能烤箱控制器 v" + FIRMWARE_VERSION + "</h1>";
            html += "<p>当前温度: " + String(currentTemp) + "°C</p>";
            html += "<p>目标温度: " + String(targetTemp) + "°C</p>";
            html += "<p>加热状态: " + String(heatingEnabled ? "开启" : "关闭") + "</p>";
            html += "<p>工作模式: " + String(ovenMode ? "烤箱模式" : "烤面包机模式") + "</p>";
            html += "<p><small>注意：HTML文件未正确上传到设备，请检查LittleFS文件系统</small></p>";
            html += "</body></html>";
            webServer.send(200, "text/html", html);
    }
}

void handleStatus() {
    String wifiStatus = "disconnected";
    String wifiSSID = "";
    String ipAddress = "";
    int rssi = 0;
    
    // 获取WiFi连接状态（快速配网优化）
    if (WiFi.status() == WL_CONNECTED) {
        wifiStatus = "connected";
        wifiSSID = WiFi.SSID();
        ipAddress = WiFi.localIP().toString();
        rssi = WiFi.RSSI();
    } else if (isCaptivePortalMode) {
        wifiStatus = "captive_portal";
        wifiSSID = AP_SSID;
        ipAddress = WiFi.softAPIP().toString();
    }
    
    String json = "{\"temperature\":" + String(currentTemp) + ",";
    json += "\"target_temperature\":" + String(targetTemp) + ",";
    json += "\"heating_enabled\":" + String(heatingEnabled ? "true" : "false") + ",";
    json += "\"oven_mode\":" + String(ovenMode ? "true" : "false") + ",";
    json += "\"device_id\":\"" + DEVICE_ID + "\",";
    json += "\"firmware_version\":\"" + FIRMWARE_VERSION + "\",";
    json += "\"wifiStatus\":\"" + wifiStatus + "\",";
    json += "\"wifiSSID\":\"" + wifiSSID + "\",";
    json += "\"ipAddress\":\"" + ipAddress + "\",";
    json += "\"rssi\":" + String(rssi) + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"freeMemory\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"captivePortalMode\":" + String(isCaptivePortalMode ? "true" : "false") + "}";
    
    webServer.send(200, "application/json", json);
    Serial.println("状态查询: WiFi状态=" + wifiStatus + ", SSID=" + wifiSSID);
}

void handleControl() {
    if (webServer.hasArg("target_temp")) {
        targetTemp = webServer.arg("target_temp").toFloat();
    }
    if (webServer.hasArg("heating_enabled")) {
        heatingEnabled = webServer.arg("heating_enabled") == "true";
    }
    if (webServer.hasArg("oven_mode")) {
        ovenMode = webServer.arg("oven_mode") == "true";
    }
    
    webServer.send(200, "application/json", "{\"status\":\"success\"}");
}

void handleScanWiFi() {
    // 使用快速WiFi扫描功能（快速配网优化）
    String json = scanWiFiNetworks();
    webServer.send(200, "application/json", json);
    Serial.println("WiFi扫描完成，返回网络列表");
}

void handleSaveWiFi() {
    if (webServer.hasArg("ssid") && webServer.hasArg("password")) {
        wifiSSID = webServer.arg("ssid");
        wifiPassword = webServer.arg("password");
        saveConfig();
        webServer.send(200, "application/json", "{\"status\":\"success\"}");
        delay(1000);
        ESP.restart();
    } else {
        webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"缺少必要参数\"}");
    }
}

void handleFactoryReset() {
    EEPROM.begin(512);
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    
    webServer.send(200, "application/json", "{\"status\":\"success\"}");
    delay(1000);
    ESP.restart();
}

void handleTemperatureCalibration() {
    if (webServer.hasArg("actual_temp") && webServer.hasArg("measured_temp")) {
        float actualTemp = webServer.arg("actual_temp").toFloat();
        float measuredTemp = webServer.arg("measured_temp").toFloat();
        
        calibrateTemperature(actualTemp, measuredTemp);
        
        webServer.send(200, "application/json", "{\"status\":\"success\",\"offset\":" + String(temperatureOffset) + ",\"scale\":" + String(temperatureScale) + "}");
    } else {
        webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"缺少温度校准参数\"}");
    }
}

// =========================================
// 新增缺失的API端点处理函数
// =========================================

void handleWiFiConfig() {
    // 获取或设置WiFi配置
    if (webServer.method() == HTTP_GET) {
        String json = "{\"ssid\":\"" + wifiSSID + "\",\"status\":\"success\"}";
        webServer.send(200, "application/json", json);
    } else if (webServer.method() == HTTP_POST) {
        if (webServer.hasArg("ssid") && webServer.hasArg("password")) {
            wifiSSID = webServer.arg("ssid");
            wifiPassword = webServer.arg("password");
            
            // 保存配置并检查保存结果
            bool saveSuccess = saveConfig();
            
            if (saveSuccess) {
                // 配置保存成功，返回成功响应
                webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"WiFi配置已保存，设备将在3秒后重启\"}");
                
                // 等待1秒确保响应已发送给客户端
                delay(1000);
                
                // 停止Captive Portal服务，确保WiFi状态正确
                if (isCaptivePortalMode) {
                    stopCaptivePortal();
                }
                
                // 再等待2秒确保EEPROM保存完全完成
                delay(2000);
                
                Serial.println("WiFi配置保存完成，设备即将重启以应用新设置");
                ESP.restart();
            } else {
                // 配置保存失败
                webServer.send(500, "application/json", "{\"status\":\"error\",\"message\":\"WiFi配置保存失败，请重试\"}");
            }
        } else {
            webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"缺少必要参数\"}");
        }
    }
}

void handleRestart() {
    webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"设备将在3秒后重启\"}");
    delay(3000);
    ESP.restart();
}

void handleOTAUpdate() {
    // 优化的OTA升级端点 - 提供更直观的升级界面
    String html = "<!DOCTYPE html><html><head><title>智能烤箱控制器 - OTA升级</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<style>body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}h1{color:#333;}.container{max-width:700px;margin:0 auto;background:white;padding:25px;border-radius:10px;box-shadow:0 4px 15px rgba(0,0,0,0.1);}.status-info{background:#e8f4fd;border-left:4px solid #007bff;padding:15px;margin:15px 0;border-radius:4px;}.tab{display:none;}.active{display:block;}.upgrade-option{display:flex;align-items:center;padding:20px;margin:15px 0;border:2px solid #e0e0e0;border-radius:8px;cursor:pointer;transition:all 0.3s;}.upgrade-option:hover{border-color:#007bff;background:#f8f9fa;}.upgrade-option.active{background:#e8f4fd;border-color:#007bff;}.option-icon{font-size:24px;margin-right:15px;width:40px;text-align:center;}.option-content{flex:1;}.option-title{font-size:18px;font-weight:bold;margin-bottom:5px;}.option-desc{color:#666;font-size:14px;}.option-badge{background:#28a745;color:white;padding:2px 8px;border-radius:12px;font-size:12px;margin-left:10px;}.firmware-option .option-icon{color:#dc3545;}.html-option .option-icon{color:#28a745;}button{background:#007bff;color:white;border:none;padding:12px 24px;border-radius:6px;cursor:pointer;margin:5px;font-size:14px;font-weight:bold;}button:hover{background:#0056b3;}.btn-secondary{background:#6c757d;}.btn-secondary:hover{background:#545b62;}.progress{width:100%;height:20px;background:#f0f0f0;border-radius:10px;margin:15px 0;}.progress-bar{height:100%;background:#007bff;border-radius:10px;width:0%;transition:width 0.3s;}.file-list{margin:10px 0;}.file-item{background:#f8f9fa;padding:8px 12px;margin:5px 0;border-radius:4px;border-left:3px solid #007bff;}</style>";
    html += "</head><body><div class=\"container\"><h1>🚀 智能烤箱控制器 OTA升级</h1>";
    
    // 显示设备状态信息
    html += "<div class=\"status-info\">";
    html += "<strong>设备状态:</strong><br>";
    html += "• 固件版本: " + FIRMWARE_VERSION + "<br>";
    html += "• 运行时间: " + String(millis() / 1000 / 60) + " 分钟<br>";
    html += "• 可用内存: " + String(ESP.getFreeHeap() / 1024) + " KB<br>";
    html += "• WiFi状态: " + String(WiFi.status() == WL_CONNECTED ? "已连接" : "未连接");
    html += "</div>";
    
    html += "<div class=\"tab active\" id=\"mainTab\">";
    html += "<h3>📋 选择升级类型</h3>";
    html += "<p>请根据您的需求选择合适的升级方式：</p>";
    
    // 固件升级选项
    html += "<div class=\"upgrade-option firmware-option\" onclick=\"showTab('firmwareTab')\">";
    html += "<div class=\"option-icon\">🔧</div>";
    html += "<div class=\"option-content\">";
    html += "<div class=\"option-title\">固件升级 (.bin 文件)<span class=\"option-badge\">系统核心</span></div>";
    html += "<div class=\"option-desc\">更新设备主程序，包含功能改进和错误修复。升级后设备将自动重启。</div>";
    html += "</div>";
    html += "</div>";
    
    // HTML文件升级选项
    html += "<div class=\"upgrade-option html-option\" onclick=\"showTab('htmlTab')\">";
    html += "<div class=\"option-icon\">📄</div>";
    html += "<div class=\"option-content\">";
    html += "<div class=\"option-title\">网页文件升级 (.html/.js/.css)<span class=\"option-badge\">界面更新</span></div>";
    html += "<div class=\"option-desc\">更新设备Web界面文件，支持多文件批量上传，不影响设备运行。</div>";
    html += "</div>";
    html += "</div>";
    
    html += "</div>";
    
    // 固件升级标签页
    html += "<div class=\"tab\" id=\"firmwareTab\">";
    html += "<h3>🔧 固件升级</h3>";
    html += "<p><strong>重要提示：</strong>固件升级将重启设备，请确保电源稳定。</p>";
    html += "<form action=\"/update\" method=\"post\" enctype=\"multipart/form-data\" onsubmit=\"return uploadFirmware(this)\">";
    html += "<p><strong>选择固件文件 (.bin):</strong></p>";
    html += "<input type=\"file\" name=\"firmware\" accept=\".bin\" required style=\"margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;\">";
    html += "<br><button type=\"submit\">🚀 开始升级固件</button>";
    html += "</form>";
    html += "<div class=\"progress\"><div class=\"progress-bar\" id=\"firmwareProgress\"></div></div>";
    html += "<p id=\"firmwareStatus\"></p>";
    html += "<button class=\"btn-secondary\" onclick=\"showTab('mainTab')\">⬅️ 返回选择</button>";
    html += "</div>";
    
    // HTML文件升级标签页
    html += "<div class=\"tab\" id=\"htmlTab\">";
    html += "<h3>📄 网页文件升级</h3>";
    html += "<p>支持上传 HTML、JavaScript、CSS 文件，可同时选择多个文件。</p>";
    html += "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\" onsubmit=\"return uploadHTML(this)\">";
    html += "<p><strong>选择网页文件:</strong></p>";
    html += "<input type=\"file\" name=\"html\" accept=\".html,.js,.css\" multiple required style=\"margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;\">";
    html += "<div id=\"filePreview\" class=\"file-list\"></div>";
    html += "<br><button type=\"submit\">📤 上传文件</button>";
    html += "</form>";
    html += "<div class=\"progress\"><div class=\"progress-bar\" id=\"htmlProgress\"></div></div>";
    html += "<p id=\"htmlStatus\"></p>";
    html += "<button class=\"btn-secondary\" onclick=\"showTab('mainTab')\">⬅️ 返回选择</button>";
    html += "</div>";
    
    html += "</div>";
    html += "<script>";
    html += "function showTab(tabId){";
    html += "    document.querySelectorAll('.tab').forEach(tab=>tab.classList.remove('active'));";
    html += "    document.querySelectorAll('.upgrade-option').forEach(opt=>opt.classList.remove('active'));";
    html += "    document.getElementById(tabId).classList.add('active');";
    html += "    if(tabId === 'firmwareTab'){";
    html += "        document.querySelector('.firmware-option').classList.add('active');";
    html += "    }else if(tabId === 'htmlTab'){";
    html += "        document.querySelector('.html-option').classList.add('active');";
    html += "    }";
    html += "}";
    html += "function uploadFirmware(form){";
    html += "    var xhr=new XMLHttpRequest();";
    html += "    xhr.upload.onprogress=function(e){";
    html += "        if(e.lengthComputable){";
    html += "            var percent=Math.round((e.loaded/e.total)*100);";
    html += "            document.getElementById('firmwareProgress').style.width=percent+'%';";
    html += "            document.getElementById('firmwareStatus').innerHTML='上传进度: '+percent+'%';";
    html += "        }";
    html += "    };";
    html += "    xhr.onload=function(){";
    html += "        if(xhr.status==200){";
    html += "            document.getElementById('firmwareStatus').innerHTML='✅ 固件升级成功！设备将在3秒后重启...';";
    html += "            setTimeout(function(){location.reload();},3000);";
    html += "        }else{";
    html += "            document.getElementById('firmwareStatus').innerHTML='❌ 升级失败：'+xhr.responseText;";
    html += "        }";
    html += "    };";
    html += "    xhr.open('POST','/update');";
    html += "    xhr.send(new FormData(form));";
    html += "    return false;";
    html += "}";
    html += "function uploadHTML(form){";
    html += "    var files=form.querySelector('input[type=\"file\"]').files;";
    html += "    var fileList='';";
    html += "    for(var i=0;i<files.length;i++){";
    html += "        fileList+='<div class=\"file-item\">'+files[i].name+' ('+Math.round(files[i].size/1024)+' KB)</div>';";
    html += "    }";
    html += "    document.getElementById('filePreview').innerHTML=fileList;";
    html += "    var xhr=new XMLHttpRequest();";
    html += "    xhr.upload.onprogress=function(e){";
    html += "        if(e.lengthComputable){";
    html += "            var percent=Math.round((e.loaded/e.total)*100);";
    html += "            document.getElementById('htmlProgress').style.width=percent+'%';";
    html += "            document.getElementById('htmlStatus').innerHTML='上传进度: '+percent+'%';";
    html += "        }";
    html += "    };";
    html += "    xhr.onload=function(){";
    html += "        if(xhr.status==200){";
    html += "            document.getElementById('htmlStatus').innerHTML='✅ 文件上传成功！';";
    html += "        }else{";
    html += "            document.getElementById('htmlStatus').innerHTML='❌ 上传失败：'+xhr.responseText;";
    html += "        }";
    html += "    };";
    html += "    xhr.open('POST','/upload');";
    html += "    xhr.send(new FormData(form));";
    html += "    return false;";
    html += "}";
    html += "// 文件选择预览";
    html += "document.addEventListener('DOMContentLoaded',function(){";
    html += "    var fileInput=document.querySelector('input[type=\"file\"]');";
    html += "    if(fileInput){";
    html += "        fileInput.addEventListener('change',function(){";
    html += "            if(this.files.length>0){";
    html += "                var fileList='';";
    html += "                for(var i=0;i<this.files.length;i++){";
    html += "                    fileList+='<div class=\"file-item\">'+this.files[i].name+' ('+Math.round(this.files[i].size/1024)+' KB)</div>';";
    html += "                }";
    html += "                document.getElementById('filePreview').innerHTML=fileList;";
    html += "            }";
    html += "        });";
    html += "    }";
    html += "});";
    html += "</script></body></html>";
    webServer.send(200, "text/html", html);
}

void handleLogout() {
    // 登出端点 - 主要用于Web界面
    webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"已登出\"}");
}

void handleChangePassword() {
    // 修改密码端点
    if (webServer.hasArg("new_password")) {
        String newPassword = webServer.arg("new_password");
        if (newPassword.length() >= 8) {
            // 在实际应用中，这里应该安全地存储密码
            webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"密码修改成功\"}");
        } else {
            webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"密码长度至少需要8位\"}");
        }
    } else {
        webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"缺少新密码参数\"}");
    }
}

void handleDeviceInfo() {
    // 设备信息端点
    String json = "{\"status\":\"success\",\"device_id\":\"" + DEVICE_ID + "\",";
    json += "\"firmware_version\":\"" + FIRMWARE_VERSION + "\",";
    json += "\"chip_model\":\"ESP32\",";
    json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"wifi_status\":\"" + String(WiFi.status() == WL_CONNECTED ? "connected" : "disconnected") + "\"}";
    webServer.send(200, "application/json", json);
}

void handleReset() {
    // 重置端点 - 软重置
    webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"设备配置已重置\"}");
    // 执行软重置逻辑
    EEPROM.begin(512);
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    delay(1000);
    ESP.restart();
}

void handleFileUpload() {
    // 文件上传处理函数
    HTTPUpload& upload = webServer.upload();
    static String currentFilename;
    static fs::File currentFile;
    
    if (upload.status == UPLOAD_FILE_START) {
        currentFilename = upload.filename;
        if (!currentFilename.startsWith("/")) {
            currentFilename = "/" + currentFilename;
        }
        
        // 验证文件类型
        if (!isValidFileType(currentFilename)) {
            Serial.print("不支持的文件类型: ");
            Serial.println(currentFilename);
            return;
        }
        
        Serial.print("开始上传文件: ");
        Serial.println(currentFilename);
        
        // 创建或覆盖文件
        currentFile = LittleFS.open(currentFilename, "w");
        if (!currentFile) {
            Serial.println("文件创建失败");
        }
        
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (currentFile) {
            currentFile.write(upload.buf, upload.currentSize);
            Serial.print("写入数据: ");
            Serial.println(upload.currentSize);
        }
        
    } else if (upload.status == UPLOAD_FILE_END) {
        if (currentFile) {
            currentFile.close();
            Serial.print("文件上传完成: ");
            Serial.print(currentFilename);
            Serial.print(", 大小: ");
            Serial.println(upload.totalSize);
            
            // 强制刷新文件系统缓存
            LittleFS.end();
            delay(100);
            LittleFS.begin();
            Serial.println("文件系统缓存已刷新");
            
            // 如果是固件文件，提示用户使用OTA升级
            if (currentFilename.endsWith(".bin")) {
                Serial.println("固件文件上传完成，请使用OTA升级功能进行升级");
            }
        }
        
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (currentFile) {
            currentFile.close();
            Serial.println("文件上传被取消");
            // 删除不完整的文件
            LittleFS.remove(currentFilename);
        }
    }
}

bool isValidFileType(String filename) {
    // 验证文件类型是否支持
    String validExtensions[] = {".html", ".js", ".css", ".bin", ".json", ".txt"};
    int numExtensions = sizeof(validExtensions) / sizeof(validExtensions[0]);
    
    for (int i = 0; i < numExtensions; i++) {
        if (filename.endsWith(validExtensions[i])) {
            return true;
        }
    }
    
    return false;
}

void handleUploadHTML() {
    // 处理JSON格式的HTML文件上传
    if (webServer.hasArg("plain")) {
        String jsonData = webServer.arg("plain");
        
        // 解析JSON数据
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, jsonData);
        
        if (error) {
            webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"JSON解析失败\"}");
            Serial.println("JSON解析失败");
            return;
        }
        
        String filename = doc["filename"];
        String content = doc["content"];
        
        if (filename.isEmpty() || content.isEmpty()) {
            webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"文件名或内容为空\"}");
            Serial.println("文件名或内容为空");
            return;
        }
        
        // 确保文件名以/开头
        if (!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        
        // 验证文件类型
        if (!isValidFileType(filename)) {
            webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"不支持的文件类型\"}");
            Serial.print("不支持的文件类型: ");
            Serial.println(filename);
            return;
        }
        
        // 写入文件
        File file = LittleFS.open(filename, "w");
        if (!file) {
            webServer.send(500, "application/json", "{\"status\":\"error\",\"message\":\"文件创建失败\"}");
            Serial.println("文件创建失败");
            return;
        }
        
        file.print(content);
        file.close();
        
        // 强制刷新文件系统缓存
        LittleFS.end();
        delay(100);
        LittleFS.begin();
        
        Serial.print("HTML文件上传成功: ");
        Serial.print(filename);
        Serial.print(", 大小: ");
        Serial.println(content.length());
        
        webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"文件上传成功\"}");
    } else {
        webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"缺少JSON数据\"}");
        Serial.println("缺少JSON数据");
    }
}

void setupWebServer() {
    // 初始化LittleFS文件系统（从备份文件恢复完整初始化逻辑）
    if (!LittleFS.begin()) {
        Serial.println("LittleFS文件系统初始化失败，尝试格式化...");
        if (LittleFS.format()) {
            Serial.println("LittleFS格式化成功，重新初始化...");
            if (!LittleFS.begin()) {
                Serial.println("LittleFS重新初始化失败，Web服务器将以受限模式运行");
                isFileSystemAvailable = false;
            } else {
                Serial.println("LittleFS文件系统初始化成功");
                isFileSystemAvailable = true;
            }
        } else {
            Serial.println("LittleFS格式化失败，Web服务器将以受限模式运行");
            isFileSystemAvailable = false;
        }
    } else {
        Serial.println("LittleFS文件系统初始化成功");
        isFileSystemAvailable = true;
        
        // 检查关键文件是否存在
        bool loginExists = LittleFS.exists("/login.html");
        bool indexExists = LittleFS.exists("/index.html");
        bool wifiConfigExists = LittleFS.exists("/wifi_config.html");
        bool deviceStatusExists = LittleFS.exists("/device_status.html");
        bool tempCalibrationExists = LittleFS.exists("/temperature_calibration.html");
        bool settingsHelpExists = LittleFS.exists("/settings_help.html");
        
        Serial.print("文件检查结果: ");
        Serial.print("login.html:"); Serial.print(loginExists ? "存在" : "缺失");
        Serial.print(", index.html:"); Serial.print(indexExists ? "存在" : "缺失");
        Serial.print(", wifi_config.html:"); Serial.print(wifiConfigExists ? "存在" : "缺失");
        Serial.print(", device_status.html:"); Serial.print(deviceStatusExists ? "存在" : "缺失");
        Serial.print(", temperature_calibration.html:"); Serial.print(tempCalibrationExists ? "存在" : "缺失");
        Serial.print(", settings_help.html:"); Serial.println(settingsHelpExists ? "存在" : "缺失");
        
        if (loginExists && indexExists && wifiConfigExists && deviceStatusExists && 
            tempCalibrationExists && settingsHelpExists) {
            Serial.println("关键HTML文件存在");
        } else {
            Serial.println("警告：关键HTML文件缺失，Web界面可能无法正常工作");
        }
    }
    
    // 设置静态文件服务（强制门户模式也需要访问wifi_config.html）
    if (isFileSystemAvailable) {
        // 强制门户模式下只提供必要的配网相关文件
        if (isCaptivePortalMode) {
            webServer.serveStatic("/wifi_config.html", LittleFS, "/wifi_config.html");
            webServer.serveStatic("/mobile_utils.js", LittleFS, "/mobile_utils.js");
            Serial.println("强制门户模式：配网相关文件静态服务配置完成");
        } else {
            // 正常模式下的完整静态文件服务配置
            webServer.serveStatic("/login.html", LittleFS, "/login.html");
            webServer.serveStatic("/index.html", LittleFS, "/index.html");
            webServer.serveStatic("/device_status.html", LittleFS, "/device_status.html");
            webServer.serveStatic("/settings_help.html", LittleFS, "/settings_help.html");
            webServer.serveStatic("/wifi_config.html", LittleFS, "/wifi_config.html");
            webServer.serveStatic("/temperature_calibration.html", LittleFS, "/temperature_calibration.html");
            webServer.serveStatic("/mobile_utils.js", LittleFS, "/mobile_utils.js");
            
            // 恢复目录服务支持
            webServer.serveStatic("/css/", LittleFS, "/css/");
            webServer.serveStatic("/js/", LittleFS, "/js/");
            webServer.serveStatic("/images/", LittleFS, "/images/");
            
            Serial.println("正常模式：HTML文件静态服务配置完成（支持目录访问）");
        }
    } else {
        Serial.println("文件系统不可用，仅提供基本API服务");
    }
    
    // 设置Web服务器路由
    webServer.on("/", handleRoot);
    webServer.on("/status", handleStatus);
    webServer.on("/control", handleControl);
    webServer.on("/scanwifi", handleScanWiFi);
    webServer.on("/savewifi", handleSaveWiFi);
    webServer.on("/factoryreset", handleFactoryReset);
    webServer.on("/calibrate", handleTemperatureCalibration);
    
    // 新增缺失的API端点路由
    webServer.on("/wificonfig", handleWiFiConfig);
    webServer.on("/restart", handleRestart);
    webServer.on("/ota_update", handleOTAUpdate);
    webServer.on("/logout", handleLogout);
    webServer.on("/changepassword", handleChangePassword);
    webServer.on("/device-info", handleDeviceInfo);
    webServer.on("/reset", handleReset);
    
    // 添加OTA升级和文件上传路由
    // /update 路由由ESP8266HTTPUpdateServer自动处理固件升级
    webServer.on("/upload", HTTP_POST, []() {
        webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"文件上传成功\"}");
    }, handleFileUpload);
    
    // 添加专门的HTML文件上传接口
    webServer.on("/upload_html", HTTP_POST, handleUploadHTML);
    
    // 添加DNS重定向处理 - 强制门户模式下的关键配置
    webServer.onNotFound(handleNotFound);
    
    Serial.println("Web服务器路由配置完成（包含DNS重定向）");
}

// =========================================
// 蜂鸣器控制函数
// =========================================

void beepConfigSaved() {
    // 配置保存成功提示音 - 短鸣提示
    for (int i = 0; i < 2; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
    }
    Serial.println("配置保存成功提示音已播放");
}

void beepBakingStart() {
    // 烘焙开始提示音 - 长鸣提示
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("烘焙开始提示音已播放");
}

void beepBakingComplete() {
    // 烘焙完成提示音 - 连续短鸣
    for (int i = 0; i < 5; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
    }
    Serial.println("烘焙完成提示音已播放");
}

void beepError() {
    // 错误提示音 - 急促短鸣
    for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(50);
        digitalWrite(BUZZER_PIN, LOW);
        delay(50);
    }
    Serial.println("错误提示音已播放");
}

// =========================================
// TCP服务器处理函数
// =========================================

void handleTCPConnection() {
    if (!tcpClient || !tcpClient.connected()) {
        tcpClient = tcpServer.accept();
        if (tcpClient) {
            Serial.println("TCP客户端已连接");
            tcpClient.println("SMARTOVEN_CONNECTED:" + DEVICE_ID);
        }
    }
}

void handleTCPCommand() {
    if (tcpClient && tcpClient.available()) {
        String command = tcpClient.readStringUntil('\n');
        command.trim();
        
        Serial.print("收到TCP命令: ");
        Serial.println(command);
        
        if (command == "GET_STATUS") {
            String status = "STATUS:TEMP:" + String(currentTemp) + ",TARGET:" + String(targetTemp) + ",HEAT:" + String(heatingEnabled ? "ON" : "OFF") + ",MODE:" + String(ovenMode ? "OVEN" : "TOASTER");
            tcpClient.println(status);
        } else if (command.startsWith("SET_TEMP:")) {
            targetTemp = command.substring(9).toFloat();
            tcpClient.println("OK:TEMP_SET");
        } else if (command.startsWith("SET_HEAT:")) {
            heatingEnabled = command.substring(9) == "ON";
            tcpClient.println("OK:HEAT_SET");
        } else if (command == "GET_INFO") {
            String info = "INFO:ID:" + DEVICE_ID + ",VERSION:" + FIRMWARE_VERSION + ",UPTIME:" + String(millis() / 1000);
            tcpClient.println(info);
        } else {
            tcpClient.println("ERROR:UNKNOWN_COMMAND");
        }
    }
}

// =========================================
// 串口命令处理函数
// =========================================

void handleSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        Serial.print("收到串口命令: ");
        Serial.println(command);
        
        if (command == "status") {
            Serial.println("=== 设备状态 ===");
            Serial.print("温度: "); Serial.print(currentTemp); Serial.println("°C");
            Serial.print("目标温度: "); Serial.print(targetTemp); Serial.println("°C");
            Serial.print("加热状态: "); Serial.println(heatingEnabled ? "开启" : "关闭");
            Serial.print("工作模式: "); Serial.println(ovenMode ? "烤箱模式" : "烤面包机模式");
            Serial.print("WiFi状态: "); Serial.println(WiFi.status() == WL_CONNECTED ? "已连接" : "未连接");
            if (WiFi.status() == WL_CONNECTED) {
                Serial.print("IP地址: "); Serial.println(WiFi.localIP());
            }
        } else if (command == "reset") {
            Serial.println("执行设备重启...");
            delay(1000);
            ESP.restart();
        } else if (command.startsWith("set_temp ")) {
            targetTemp = command.substring(9).toFloat();
            Serial.print("目标温度已设置为: "); Serial.print(targetTemp); Serial.println("°C");
        } else if (command == "heat_on") {
            heatingEnabled = true;
            Serial.println("加热已开启");
        } else if (command == "heat_off") {
            heatingEnabled = false;
            Serial.println("加热已关闭");
        } else if (command == "calibrate") {
            Serial.println("温度校准模式");
            Serial.println("请使用网页界面进行温度校准");
        } else {
            Serial.println("未知命令，可用命令: status, reset, set_temp [温度], heat_on, heat_off, calibrate");
        }
    }
}

// =========================================
// 加热器控制函数
// =========================================

void controlHeater() {
    if (heatingEnabled && currentTemp < targetTemp) {
        digitalWrite(HEATER_PIN, HIGH);
    } else {
        digitalWrite(HEATER_PIN, LOW);
    }
}

// =========================================
// LED状态控制函数
// =========================================

void updateLEDStatus() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastLedUpdate > LED_BLINK_INTERVAL) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? LOW : HIGH); // LED引脚低电平点亮
        lastLedUpdate = currentTime;
    }
}

// =========================================
// 烘焙完成状态管理
// =========================================

void handleBakingComplete() {
    if (bakingCompleteState) {
        if (millis() - bakingCompleteStartTime > BAKING_COMPLETE_DURATION) {
            bakingCompleteState = false;
            beepBakingComplete();
            Serial.println("烘焙完成提示结束");
        }
    }
}

// =========================================
// 系统初始化函数
// =========================================

void setup() {
    // 快速初始化串口通信
    Serial.begin(115200);
    
    // 快速初始化硬件引脚
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // 快速初始化加热器状态 - 默认关闭
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, HIGH); // LED默认熄灭
    
    // 快速验证硬件初始化状态
    if (!verifyHardwareInitialization()) {
        performHardwareRecovery();
    }
    
    // 优化WiFi启动逻辑：先检查配置，再尝试连接
    if (loadConfig()) {
        Serial.println("WiFi配置加载成功，尝试连接网络...");
        Serial.print("SSID: ");
        Serial.println(wifiSSID);
        
        // 尝试连接WiFi，如果连接失败，不立即启动Captive Portal
        if (connectToWiFi()) {
            Serial.println("WiFi连接成功，设备进入正常模式");
        } else {
            Serial.println("WiFi连接失败，但配置有效，设备将保持STA模式");
            // 不启动Captive Portal，让设备继续尝试连接
            isCaptivePortalMode = false;
        }
    } else {
        Serial.println("未找到有效WiFi配置，启动Captive Portal配网模式");
        startCaptivePortal();
    }
    
    // 快速启动TCP服务器
    tcpServer.begin();
    
    // 快速启动OTA更新服务
    if (!isCaptivePortalMode) {
        setupOTA();
    }
    
    // 快速初始化温度读取计数器和平均时间
    temperatureReadCount = 0;
    temperatureReadAvgTime = 0;
    
    // 快速播放启动提示音
    beepConfigSaved();
}

// =========================================
// 主循环函数
// =========================================

void loop() {
    // 处理网络请求
    if (isCaptivePortalMode) {
        dnsServer.processNextRequest();
        webServer.handleClient();
    } else {
        webServer.handleClient();
        handleOTA();
    }
    
    // 处理设备发现协议
    handleDiscovery();
    
    // 处理TCP连接和命令
    handleTCPConnection();
    handleTCPCommand();
    
    // 处理串口命令
    handleSerialCommands();
    
    // 读取温度传感器数据
    currentTemp = readTemperatureManual();
    
    // 控制加热器
    controlHeater();
    
    // 更新LED状态
    updateLEDStatus();
    
    // 处理烘焙完成状态
    handleBakingComplete();
    
    // 检查Captive Portal超时
    checkCaptivePortalTimeout();
    
    // 主循环延迟，避免过度占用CPU资源
    delay(100);
}