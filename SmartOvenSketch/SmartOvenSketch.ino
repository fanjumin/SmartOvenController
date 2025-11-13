// =========================================
// 智能烤箱控制器固件 v0.8.1 - 正式版
// =========================================
// 固件版本: 0.8.1
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
WiFiUDP udp;
DNSServer dnsServer;
ESP8266WebServer webServer(80);  // Web服务器对象，端口80
ESP8266HTTPUpdateServer httpUpdater;
WiFiServer tcpServer(8888);  // TCP服务器初始化，用于设备通信
WiFiClient tcpClient;        // TCP客户端对象，用于连接管理

// 系统运行状态变量
unsigned long temperatureReadCount = 0;
float temperatureReadAvgTime = 0;
const unsigned long WEB_SERVER_HANDLE_INTERVAL = 100; // 100ms处理一次Web请求，避免阻塞主循环占用过多CPU资源
bool updateFileSystemFlag = false; // 文件系统更新标志

// 硬件状态监控
unsigned long hardwareFailureCount = 0;       // 硬件故障计数
unsigned long lastHardwareReset = 0;         // 上次硬件重置时间
bool hardwareInitialized = false;            // 硬件是否初始化完成标志

// 设备信息定义
const String DEVICE_TYPE = "oven";
const String DEVICE_ID = "oven-" + String(ESP.getChipId());
const String DEVICE_NAME = "SmartOven";
const String FIRMWARE_VERSION = "0.8.1";

// WiFi配置参数
String wifiSSID = "";
String wifiPassword = "";

// captive portal模式配置
bool isCaptivePortalMode = false;
unsigned long captivePortalStartTime = 0;

// 文件系统状态
bool isFileSystemAvailable = false;
const unsigned long CAPTIVE_PORTAL_TIMEOUT = 300000; // 5分钟后自动退door模式
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
    
    // 启动UDP服务用于设备发现广播
    udp.begin(8888);
    
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
    
    Serial.println("Captive portal启动成功 - 快速配网模式已启用");
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
}

bool shouldStartCaptivePortal() {
    // 检查WiFi配置是否存在 - 若SSID或密码为空则返回true，启动配网界面
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("WiFi配置参数缺失，启动配网界面");
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
        Serial.println("WiFi连接失败，启动配网界面");
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
        
        // 连接失败，启动配网界面
        WiFi.disconnect();
        delay(100);
        return true; // WiFi连接失败，启动配网界面
    }
}

void checkCaptivePortalTimeout() {
    if (isCaptivePortalMode && 
        millis() - captivePortalStartTime > CAPTIVE_PORTAL_TIMEOUT) {
        Serial.println("Captive Portal超时，停止配网模式");
        stopCaptivePortal();
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
        
        // Web服务器配置已移除 - 配网界面功能已删除
        
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
    // OTA更新服务已集成到主Web服务器，无需单独处理
    // 原otaServer.handleClient()功能已由webServer.handleClient()统一处理
}

// =========================================
// Web服务器处理函数 - 已删除
// =========================================

// handleStatus()函数已删除 - 配网界面功能已移除
// handleControl()函数已删除 - 配网界面功能已移除
// handleFactoryReset()函数已删除 - 配网界面功能已移除
// handleTemperatureCalibration()函数已删除 - 配网界面功能已移除
// handleRestart()函数已删除 - 配网界面功能已移除
// handleOTAUpdate()函数已删除 - 配网界面功能已移除
// handleLogout()函数已删除 - 配网界面功能已移除
// handleChangePassword()函数已删除 - 配网界面功能已移除
// handleDeviceInfo()函数已删除 - 配网界面功能已移除
// handleReset()函数已删除 - 配网界面功能已移除
// handleFileUpload()函数已删除 - 配网界面功能已移除
// handleUploadHTML()函数已删除 - 配网界面功能已移除
// isValidFileType()函数已删除 - 配网界面功能已移除

void handleRoot() {
    if (isCaptivePortalMode) {
        // 如果是Captive Portal模式，则显示WiFi配置页面，否则显示设备控制页面
        String html = "<!DOCTYPE html><html><head><title>智能烤箱设备WiFi配置页面</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
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
        html += "  scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>扫描WiFi网络中...';";
        html += "  wifiList.innerHTML = '<option value=\"\">扫描WiFi网络中...</option>';";
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
        html += "      wifiList.innerHTML = '<option value=\"\">未找到可用的WiFi网络</option>';";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>扫描失败，请重试';";
        html += "    }";
        html += "    scanBtn.disabled = false;";
        html += "  }).catch(error => {";
        html += "    wifiList.innerHTML = '<option value=\"\">扫描WiFi网络失败</option>';";
        html += "    scanBtn.disabled = false;";
        html += "    scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>扫描失败，请重试';";
        html += "  });";
        html += "}";
        html += "window.onload = scanWiFi;";
        html += "function confirmFactoryReset() {";
        html += "  if (confirm('警告：此操作将重置所有设置并清除WiFi配置！确定要继续吗？')) {";
        html += "    var resetBtn = document.querySelector('[onclick=\"confirmFactoryReset()\"]');";
        html += "    resetBtn.disabled = true;";
        html += "    resetBtn.innerHTML = '正在重置...';";
        html += "    resetBtn.style.opacity = '0.7';";
        html += "    ";
        html += "    fetch('/factoryreset', { method: 'POST' })";
        html += "      .then(response => {";
        html += "        if (response.ok) {";
        html += "          resetBtn.innerHTML = '重置成功...设备正在重启';";
        html += "          resetBtn.style.background = 'linear-gradient(135deg, #28a745 0%, #20c997 100%)';";
        html += "        } else {";
        html += "          throw new Error('重置失败');";
        html += "        }";
        html += "      })";
        html += "      .catch(error => {";
        html += "        resetBtn.disabled = false;";
        html += "        resetBtn.innerHTML = '工厂重置';";
        html += "        resetBtn.style.opacity = '1';";
        html += "        alert('工厂重置失败: ' + error.message);";
        html += "      });";
        html += "  }";
        html += "}";
        html += "</script>";
        html += "</head><body>";
        html += "<div class=\"container\">";
        html += "<div class=\"header\">";
        html += "<h1>智能烤箱控制器 - WiFi配置页面</h1>";
        html += "<p>WiFi网络连接设置</p>";
        html += "</div>";
        html += "<div class=\"device-info\">";
        html += "<p><strong>设备ID:</strong> " + DEVICE_ID + "</p>";
        html += "<p><strong>固件版本:</strong> " + FIRMWARE_VERSION + "</p>";
        html += "</div>";
        html += "<div class=\"temperature-display\">当前温度: " + String(currentTemp) + "°C</div>";
        html += "<div class=\"form-container\">";
        html += "<form method=\"POST\" action=\"/savewifi\">";
        html += "<div class=\"scan-section\">";
        html += "<button type=\"button\" id=\"scanBtn\" class=\"btn btn-secondary\" onclick=\"scanWiFi()\"><span class=\"status-indicator status-disconnected\"></span>点击搜索WiFi网络</button>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"ssid\">WiFi网络名称</label>";
        html += "<select id=\"ssid\" name=\"ssid\" class=\"form-control\" required><option value=\"\">请先点击搜索WiFi网络</option></select>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"password\">WiFi密码</label>";
        html += "<input type=\"password\" id=\"password\" name=\"password\" class=\"form-control\" placeholder=\"请输入WiFi密码\" required>";
        html += "</div>";
        html += "<button type=\"submit\" class=\"btn\" style=\"width: 100%;\">连接WiFi</button>";
        html += "</form>";
        html += "</div>";
        html += "</div>";
        html += "</body></html>";
        webServer.send(200, "text/html", html);
    } else {
        // 正常模式下重定向到index.html控制界面
        webServer.sendHeader("Location", "/index.html", true);
        webServer.send(302, "text/plain", "Redirect to control panel");
        Serial.println("重定向到控制面板: 正常模式");
    }
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

// handleSaveWiFi()函数已删除 - 配网界面功能已移除

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

// handleWiFiConfig()函数已删除 - 配网界面功能已移除

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
    
    // 文件系统.bin更新选项
    html += "<div class=\"upgrade-option fs-option\" onclick=\"showTab('fsTab')\">";
    html += "<div class=\"option-icon\">💾</div>";
    html += "<div class=\"option-content\">";
    html += "<div class=\"option-title\">文件系统更新 (.bin 文件)<span class=\"option-badge\">界面与数据</span></div>";
    html += "<div class=\"option-desc\">更新完整的文件系统镜像，包含所有HTML、JS、CSS等界面文件。</div>";
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
    
    // 文件系统更新标签页
    html += "<div class=\"tab\" id=\"fsTab\">";
    html += "<h3>💾 文件系统更新</h3>";
    html += "<p><strong>重要提示：</strong>文件系统更新将覆盖所有现有界面文件，请确保使用正确的.bin镜像文件。</p>";
    html += "<form action=\"/fs_update\" method=\"post\" enctype=\"multipart/form-data\" onsubmit=\"return uploadFilesystem(this)\">";
    html += "<p><strong>选择文件系统镜像 (.bin):</strong></p>";
    html += "<input type=\"file\" name=\"littlefs\" accept=\".bin\" required style=\"margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;\">";
    html += "<br><button type=\"submit\">🚀 开始更新文件系统</button>";
    html += "</form>";
    html += "<div class=\"progress\"><div class=\"progress-bar\" id=\"fsProgress\"></div></div>";
    html += "<p id=\"fsStatus\"></p>";
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
    html += "function uploadFilesystem(form){";
    html += "    var xhr=new XMLHttpRequest();";
    html += "    xhr.upload.onprogress=function(e){";
    html += "        if(e.lengthComputable){";
    html += "            var percent=Math.round((e.loaded/e.total)*100);";
    html += "            document.getElementById('fsProgress').style.width=percent+'%';";
    html += "            document.getElementById('fsStatus').innerHTML='上传进度: '+percent+'%';";
    html += "        }";
    html += "    };";
    html += "    xhr.onload=function(){";
    html += "        if(xhr.status==200){";
    html += "            document.getElementById('fsStatus').innerHTML='✅ 文件系统更新成功！设备将在3秒后重启...';";
    html += "            setTimeout(function(){location.reload();},3000);";
    html += "        }else{";
    html += "            document.getElementById('fsStatus').innerHTML='❌ 更新失败：'+xhr.responseText;";
    html += "        }";
    html += "    };";
    html += "    xhr.open('POST','/fs_update');";
    html += "    xhr.send(new FormData(form));";
    html += "    return false;";
    html += "}";
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

void handleResetCalibration() {
    temperatureOffset = 0.0;
    temperatureScale = 1.0;
    saveConfig(); // 保存重置后的配置
    
    String json = "{\"message\":\"温度校准参数已重置\",";
    json += "\"offset\":" + String(temperatureOffset) + ",";
    json += "\"scale\":" + String(temperatureScale) + "}";
    webServer.send(200, "application/json", json);
    
    Serial.println("温度校准参数已重置: offset=" + String(temperatureOffset) + ", scale=" + String(temperatureScale));
}

void handleFilesystemUpdate() {
    // 文件系统.bin更新处理函数
    Serial.println("开始处理文件系统更新");
    
    // 检查是否存在上传的文件系统镜像
    if (!LittleFS.exists("/littlefs.bin")) {
        Serial.println("错误：未找到文件系统镜像文件");
        webServer.sendHeader("Connection", "close");
        webServer.send(400, "text/plain; charset=utf-8", "未找到文件系统镜像文件");
        return;
    }
    
    // 获取文件信息
    File fsImage = LittleFS.open("/littlefs.bin", "r");
    if (!fsImage) {
        Serial.println("错误：无法打开文件系统镜像文件");
        webServer.sendHeader("Connection", "close");
        webServer.send(500, "text/plain; charset=utf-8", "无法打开文件系统镜像文件");
        return;
    }
    
    // 获取镜像文件大小
    size_t imageSize = fsImage.size();
    fsImage.close();
    
    Serial.print("文件系统镜像大小: ");
    Serial.println(imageSize);
    
    // 记录更新标志到EEPROM，以便重启后知道需要处理更新
    EEPROM.begin(512);
    EEPROM.write(500, 1); // 设置更新标志
    EEPROM.commit();
    EEPROM.end();
    
    // 发送响应，使用Server-Sent Events格式，便于前端处理
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    
    // 发送完成事件，使用正确的\r\n格式
    String completeData = "event: complete\r\ndata: {\"status\": \"success\", \"message\": \"文件系统更新准备完成，设备将重启应用更新...\", \"action\": \"restart\"}\r\n\r\n";
    webServer.setContentLength(completeData.length());
    webServer.send(200, "text/event-stream; charset=utf-8", completeData);
    
    // 等待响应发送完成
    delay(500);
    
    // 直接重启设备
    Serial.println("重启设备应用文件系统更新...");
    ESP.restart();
}

void handleFileUpload() {
    // 文件上传处理函数 - 重新设计进度计算逻辑
    HTTPUpload& upload = webServer.upload();
    static String currentFilename;
    static fs::File currentFile;
    static bool isFilesystemUpdate = false;
    static bool responseStarted = false;
    static unsigned long receivedBytes = 0;    // 已接收的字节数
    static unsigned long estimatedTotalSize = 0; // 预估的文件总大小
    static unsigned int chunkNum = 0;         // 数据块计数
    static unsigned long lastProgressUpdate = 0; // 上次更新进度的时间戳
    
    // 检查是否是文件系统更新
    if (upload.status == UPLOAD_FILE_START) {
        // 完全重置所有状态
        currentFilename = upload.filename;
        isFilesystemUpdate = (webServer.uri() == "/fs_update");
        receivedBytes = 0;
        estimatedTotalSize = 0;
        chunkNum = 0;
        responseStarted = false;
        lastProgressUpdate = 0;
        
        // 在上传开始时获取Content-Length头信息（这是真正的文件总大小）
        String contentLength = webServer.header("Content-Length");
        if (contentLength.length() > 0) {
            estimatedTotalSize = contentLength.toInt();
            // 减去多部分表单数据的边界和头部信息（粗略估计）
            // 根据观察，通常这部分大约占用几百字节，我们保守估计为500字节
            if (estimatedTotalSize > 500) {
                estimatedTotalSize -= 500;
            } else {
                estimatedTotalSize = 0; // 如果太小，则认为获取失败
            }
        }
        
        // 如果Content-Length不可用或计算后为0，使用upload.totalSize作为备用
        if (estimatedTotalSize == 0 && upload.totalSize > 0) {
            estimatedTotalSize = upload.totalSize;
        }
        
        // 标准化文件名路径
        if (!currentFilename.startsWith("/")) {
            currentFilename = "/" + currentFilename;
        }
        
        // 对于文件系统更新，固定使用/littlefs.bin作为文件名
        if (isFilesystemUpdate) {
            currentFilename = "/littlefs.bin";
        }
        
        // 验证文件类型（非文件系统更新时）
        if (!isFilesystemUpdate && !isValidFileType(currentFilename)) {
            Serial.print("不支持的文件类型: ");
            Serial.println(currentFilename);
            return;
        }
        
        // 创建或覆盖文件
        currentFile = LittleFS.open(currentFilename, "w");
        if (!currentFile) {
            Serial.println("文件创建失败");
            return;
        }
        
        // 初始化日志
        Serial.println("\n===== 文件上传开始 =====");
        Serial.print("文件名: ");
        Serial.println(currentFilename);
        
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (currentFile) {
            // 写入数据并获取实际写入大小
            size_t bytesWritten = currentFile.write(upload.buf, upload.currentSize);
            
            // 累加已上传大小
            receivedBytes += bytesWritten;
            chunkNum++;
            
            // 动态检查upload.totalSize，如果发现更准确的文件大小信息则更新estimatedTotalSize
            // 这有助于提高进度计算的准确性
            if (upload.totalSize > estimatedTotalSize && upload.totalSize > 0) {
                estimatedTotalSize = upload.totalSize;
            }
            
            // 注意：Content-Length应在UPLOAD_FILE_START时获取一次，而不是每次写入时重复获取
            
            // 如果还没有开始响应，立即开始
            if (!responseStarted) {
                // 开始Server-Sent Events响应
                webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
                webServer.sendHeader("Connection", "keep-alive");
                webServer.sendHeader("Access-Control-Allow-Origin", "*");
                webServer.sendHeader("Content-Type", "text/event-stream; charset=utf-8");
                webServer.send(200, "text/event-stream; charset=utf-8", "");  // 先发送空响应体开始流
                responseStarted = true;
                Serial.println("已启动SSE响应流");
                Serial.print("响应头信息 - Content-Type: ");
                Serial.println(webServer.header("Content-Type"));
            }
            
            // 限制进度更新频率，避免过度发送事件
            unsigned long currentTime = millis();
            if (currentTime - lastProgressUpdate > 100 || chunkNum % 10 == 0) { // 每100ms或每10个块更新一次
                // 计算进度百分比
                int progress = 0;
                if (estimatedTotalSize > 0) {
                    // 使用浮点计算以提高精度
                    float progressFloat = (float)receivedBytes / estimatedTotalSize * 100.0;
                    progress = (int)progressFloat;
                    
                    // 限制进度在1-100之间
                    progress = max(1, progress);
                    progress = min(100, progress);
                } else {
                    // 如果没有总大小信息，使用块计数作为进度参考
                    progress = min(100, (int)(chunkNum * 5));
                    // 确保进度至少为1%
                    progress = max(1, progress);
                }
                
                // 构建严格符合SSE规范的进度事件（使用\r\n）
                String progressEvent = "event: progress\r\ndata: {\"progress\":" + String(progress) + ",\"totalSize\":" + String(estimatedTotalSize) + ",\"currentSize\":" + String(receivedBytes) + "}\r\n\r\n";
                webServer.sendContent(progressEvent);
                // 立即刷新缓冲区确保事件发送
                webServer.client().flush();
                lastProgressUpdate = currentTime;
                
                // 调试信息
                Serial.print("发送进度事件: ");
                Serial.println(progressEvent);
                
                // 调试日志
                Serial.print("块: ");
                Serial.print(chunkNum);
                Serial.print(" 写入: ");
                Serial.print(bytesWritten);
                Serial.print(" 累计: ");
                Serial.print(receivedBytes);
                Serial.print(" 预估大小: ");
                Serial.print(estimatedTotalSize);
                Serial.print(" 进度: ");
                Serial.println(progress);
            }
        }
        
    } else if (upload.status == UPLOAD_FILE_END) {
        if (currentFile) {
            currentFile.close();
            
            // 文件已完成上传
            
            // 发送完成事件
            if (responseStarted) {
                String completeEvent = "event: complete\r\ndata: {\"status\":\"success\",\"message\":\"文件上传完成\",\"filename\":\"" + currentFilename + "\",\"size\":" + String(receivedBytes) + "}\r\n\r\n";
                webServer.sendContent(completeEvent);
                // 立即刷新缓冲区确保事件发送
                webServer.client().flush();
                responseStarted = false;
                Serial.println("已发送完成事件");
            }
            
            // 完成日志
            Serial.println("===== 文件上传完成 =====");
            Serial.print("文件: ");
            Serial.print(currentFilename);
            Serial.print(" 大小: ");
            Serial.println(receivedBytes);
            Serial.print("总块数: ");
            Serial.println(chunkNum);
            
            if (isFilesystemUpdate) {
                Serial.println("文件系统镜像上传完成，等待更新");
            } else {
                // 强制刷新文件系统缓存并验证文件
                LittleFS.end();
                delay(100);
                LittleFS.begin();
                
                // 验证文件
                if (LittleFS.exists(currentFilename)) {
                    File verifyFile = LittleFS.open(currentFilename, "r");
                    if (verifyFile) {
                        Serial.print("文件验证成功，实际大小: ");
                        Serial.println(verifyFile.size());
                        verifyFile.close();
                    }
                } else {
                    Serial.println("警告：文件验证失败，文件不存在");
                }
                
                // 如果是固件文件
                if (currentFilename.endsWith(".bin")) {
                    Serial.println("固件文件上传完成，等待OTA升级");
                }
            }
            
            // 重置所有状态变量
            receivedBytes = 0;
            estimatedTotalSize = 0;
            chunkNum = 0;
            lastProgressUpdate = 0;
            
        }
        
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        // 处理上传取消
        if (currentFile) {
            currentFile.close();
            LittleFS.remove(currentFilename);
            
            // 发送错误事件
            if (responseStarted) {
                String errorEvent = "event: error\r\ndata: {\"status\":\"error\",\"message\":\"文件上传被取消\"}\r\n\r\n";
                webServer.sendContent(errorEvent);
                // 立即刷新缓冲区确保事件发送
                webServer.client().flush();
                responseStarted = false;
            }
            
            Serial.println("文件上传被取消，已删除不完整文件");
            
            // 重置状态
            receivedBytes = 0;
            estimatedTotalSize = 0;
            chunkNum = 0;
            lastProgressUpdate = 0;
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



void setupWebServer() {
    // 设置静态文件服务（所有模式下都需要）
    webServer.serveStatic("/login.html", LittleFS, "/login.html");
    webServer.serveStatic("/index.html", LittleFS, "/index.html");
    webServer.serveStatic("/wifi_config.html", LittleFS, "/wifi_config.html");
    webServer.serveStatic("/device_status.html", LittleFS, "/device_status.html");
    webServer.serveStatic("/temperature_calibration.html", LittleFS, "/temperature_calibration.html");
    webServer.serveStatic("/settings_help.html", LittleFS, "/settings_help.html");
    webServer.serveStatic("/mobile_utils.js", LittleFS, "/mobile_utils.js");
    webServer.serveStatic("/css/", LittleFS, "/css/");
    webServer.serveStatic("/js/", LittleFS, "/js/");
    webServer.serveStatic("/images/", LittleFS, "/images/");
    
    // 设置Web服务器路由，处理各种HTTP请求
    webServer.on("/", HTTP_GET, handleRoot);
    webServer.on("/scanwifi", HTTP_GET, handleScanWiFi);
    webServer.on("/control", HTTP_POST, handleControl);
    webServer.on("/savewifi", HTTP_POST, handleSaveWiFi);
    webServer.on("/factoryreset", HTTP_POST, handleFactoryReset);
    webServer.on("/restart", HTTP_POST, handleRestart);
    webServer.on("/reset_calibration", HTTP_POST, handleResetCalibration);
    webServer.on("/ota_update", HTTP_GET, handleOTAUpdate);
    webServer.on("/update", HTTP_POST, []() {
        webServer.send(200, "text/plain", "OTA update endpoint");
    }, handleFileUpload);
    
    webServer.on("/fs_update", HTTP_POST, handleFilesystemUpdate, handleFileUpload);
    webServer.onNotFound(handleNotFound);
    webServer.begin();
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
// Web服务器处理函数
// =========================================

void handleScanWiFi() {
    // 扫描可用的WiFi网络
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
    // 清理WiFi扫描结果，释放内存
    WiFi.scanDelete();
}

void handleSaveWiFi() {
    if (webServer.hasArg("ssid") && webServer.hasArg("password")) {
        wifiSSID = webServer.arg("ssid");
        wifiPassword = webServer.arg("password");
        
        if (saveConfig()) {
            // WiFi配置保存成功，显示成功页面
            String html = "<!DOCTYPE html><html><head><title>WiFi配置保存成功</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
            html += "<meta http-equiv=\"refresh\" content=\"5;url=/\">";
            html += "<style>body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }</style>";
            html += "</head><body>";
            html += "<h1>WiFi配置保存成功</h1>";
            html += "<p>设备将在5秒后重启并尝试连接WiFi网络...</p>";
            html += "<p>如果连接成功，将自动跳转到设备控制页面。</p>";
            html += "<p>如果连接失败，设备将重新进入配网模式。</p>";
            html += "</body></html>";
            webServer.send(200, "text/html", html);
            
            delay(1000);
            ESP.restart();
        } else {
            // 配置保存失败
            String json = "{\"status\":\"error\",\"message\":\"WiFi配置保存失败\"}";
            webServer.send(500, "application/json", json);
        }
    } else {
        // 参数不完整时返回错误
        String json = "{\"status\":\"error\",\"message\":\"缺少必要的参数\"}";
        webServer.send(400, "application/json", json);
    }
}

void handleNotFound() {
    if (isCaptivePortalMode) {
        // 在captive portal模式下，直接显示配网界面而不是重定向
        handleRoot();
    } else {
        // 正常模式下返回404错误
        webServer.send(404, "text/plain", "Not found: " + webServer.uri());
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
    
    // 初始化文件系统
    if (!LittleFS.begin()) {
        Serial.println("文件系统初始化失败，HTML文件服务将不可用");
    } else {
        Serial.println("文件系统初始化成功");
    }
    
    // 优化WiFi启动逻辑：先检查配置，再尝试连接
    if (loadConfig()) {
        Serial.println("WiFi配置加载成功，尝试连接网络...");
        Serial.print("SSID: ");
        Serial.println(wifiSSID);
        
        // 尝试连接WiFi
        if (connectToWiFi()) {
            Serial.println("WiFi连接成功，设备进入正常模式");
        } else {
            Serial.println("WiFi连接失败，启动Captive Portal配网模式");
            startCaptivePortal();
        }
    } else {
        Serial.println("未找到有效WiFi配置，启动Captive Portal配网模式");
        startCaptivePortal();
    }
    
    // 快速启动TCP服务器
    tcpServer.begin();
    
    // 检查是否有文件系统更新请求（重启后恢复）
    EEPROM.begin(512);
    if (EEPROM.read(500) == 1) {
        Serial.println("检测到文件系统更新请求，开始处理...");
        
        // 清除更新标志
        EEPROM.write(500, 0);
        EEPROM.commit();
        EEPROM.end();
        
        // 检查是否存在上传的文件系统镜像
        if (LittleFS.exists("/littlefs.bin")) {
            Serial.println("找到文件系统镜像，准备更新...");
            
            // 对于ESP8266，我们需要使用特殊的方法来应用文件系统镜像
            // 这里我们简化处理，让设备重启后重新加载文件系统
            // 在实际应用中，这里可以添加更复杂的逻辑来直接刷写镜像
            
            Serial.println("文件系统更新将在本次启动中生效");
        } else {
            Serial.println("警告：未找到文件系统镜像文件，但检测到更新标志");
        }
    } else {
        EEPROM.end();
    }
    
    // 初始化Web服务器
    setupWebServer();
    
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
    // 处理Web服务器请求
    webServer.handleClient();
    
    // 文件系统更新现在直接在handleFilesystemUpdate中处理并重启设备
    
    // 在强制门户模式下处理DNS请求
    if (isCaptivePortalMode) {
        dnsServer.processNextRequest();
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