// =========================================
// 智能烤箱控制器固件 v0.6.6
// =========================================
// 固件版本: 0.6.6
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
// =========================================
// 硬件引脚定义
// =========================================
#define THERMO_CLK   14  // MAX6675时钟引脚(D5/GPIO14)
#define THERMO_CS    12  // MAX6675片选引�?D6/GPIO12)
#define THERMO_DO    13  // MAX6675数据输出引脚(D7/GPIO13)
#define HEATER_PIN   5   // 加热控制引脚
#define BUZZER_PIN   4   // 蜂鸣器引�?GPIO4/D2)
#define LED_PIN      2   // LED指示灯引�?GPIO2/D4)
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
// 系统运行状态变�?
unsigned long temperatureReadCount = 0;
float temperatureReadAvgTime = 0;
const unsigned long WEB_SERVER_HANDLE_INTERVAL = 100; // 100ms处理一次Web请求，避免阻塞主循环占用过多CPU资源
// 硬件状态监�?
unsigned long hardwareFailureCount = 0;       // 硬件故障计数
unsigned long lastHardwareReset = 0;         // 上次硬件重置时间
bool hardwareInitialized = false;            // 硬件是否初始化完成标�?
// 设备信息定义
const String DEVICE_TYPE = "oven";
const String DEVICE_ID = "oven-" + String(ESP.getChipId());
const String DEVICE_NAME = "SmartOven";
const String FIRMWARE_VERSION = "0.6.6";
// WiFi配置参数
String wifiSSID = "";
String wifiPassword = "";
//  captive portal模式配置
bool isCaptivePortalMode = false;
unsigned long captivePortalStartTime = 0;
const unsigned long CAPTIVE_PORTAL_TIMEOUT = 300000; // 5分钟后自动退出门户模�?
const String AP_SSID = "SmartOven-" + String(ESP.getChipId());
const String AP_PASSWORD = "12345678";
// 温度控制变量
float currentTemp = 0.0;
float targetTemp = 180.0;
bool heatingEnabled = false;
bool ovenMode = true; // 烤箱工作模式：true=自动模式，false=手动模式
// 网络通信配置
const int DEFAULT_PORT = 8888;
// =========================================
// MAX6675温度传感器驱�?
// =========================================
// 读取MAX6675原始数据 - 16位数据格式，包含温度信息和状态位
uint16_t readMAX6675RawData() {
  uint16_t data = 0;
  // 初始化MAX6675片选引�?- 先置高电平禁用通信
  digitalWrite(THERMO_CS, HIGH);
  digitalWrite(THERMO_CLK, LOW);
  delay(10);  // 等待芯片稳定 - 延迟10ms
  // 开始数据读�?
  digitalWrite(THERMO_CS, LOW);
  delayMicroseconds(100);  // 等待转换完成 - 确保数据稳定
  // 读取16位数�?- 从高位到低位
  for (int i = 15; i >= 0; i--) {
    digitalWrite(THERMO_CLK, HIGH);  // 时钟信号高电�?- 读取数据�?
    delayMicroseconds(5);  // 短暂延迟确保数据稳定读取
    if (digitalRead(THERMO_DO)) {   // 读取数据�?- 如果DO引脚为高电平则设置当前位
      data |= (1 << i);
    }
    digitalWrite(THERMO_CLK, LOW);  // 时钟信号低电�?- 结束当前位读�?
    delayMicroseconds(5);  // 短暂延迟确保数据稳定
  }
  // 结束数据读取
  digitalWrite(THERMO_CS, HIGH);
  delayMicroseconds(100);  // 等待通信完成 - 确保芯片进入空闲状�?
  // 检查传感器数据有效�?- 判断是否通信正常
  if (data == 0x0000 || data == 0xFFFF) {
    Serial.println("传感器通信错误: MAX6675未连接或读取失败");
  }
  return data;
}
// 温度校准参数配置
float temperatureOffset = 0.0;  // 温度补偿�?- 用于校准温度传感器读数偏�?
float temperatureScale = 1.0;    // 温度缩放系数 - 用于调整温度读数的比例关�?
// 硬件初始化验证
bool verifyHardwareInitialization() {
    Serial.println("验证硬件初始化状态...");
    // 初始化MAX6675引脚模式
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    // 配置MAX6675初始状态
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    delay(100);  // 等待传感器稳定- 延迟100ms
    // 检查MAX6675传感器响应状态
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
    // 增加硬件故障计数
    hardwareFailureCount++;
    // 重置MAX6675传感�?- 尝试恢复通信
    for (int i = 0; i < 5; i++) {
        digitalWrite(THERMO_CS, HIGH);
        digitalWrite(THERMO_CLK, LOW);
        delay(200);  // 等待传感器稳�?- 延迟200ms
        digitalWrite(THERMO_CS, LOW);
        delay(100);
        digitalWrite(THERMO_CS, HIGH);
        delay(200);
    }
    // 重新初始化MAX6675引脚配置
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    lastHardwareReset = millis();
    Serial.println("硬件恢复流程已执行，失败次数: " + String(hardwareFailureCount));
}

// 初始化MAX6675 SPI通信接口 - 确保数据传输稳定�?
float readTemperatureManual() {
    // 温度读取带重试机�?- 最多尝�?次读取传感器数据
    for (int retry = 0; retry < 3; retry++) {
        uint16_t rawData = readMAX6675RawData();
        // 输出当前重试次数及原始数�?
        Serial.print("读取尝试"); Serial.print(retry + 1); 
        Serial.print(": 原始数据: 0x"); Serial.println(rawData, HEX);
        // 检查传感器数据有效�?- 排除无效数据�?x0000�?xFFFF�?
        if (rawData == 0x0000 || rawData == 0xFFFF) {
            if (retry < 2) {
                Serial.println("传感器数据无效，准备重试...");
                delay(100);  // 重试前延�?00ms
                continue;
            } else {
                Serial.println("传感器通信失败 - 已达最大重试次数，无法获取有效数据 - 将返回默认温�?);
                // 传感器读取失败，增加硬件故障计数
                hardwareFailureCount++;
                // 传感器读取失败，已达最大重试次数，返回默认温度 25.0°C
                Serial.println("传感器读取失败，返回默认温度: 25.0°C");
                return 25.0;
            }
        }
        // 检查传感器连接状态位 - �?位为0表示连接正常
        if (!(rawData & 0x04)) {
            uint16_t tempBits = rawData >> 3;  // 将原始数据右�?位以提取温度相关�?
            float temperature = tempBits * 0.25;  // 每一位代�?.25°C，计算实际温度�?
            // 应用温度校准参数（缩放和偏移�?
            temperature = (temperature * temperatureScale) + temperatureOffset;
            // 验证温度值是否在有效范围内（-50.0°C �?400.0°C�?
            if (temperature >= -50.0 && temperature <= 400.0) {
            Serial.print("读取到的温度: ");
            Serial.print(temperature); Serial.println("°C");
            // 重置传感器错误计数器，更新最后成功读取时�?
                if (retry == 0) {
                    hardwareFailureCount = 0;
                }
                return temperature;
            } else {
                Serial.println("温度值超出有效范�?);
                return -1.0;
            }
        } else {
            if (retry < 2) {
                Serial.println("传感器连接状态异�?- 尝试重新连接...");
                delay(100);  // 重试前延�?00ms
                continue;
            } else {
                Serial.println("传感器连接状态异�?- 已达最大重试次�?);
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
// 温度校准参数设置函数 - 用于调整传感器读数准确�?
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
    Serial.print("传感器测量温�? "); Serial.print(measuredTemp); Serial.println("°C");
    Serial.print("温度校准偏移�? "); Serial.print(temperatureOffset); Serial.println("°C");
    Serial.print("温度校准缩放系数: "); Serial.println(temperatureScale);
    // 保存温度校准参数到EEPROM
    saveConfig();
    Serial.println("温度校准参数已保存到EEPROM");
}
// 设备发现配置
bool discoveryEnabled = true;
const unsigned long DISCOVERY_INTERVAL = 10000; // 10秒设备发现广播间�?
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
    float temperatureOffset;  // 温度校准偏移�?
    float temperatureScale;    // 温度校准缩放系数
    char signature[16];  // 配置文件签名，用于验证配置有效�?
};

void saveConfig() {
    Config config;
    // 初始化配置结构体，清空内存空�?
    memset(&config, 0, sizeof(config));
    // 将WiFi配置参数复制到配置结构体
    strncpy(config.ssid, wifiSSID.c_str(), sizeof(config.ssid) - 1);
    strncpy(config.password, wifiPassword.c_str(), sizeof(config.password) - 1);
    strncpy(config.signature, "SMARTOVEN", sizeof(config.signature) - 1);
    // 应用温度校准参数到配置结构体
    config.temperatureOffset = temperatureOffset;
    config.temperatureScale = temperatureScale;
    // 确保字符串以null终止符结�?
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
    Serial.print("温度校准偏移�? ");
    Serial.print(config.temperatureOffset);
    Serial.println("°C");
    Serial.print("温度校准缩放系数: ");
    Serial.println(config.temperatureScale);
    Serial.print("配置签名: ");
    Serial.println(config.signature);
    // 配置保存成功提示�?- 短鸣提示
    beepConfigSaved();
}

bool loadConfig() {
    Config config;
    EEPROM.begin(512);
    EEPROM.get(0, config);
    EEPROM.end();
    Serial.println("从EEPROM加载配置参数...");
    Serial.print("配置签名: ");
    Serial.println(config.signature);
    Serial.print("配置SSID: ");
    Serial.println(config.ssid);
    Serial.print("密码长度: ");
    Serial.println(strlen(config.password));
    Serial.print("温度校准偏移�? ");
    Serial.print(config.temperatureOffset);
    Serial.println("°C");
    Serial.print("温度校准缩放系数: ");
    Serial.println(config.temperatureScale);
    if (strcmp(config.signature, "SMARTOVEN") == 0) {
        wifiSSID = String(config.ssid);
        wifiPassword = String(config.password);
        // 加载配置文件中的温度校准参数
        temperatureOffset = config.temperatureOffset;
        temperatureScale = config.temperatureScale;
        Serial.println("配置文件加载成功，应用温度校准参�?);
        Serial.print("温度校准偏移�? ");
        Serial.print(temperatureOffset);
        Serial.println("°C");
        Serial.print("温度校准缩放系数: ");
        Serial.println(temperatureScale);
        return true;
    } else {
        Serial.println("配置文件签名验证失败，使用默认配置参�?);
        // 初始化默认配置参�?
        wifiSSID = "";
        wifiPassword = "";
        // 重置温度校准参数为默认�?- 偏移量和比例系数
        temperatureOffset = 0.0;
        temperatureScale = 1.0;
        return false;
    }
}
// =========================================
// 网络配置与Captive Portal管理
// =========================================
void startCaptivePortal() {
    Serial.println("启动Captive Portal服务...");
    // 断开现有WiFi连接，准备启动AP模式
    WiFi.disconnect();
    delay(100);
    // 配置并启动WiFi接入点模�?
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
    Serial.print("AP名称: ");
    Serial.println(AP_SSID);
    Serial.print("AP IP地址: ");
    Serial.println(WiFi.softAPIP());
    // 配置DNS服务器参�?
    dnsServer.start(53, "*", WiFi.softAPIP());
    // 启动Web服务器服�?
    setupWebServer();
    webServer.begin();
    // 启动UDP服务用于设备发现广播
    udp.begin(8888);
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
    Serial.println("Captive portal启动成功 - DNS服务器运行在端口53，Web服务器运行在端口80，UDP服务运行在端�?888");
}

void stopCaptivePortal() {
    Serial.println("停止Captive Portal服务，关闭相关网络服�?..");
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
    // 设置WiFi连接超时处理机制，最多等待30秒连接成功
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
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
// WiFi连接管理函数 - 处理网络连接和重连逻辑
// =========================================
void connectToWiFi() {
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("未配置WiFi信息，启动Captive Portal进行网络配置");
        startCaptivePortal();
        return;
    }
    Serial.println("正在连接WiFi网络...");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("密码长度: ");
    Serial.println(wifiPassword.length());
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    // WiFi连接超时处理：最多尝�?0秒，期间�?00ms检查一次连接状�?
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        // 定期输出连接状态，帮助调试连接问题
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("连接状�? ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("闲置状�?); break;
                case WL_NO_SSID_AVAIL: Serial.println("无法找到指定SSID的WiFi网络"); break;
                case WL_SCAN_COMPLETED: Serial.println("WiFi网络扫描已完�?); break;
                case WL_CONNECTED: Serial.println("WiFi连接成功"); break;
                case WL_CONNECT_FAILED: Serial.println("连接失败 - 请检查密码是否正确"); break;
                case WL_CONNECTION_LOST: Serial.println("连接丢失 - 正在尝试重新连接"); break;
                case WL_DISCONNECTED: Serial.println("已断开连接 - 准备重新连接"); break;
                default: Serial.println("未知连接状态 - 请检查网络环境"); break;
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
        Serial.println("�?);
        // 启动Web服务�?- 提供设备控制和监控界�?
        setupWebServer();
        // 配置OTA升级服务 - 支持远程固件更新
        setupOTA();
        // 启动UDP服务 - 用于设备发现和网络通信
        udp.begin(8888);
        // WiFi连接成功时点亮LED指示灯，提供视觉反馈
        digitalWrite(LED_PIN, HIGH);
        // 连接成功后输出各服务启动状态和访问信息
        Serial.println("系统提示: Web服务器已启动并开始监听连�?);
        Serial.println("系统提示: UDP服务已启动，等待设备发现请求");
        Serial.println("系统提示: OTA升级服务已就绪，等待更新请求");
        Serial.println("设备已就�?- 请通过浏览器访问以下地址进行控制: http://" + WiFi.localIP().toString());
    } else {
        Serial.println("");
        Serial.println("WiFi连接失败，启动Captive Portal进行网络配置");
        Serial.print("连接失败原因: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("闲置状�?- 等待连接请求"); break;
            case WL_NO_SSID_AVAIL: Serial.println("未找到指定的WiFi网络 - 请检查SSID是否正确"); break;
            case WL_SCAN_COMPLETED: Serial.println("WiFi网络扫描已完�?- 正在尝试连接"); break;
            case WL_CONNECT_FAILED: Serial.println("连接失败 - 请检查密码是否正�?); break;
            case WL_CONNECTION_LOST: Serial.println("连接丢失 - 正在尝试重新连接"); break;
            case WL_DISCONNECTED: Serial.println("已断开连接 - 准备重新连接"); break;
            default: Serial.println("未知连接状�?- 请检查网络环�?); break;
        }
        startCaptivePortal();
    }
}

// =========================================
// 设备发现协议处理 - 响应网络中的设备发现请求
// =========================================
void handleDiscovery() {
    // 检查UDP数据�?- 响应设备发现请求并提供设备信�?
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
                Serial.println("收到设备发现请求，正在发送响�?);
                sendDiscoveryResponse();
            }
        }
    }
    
    // 定期广播设备发现信息，使客户端能够自动发现设�?
    if (discoveryEnabled && millis() - lastDiscoveryTime > DISCOVERY_INTERVAL) {
        broadcastDiscovery();
        lastDiscoveryTime = millis();
    }
}

void sendDiscoveryResponse() {
    String response;
    // 构建设备信息响应，包含设备名称、MAC地址、端口、类型、版本等信息
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
    // 发送响应到请求源，使用相同的UDP端口
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
    Serial.println("发送广播发现信�? " + broadcastMsg);
}

// =========================================
// OTA更新设置 - 配置远程固件升级功能
// =========================================
void setupOTA() {
    httpUpdater.setup(&otaServer);
    otaServer.begin();
    Serial.println("OTA服务器已启动并监听端�?8080");
    Serial.println("OTA更新页面地址: http://" + WiFi.localIP().toString() + ":8080/update");
}

void handleOTA() {
    if (!isCaptivePortalMode && WiFi.status() == WL_CONNECTED) {
        otaServer.handleClient();
    }
}

// OTA升级页面处理函数
void handleOTAWebPage() {
    String html = "<!DOCTYPE html><html><head><title>智能烤箱设备OTA固件升级</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
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
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-success\">当前固件版本: ' + data.current_version + '</div>';";
    html += "    if (data.update_available) {";
    html += "      document.getElementById('updateStatus').innerHTML += '<div class=\"status status-warning\">发现新版�? ' + data.latest_version + '</div>';";
    html += "    }";
    html += "  }).catch(error => {";
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-error\">检查更新失败，请重�?/div>';";
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
    html += "<h1>智能烤箱设备OTA固件升级</h1>";
    html += "<p>通过OTA功能可以远程升级设备固件，提升设备性能和功�?/p>";
    html += "</div>";
    html += "<div class=\"content\">";
    html += "<div class=\"info-card\">";
    html += "<p><strong>设备ID:</strong> " + DEVICE_ID + "</p>";
    html += "<p><strong>IP地址:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>OTA端口:</strong> 8080</p>";
    html += "</div>";
    html += "<div id=\"updateStatus\"></div>";
    html += "<div class=\"ota-section\">";
    html += "<h3>OTA固件升级操作</h3>";
    html += "<p>OTA功能允许您远程升级设备固件，无需物理连接设备。点击下方按钮将打开OTA升级页面，您可以在该页面上传新的固件文件进行升级操作�?/p>";
    html += "<button class=\"btn btn-success\" onclick=\"startOTA()\">打开OTA升级页面</button>";
    html += "<button class=\"btn\" onclick=\"checkUpdate()\">检查更�?/button>";
    html += "</div>";
    html += "<div style=\"text-align: center; margin-top: 20px;\">";
    html += "<a href=\"/\" style=\"color: #2196F3; text-decoration: none;\">返回设备控制面板</a>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</body></html>";
    webServer.send(200, "text/html", html);
}

// 韫囨瑦瀚濋懕娆忕箹閼卞吋銈艰箛娆掍紩妤规挸绻栭懕铏箠API
void handleCheckUpdate() {
    String json = "{\"current_version\":\"" + FIRMWARE_VERSION + "\",\"latest_version\":\"0.6.6\",\"update_available\":false}";
    // 返回JSON响应，包含设备状态信息，供前端页面显示和设备控制使用
    // 发送JSON响应给客户端，包含设备状态信�?
    webServer.send(200, "application/json", json);
}

// =========================================
// Web服务器设置函�?
// =========================================
void setupWebServer() {
    // 如果不是Captive Portal模式，则设置静态文件服务，提供登录页面、主页、CSS、JS和图片资�?
    if (!isCaptivePortalMode) {
        webServer.serveStatic("/login.html", LittleFS, "/login.html");
        webServer.serveStatic("/index.html", LittleFS, "/index.html");
        webServer.serveStatic("/css/", LittleFS, "/css/");
        webServer.serveStatic("/js/", LittleFS, "/js/");
        webServer.serveStatic("/images/", LittleFS, "/images/");
    }
    
    // 设置Web服务器路由，处理各种HTTP请求
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
    // 闁哄倻澧楅崸濠囧礉閻樺灚鐣盇PI缂佹棏鍨抽崑?
    // webServer.on("/temperature_history", HTTP_GET, handleTemperatureHistory);
    // webServer.on("/scheduler", HTTP_GET, handleScheduler);
    // webServer.on("/device_info", HTTP_GET, handleDeviceInfo);
    // webServer.on("/security_monitor", HTTP_GET, handleSecurityMonitor);
    // webServer.on("/energy_stats", HTTP_GET, handleEnergyStats);
    webServer.onNotFound(handleNotFound);
    webServer.begin();
}

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
    // 清理WiFi扫描结果，释放内�?
    WiFi.scanDelete();
}

void handleRoot() {
    if (isCaptivePortalMode) {
        // 如果是Captive Portal模式，则显示WiFi配置页面，否则显示设备控制页�?
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
        html += "  scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>扫描WiFi网络�?..';";
        html += "  wifiList.innerHTML = '<option value=\"\">扫描WiFi网络�?..</option>';";
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
        html += "      scanBtn.innerHTML = '<span class="status-indicator status-connected"></span>扫描完成 (' + data.networks.length + '个网�?';";
        html += "    } else {";
        html += "      wifiList.innerHTML = '<option value=\\\"\\\">未找到可用的WiFi网络</option>';";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>扫描失败，请重试';";
        html += "    }";
        html += "    scanBtn.disabled = false;";
        html += "  }).catch(error => {";
        html += "    wifiList.innerHTML = '<option value=\\\"\\\">扫描WiFi网络失败</option>';";
        html += "    scanBtn.disabled = false;";
        html += "    scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>扫描失败，请重试';";
        html += "  });";
        html += "}";
        html += "window.onload = scanWiFi;";
        html += "function confirmFactoryReset() {";
        html += "  if (confirm('警告：此操作将重置所有设置并清除WiFi配置！确定要继续吗？')) {";
        html += "    var resetBtn = document.querySelector('[onclick=\\\"confirmFactoryReset()\\\"]');";
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
        html += "<h1>智能烤箱控制�?- WiFi配置页面</h1>";
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
        html += "<label for=\"ssid\">WiFi閼句粙妾伴懕顓″箺缁傚嫯浼?/label>";
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
        // 温度控制相关的HTML和JavaScript
        html += "<script>";
        html += "let currentTargetTemp = " + String(targetTemp) + ";";
        html += "let currentHeatingState = " + String(heatingEnabled ? "true" : "false") + ";";
        html += "// 定期更新当前温度显示";
        html += "  fetch('/status')";
        html += "    .then(response => response.json())";
        html += "    .then(data => {";
        html += "      document.getElementById('current-temp').textContent = data.temperature.toFixed(1) + '°C';";
        html += "      document.getElementById('target-temp').textContent = data.target_temperature.toFixed(1) + '°C';";
        html += "      currentTargetTemp = data.target_temperature;";
        html += "      currentHeatingState = data.heating_enabled;";
        html += "      ";
        html += "      // 更新加热状态按钮样式和文本";
        html += "      const heatingBtn = document.getElementById('heating-btn');";
        html += "      const heatingText = document.getElementById('heating-text');";
        html += "      if (data.heating_enabled) {";
        html += "        heatingBtn.classList.add('active');";
        html += "        heatingText.textContent = '加热�?;";
        html += "      } else {";
        html += "        heatingBtn.classList.remove('active');";
        html += "        heatingText.textContent = '已关�?;";
        html += "      }";
        html += "    })";
        html += "    .catch(error => console.error('获取状态更新失�?, error));";
        html += "}";
        html += "// 调整目标温度";
        html += "function changeTemp(delta) {";
        html += "  const newTemp = Math.max(0, Math.min(300, currentTargetTemp + delta));";
        html += "  setTargetTemp(newTemp);";
        html += "}";
        html += "// 设置预设温度";
        html += "function setPresetTemp(temp) {";
        html += "  setTargetTemp(temp);";
        html += "}";
        html += "// 设置自定义温�?;
        html += "function setCustomTemp() {";
        html += "  const input = document.getElementById('temp-input');";
        html += "  const temp = parseInt(input.value);";
        html += "  if (!isNaN(temp) && temp >= 0 && temp <= 300) {";
        html += "    setTargetTemp(temp);";
        html += "    input.value = '';";
        html += "  } else {";
        html += "    alert('请输入有效的温度�?(0-300°C)');";
        html += "  }";
        html += "}";
        html += "// 设置目标温度并发送到服务�?;
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
        html += "        throw new Error('设置目标温度失败');";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('设置目标温度时发生错�?, error);";
        html += "      alert('设置目标温度失败，请重试');";
        html += "    });";
        html += "}";
        html += "// 温度控制相关的UI更新函数";
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
        html += "          heatingText.textContent = '加热�?;";
        html += "        } else {";
        html += "          heatingBtn.classList.remove('active');";
        html += "          heatingText.textContent = '已关�?;";
        html += "        }";
        html += "        console.log('加热状态已更新�? ' + (newHeatingState ? '开�? : '关闭'));";
        html += "      } else {";
        html += "        throw new Error('切换加热状态失�?);";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('切换加热状态时发生错误', error);";
        html += "      alert('切换加热状态失败，请重�?);";
        html += "    });";
        html += "}";
        html += "// 设置定期更新温度显示的定时器";
        html += "setInterval(updateTemperatureDisplay, 2000);";
        html += "// 页面加载完成后初始化温度显示";
        html += "document.addEventListener('DOMContentLoaded', function() {";
        html += "  updateTemperatureDisplay();";
        html += "  ";
        html += "  // 为温度输入框添加回车键事件监�?;
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
        // 如果未登录，重定向到登录页面
        webServer.sendHeader("Location", "/login.html", true);
        webServer.send(302, "text/plain", "Redirecting to login page");
    }
}

// 处理未找到页面的请求
void handleNotFound() {
    if (isCaptivePortalMode) {
        // 在captive portal模式下重定向到配置页�?
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "Redirect to configuration page");
    } else {
        // 正常模式下返�?04错误
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
        html += "<div class=\"success-icon\">�?/div>"; 
        html += "<h1 class=\"success-title\">WiFi配置保存成功</h1>";
        html += "<p class=\"success-message\">WiFi配置已保存，设备将重启以应用新设置。请确保您的WiFi网络可用，重启后设备将连接到指定网络�?/p>";
        html += "<p class=\"countdown\">5秒后自动跳转至配置页�?..</p>";
        html += "</div>";
        html += "</body></html>";
        webServer.send(200, "text/html", html);
        delay(1000);
        ESP.restart();
    }
}

void handleFactoryReset() {
    // 初始化EEPROM并准备清除配置数�?
    EEPROM.begin(512);
    // 将EEPROM所有字节写�?以清除配置数�?
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    // 提交EEPROM更改并准备重启设备以完成重置
    wifiSSID = "";
    wifiPassword = "";
    temperatureOffset = 0.0;
    temperatureScale = 1.0;
    Serial.println("工厂重置已完成，准备重启设备...");
    // 构建工厂重置完成后的HTML响应页面
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
    html += "<div class=\"reset-icon\">�?/div>"; 
    html += "<h1 class=\"reset-title\">工厂重置已完�?/h1>"; 
    html += "<p class=\"reset-message\">所有设备设置已恢复出厂默认值，包括WiFi配置和用户偏好设置。设备将重启并进入配置模式�?/p>"; 
    html += "<p class=\"reset-message\">请在设备重启后重新连接WiFi网络并进行必要的配置�?/p>"; 
    html += "<p class=\"countdown\">5秒后设备将自动重�?..</p>"; 
    html += "</div>";
    html += "</body></html>";
    webServer.send(200, "text/html", html);
    delay(1000);
    ESP.restart();
}

void handleRestart() {
    Serial.println("Factory reset completed, restarting device...");
    // 重启设备以应用工厂重置设�?
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
    html += "<h1 class=\"restart-title\">设备重启�?/h1>"; 
    html += "<p class=\"restart-message\">设备正在重启以应用新设置，此过程可能需�?0秒。请保持设备电源连接，不要断开电源�?/p>"; 
    html += "<p class=\"restart-message\">重启完成后，设备将自动连接到指定的WiFi网络。您可以通过配置页面查看设备状态�?/p>"; 
    html += "<p class=\"countdown\">5秒后自动跳转至配置页�?..</p>"; 
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
    // Initialize communication with MAX6675 thermocouple sensor
    uint16_t rawData = readMAX6675RawData();
    bool errorFlag = (rawData & 0x04) != 0;
    uint16_t tempBits = rawData >> 3;
    float rawTemperature = tempBits * 0.25;
    // 根据错误标志判断传感器状态并准备诊断消息
    String sensorStatus = "正常"; 
    String diagnosticAdvice = "请检查传感器连接是否松动或损�?; 
    if (errorFlag) {
        sensorStatus = "传感器错�?; 
        diagnosticAdvice = "热电偶传感器连接异常，请检查接线是否牢固或传感器是否损�?; 
    } else if (rawData == 0x0000 || rawData == 0xFFFF) {
        sensorStatus = "未连�?; 
        diagnosticAdvice = "传感器未连接，请检查接线是否正确或传感器是否正常工�?; 
    } else if (rawTemperature < -20 || rawTemperature > 1024) {
        sensorStatus = "温度异常"; 
        diagnosticAdvice = "检测到异常温度值，请确认传感器是否正常工作或环境温度是否在合理范围"; 
    }
    
    // 构建包含传感器诊断结果的JSON响应
    String json = "{";
    json += "\"sensor_status\":\"" + sensorStatus + "\",";
    json += "\"raw_data\":\"0x" + String(rawData, HEX) + "\",";
    json += "\"error_flag\":" + String(errorFlag ? "true" : "false") + ",";
    json += "\"temp_bits\":" + String(tempBits) + ",";
    json += "\"raw_temperature\":" + String(rawTemperature) + ",";
    json += "\"calibration_params\":\"温度偏移�? " + String(temperatureOffset) + " 温度缩放系数: " + String(temperatureScale) + "\",";
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
    bool wasHeating = heatingEnabled;  // 保存之前的加热状�?
    if (webServer.hasArg("target_temp")) {
        targetTemp = webServer.arg("target_temp").toFloat();
    }
    if (webServer.hasArg("heating")) {
        heatingEnabled = webServer.arg("heating") == "true";
    }
    
    // 检测加热状态变化并触发蜂鸣器提�?
    if (!wasHeating && heatingEnabled) {
        // 从关闭到开启：开始烘�?
        beepBakingStart();
        Serial.println("烘烤开�?- 目标温度: " + String(targetTemp) + "°C");
    } else if (wasHeating && !heatingEnabled) {
        // 从开启到关闭：烘烤完�?
        beepBakingComplete();
        Serial.println("烘烤完成 - 最终温�? " + String(currentTemp) + "°C");
        // 触发烘烤结束状态：快闪10�?
        bakingCompleteState = true;
        bakingCompleteStartTime = millis();
        Serial.println("触发烘烤结束快闪状态，持续10�?);
    }
    webServer.send(200, "text/plain", "OK");
}

// =========================================
// 新增API端点处理函数
// =========================================

// 温度历史记录端点
void handleTemperatureHistory() {
    String json = "{\"temperature_history\":[";
    // 模拟温度历史数据（实际应用中可以从EEPROM或文件系统读取）
    for (int i = 0; i < 10; i++) {
        if (i > 0) json += ",";
        json += "{\"timestamp\":" + String(millis() - i * 60000) + ",";
        json += "\"temperature\":" + String(currentTemp - i * 0.5) + "}";
    }
    json += "]}";
    webServer.send(200, "application/json", json);
}

// 定时任务管理端点
void handleTimerTasks() {
    if (webServer.method() == HTTP_GET) {
        // 获取定时任务列表
        String json = "{\"timers\":[";
        json += "{\"id\":1,\"enabled\":false,\"target_temp\":180,\"duration\":3600}";
        json += "]}";
        webServer.send(200, "application/json", json);
    } else if (webServer.method() == HTTP_POST) {
        // 创建或更新定时任�?
        if (webServer.hasArg("action") && webServer.arg("action") == "create") {
            webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"定时任务创建成功\"}");
        } else {
            webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"定时任务更新成功\"}");
        }
    }
}

// 设备信息端点
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

// 安全监控端点
void handleSafetyMonitor() {
    String json = "{";
    json += "\"temperature_alerts\":[";
    json += "{\"type\":\"high_temperature\",\"threshold\":250,\"current\":" + String(currentTemp) + ",\"triggered\":" + String(currentTemp > 250 ? "true" : "false") + "}";
    json += "],";
    json += "\"safety_status\":\"normal\",";
    json += "\"last_check\":" + String(millis()) + ",";
    json += "\"recommendations\":[";
    json += "\"保持设备周围通风良好\",";
    json += "\"定期检查温度传感器\",";
    json += "\"避免长时间高温运行\";";
    json += "]}";
    webServer.send(200, "application/json", json);
}

// 能耗统计端�?
void handleEnergyStats() {
    String json = "{";
    json += "\"total_energy_used\":" + String(millis() / 3600000.0 * 1500) + ","; // 模拟能耗数�?
    json += "\"current_power\":" + String(heatingEnabled ? "1500" : "0") + ",";
    json += "\"today_energy\":" + String(millis() / 86400000.0 * 1500) + ",";
    json += "\"energy_unit\":\"Wh\",";
    json += "\"power_unit\":\"W\",";
    json += "\"stats_period\":\"lifetime\"";
    json += "}";
    webServer.send(200, "application/json", json);
}

// =========================================
// 温度控制功能
// =========================================

// 温度读取定时�?
unsigned long lastTemperatureRead = 0;
const unsigned long TEMPERATURE_READ_INTERVAL = 200; // 200ms读取一次温度（优化：提高温度响应速度）
void readTemperature() {
    unsigned long currentTime = millis();
    // 定时读取温度，避免频繁读取影响性能
    if (currentTime - lastTemperatureRead >= TEMPERATURE_READ_INTERVAL) {
        currentTemp = readTemperatureWithMonitoring();
        if (currentTemp < 0) {
            Serial.println("温度传感器读取错�?);
        } else {
            // 只有在温度变化较大时才打印日志，减少串口输出
            static float lastPrintedTemp = -999;
            if (fabs(currentTemp - lastPrintedTemp) >= 0.5) {
                Serial.println("当前温度读数: " + String(currentTemp) + "°C");
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
// LED状态指示与系统状态反�?
// =========================================
void updateLED() {
    unsigned long currentTime = millis();
    // 烘焙完成状态：控制LED闪烁和蜂鸣器提示
    if (bakingCompleteState) {
        if (currentTime - bakingCompleteStartTime > BAKING_COMPLETE_DURATION) {
            // 烘焙完成状态超时，恢复正常状�?
            bakingCompleteState = false;
            Serial.println("烘焙完成状态已结束，系统恢复正常监控模�?);
        }
    }
    if (currentTime - lastLedUpdate > LED_BLINK_INTERVAL) {
        if (bakingCompleteState) {
            // 烘焙完成状态：快速闪烁模式（500ms间隔�?
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED状�? 烘焙完成状�?- 快闪模式(�?");
            } else {
                Serial.println("LED状�? 烘焙完成状�?- 快闪模式(�?");
            }
        } else if (isCaptivePortalMode) {
            // 强制门户模式：慢速闪烁模式（1000ms间隔�?
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED状�? 强制门户模式 - 慢闪模式(�?");
            } else {
                Serial.println("LED状�? 强制门户模式 - 慢闪模式(�?");
            }
        } else if (heatingEnabled) {
            // 加热状态：LED常亮
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED状�? 加热状�?- 常亮模式");
        } else {
            // 空闲状态：慢速闪烁模式（1000ms间隔�?
            if (currentTime - lastLedUpdate > 1000) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState ? HIGH : LOW);
                if (ledState) {
                    Serial.println("LED状�? 空闲状�?- 慢闪模式(�?");
                } else {
                    Serial.println("LED状�? 空闲状�?- 慢闪模式(�?");
                }
                lastLedUpdate = currentTime;
            }
            return; // 空闲状态LED控制已更新lastLedUpdate
        }
        lastLedUpdate = currentTime;
    }
}

// =========================================
// 蜂鸣器控制函数：提供不同场景的声音反�?
// =========================================
void beep(int duration = 100) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}

// 蜂鸣器提示音控制函数
void beepConfigSaved() {
    // 配置保存成功提示音：短促三声
    beep(100);
    delay(100);
    beep(100);
    delay(100);
    beep(100);
    Serial.println("配置保存成功：蜂鸣器已发出确认提示音");
}

void beepBakingStart() {
    // 烘烤开始提示音：长-�?
    beep(300);
    delay(200);
    beep(100);
    Serial.println("蜂鸣器提示音: 烘烤开�?);
}

void beepBakingComplete() {
    // 烘烤完成提示音：长三�?
    beep(500);
    delay(200);
    beep(500);
    delay(200);
    beep(500);
    Serial.println("蜂鸣器提示音: 烘烤完成");
}

void beepTemperatureChange() {
    // 温度变化提示音：短促四声
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    Serial.println("蜂鸣器提示音: 温度变化");
}

// =========================================
// 系统初始化函�?
// =========================================
void setup() {
    Serial.begin(115200);
    Serial.println("");
    Serial.println("=========================================");
    Serial.println("智能烤箱控制器固件初始化�?- 版本 v" + FIRMWARE_VERSION);
    Serial.println("MAX6675温度传感器与SPI接口初始�?- 正在验证传感器连接状�?);
    Serial.println("=========================================");
    // 初始化加热器、蜂鸣器和LED引脚
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    // 初始化MAX6675温度传感器引�?
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    // MAX6675传感器硬件接口配�?
    digitalWrite(THERMO_CS, HIGH);  // CS引脚置高：禁用传感器通信，等待读取指�?
    digitalWrite(THERMO_CLK, LOW);  // CLK引脚初始化为低电平：准备时钟信号
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    // 系统启动提示音：两声短鸣
    beep(200);
    delay(100);
    beep(200);
    // 验证硬件初始化状�?
    if (!verifyHardwareInitialization()) {
        Serial.println("硬件初始化警告：温度传感器通信失败，请检查接线是否正确并尝试重启设备");
        // 尝试恢复硬件通信
        for (int i = 0; i < 3; i++) {
            digitalWrite(THERMO_CS, HIGH);
            digitalWrite(THERMO_CLK, LOW);
            delay(100);
            digitalWrite(THERMO_CS, LOW);
            delay(50);
            digitalWrite(THERMO_CS, HIGH);
            delay(100);
        }
        
        // 重新验证硬件状�?
        if (verifyHardwareInitialization()) {
            Serial.println("硬件初始化成功：温度传感器通信已恢�?);
        } else {
            Serial.println("硬件初始化失败：温度传感器未连接，请检查硬�?);
        }
    }
    
    // 加载系统配置
    if (loadConfig()) {
        Serial.println("配置加载成功：从EEPROM读取参数完成");
    } else {
        Serial.println("配置加载失败：使用默认参数初始化系统");
    }
    
    // 检查是否启动捕获门户模�?
    if (shouldStartCaptivePortal()) {
        startCaptivePortal();
    } else {
        connectToWiFi();
    }
    
    // 系统初始化完�?
    Serial.println("系统初始化完成：Web服务器已启动，等待客户端连接");
    // 启动TCP服务�?
    tcpServer.begin();
    Serial.println("TCP服务器已启动，监听端�? " + String(DEFAULT_PORT));
    // 初始化LittleFS文件系统
  if (LittleFS.begin()) {
    Serial.println("LittleFS文件系统初始化成�?);
        // 检查Web页面文件是否存在
        if (LittleFS.exists("/login.html")) {
            Serial.println("文件存在: Web登录页面 /login.html");
        } else {
            Serial.println("文件不存在：未找到Web登录页面 /login.html，使用默认界�?);
        }
        if (LittleFS.exists("/index.html")) {
            Serial.println("文件存在: 主页文件 /index.html");
        } else {
            Serial.println("文件不存在：未找到主页文�?/index.html，使用默认配�?);
        }
    } else {
        Serial.println("LittleFS初始化失败：无法挂载文件系统，使用内存存储模�?);
    }
    Serial.println("系统初始化完成：所有硬件模块已准备就绪");
}

// =========================================
// 串口命令处理函数
// =========================================
void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command.length() > 0) {
            Serial.println("接收到串口命�? " + command);
            if (command == "LED_ON") {
                digitalWrite(LED_PIN, HIGH);
                Serial.println("LED已开�?);
            } else if (command == "LED_OFF") {
                digitalWrite(LED_PIN, LOW);
                Serial.println("LED已关�?);
            } else if (command == "LED_BLINK") {
                // LED闪烁测试：闪�?�?
                for (int i = 0; i < 5; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(200);
                    digitalWrite(LED_PIN, LOW);
                    delay(200);
                }
                Serial.println("LED闪烁测试已完�?);
            } else if (command == "BEEP") {
                beep(100);
                Serial.println("蜂鸣器已触发 - 标准�?);
            } else if (command == "BEEP_LONG") {
                beep(500);
                Serial.println("蜂鸣器已触发 - 长音");
            } else if (command == "BEEP_SHORT") {
                beep(50);
                Serial.println("蜂鸣器已触发 - 短音");
            } else if (command == "GET_STATUS") {
                // 获取设备状态信息：温度，目标温度，加热状态，模式，运行时�?
                String statusResponse = "TEMP:" + String(currentTemp) + 
                                      ",TARGET:" + String(targetTemp) + 
                                      ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                                      ",MODE:" + String(ovenMode ? "1" : "0") + 
                                      ",UPTIME:" + String(millis() / 1000);
                Serial.println(statusResponse);
            } else if (command == "GET_TEMP") {
                Serial.println("当前温度: " + String(currentTemp) + "°C");
            } else if (command.startsWith("CALIBRATE_TEMP")) {
                // 温度校准：CALIBRATE_TEMP 实际温度�?
                int spaceIndex = command.indexOf(' ');
                if (spaceIndex > 0) {
                    String actualTempStr = command.substring(spaceIndex + 1);
                    float actualTemp = actualTempStr.toFloat();
                    float measuredTemp = currentTemp;
                    if (actualTemp > 0) {
                        calibrateTemperature(actualTemp, measuredTemp);
                        Serial.println("温度校准完成: 实际温度 = " + String(actualTemp) + "°C, 测量温度 = " + String(measuredTemp) + "°C");
                    } else {
                        Serial.println("参数错误: 实际温度必须大于0");
                    }
                } else {
                    Serial.println("命令格式错误: CALIBRATE_TEMP 需要指定实际温度�?);
                    Serial.println("例如: CALIBRATE_TEMP 16.0");
                }
            } else if (command == "RESET_CALIBRATION") {
                temperatureOffset = 0.0;
                temperatureScale = 1.0;
                Serial.println("温度校准已重�? 偏移�?= 0.0, 比例 = 1.0");
            } else if (command == "GET_RAW_TEMP") {
                // 获取原始温度数据：读取MAX6675传感器原始数据并解析
                uint16_t rawData = readMAX6675RawData();
                if (!(rawData & 0x04)) {
                    uint16_t tempBits = rawData >> 3;
                    float rawTemp = tempBits * 0.25;
                    Serial.println("原始温度数据(未校�?:");
                    Serial.print("原始传感器数�? 0x"); Serial.println(rawData, HEX);
                    Serial.print("温度数据�? "); Serial.println(tempBits);
                    Serial.print("原始温度�?未校�?: "); Serial.print(rawTemp); Serial.println("°C");
                    Serial.print("当前已校准温�? "); Serial.print(currentTemp); Serial.println("°C");
                } else {
                    Serial.println("传感器检测到错误: 热电偶断路或连接故障");
                }
            } else if (command == "GET_PERFORMANCE") {
                // 获取系统性能监控数据
                Serial.println("系统性能监控数据:");
                Serial.print("温度读取平均时间: "); Serial.print(temperatureReadAvgTime); Serial.println("ms");
                Serial.print("温度读取总次�? "); Serial.println(temperatureReadCount);
                // 内存状态监控与碎片率分�?
                uint32_t freeHeap = ESP.getFreeHeap();
                uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
                uint32_t heapFragmentation = ESP.getHeapFragmentation();
                Serial.print("可用堆内�? "); Serial.print(freeHeap); Serial.println(" bytes");
                Serial.print("最大可用块大小: "); Serial.print(maxFreeBlock); Serial.println(" bytes");
                Serial.print("堆内存碎片率: "); Serial.print(heapFragmentation); Serial.println("%");
                // 系统配置参数监控
                Serial.print("Web服务器处理间�? "); Serial.print(WEB_SERVER_HANDLE_INTERVAL); Serial.println("ms");
                Serial.print("温度读取间隔: "); Serial.print(TEMPERATURE_READ_INTERVAL); Serial.println("ms");
            } else if (command == "RESET_PERFORMANCE") {
                // 重置性能统计数据
                temperatureReadCount = 0;
                temperatureReadAvgTime = 0;
                Serial.println("性能统计已重�?);
            } else {
                Serial.println("未知命令，请使用以下支持的命�?");
                Serial.println("LED_ON, LED_OFF, LED_BLINK");
                Serial.println("BEEP, BEEP_LONG, BEEP_SHORT");
                Serial.println("GET_STATUS, GET_TEMP, GET_RAW_TEMP");
                Serial.println("CALIBRATE_TEMP 实际温度�?);
                Serial.println("RESET_CALIBRATION");
                Serial.println("GET_PERFORMANCE, RESET_PERFORMANCE");
            }
        }
    }
}

// =========================================
// TCP服务器连接处理函�?
// =========================================

void handleTCPConnection() {
    // 处理TCP客户端连接：检查是否有新的客户端连�?
    if (tcpServer.hasClient()) {
        // 检查是否有已连接的客户端：如果有连接则先断开
        if (tcpClient && tcpClient.connected()) {
            tcpClient.stop();
            Serial.println("TCP客户端连接已断开");
        }
        
        // 接受新的TCP客户端连�?
        tcpClient = tcpServer.available();
        if (tcpClient) {
            Serial.println("TCP客户端连接已建立，远程IP: " + tcpClient.remoteIP().toString());
            // 发送欢迎消息给客户�?
            tcpClient.println("SmartOven Controller v" + FIRMWARE_VERSION);
            tcpClient.println("欢迎使用智能烤箱控制器，请输入命�?");
        }
    }
    
    // 处理已连接的TCP客户端数据：接收并处理客户端发送的命令
    if (tcpClient && tcpClient.connected()) {
        if (tcpClient.available()) {
            String command = tcpClient.readStringUntil('\n');
            command.trim();
            if (command.length() > 0) {
                Serial.println("TCP客户端命令已接收: " + command);
                handleTCPCommand(command);
            }
        }
    }
}

void handleTCPCommand(String command) {
    if (command == "GET_STATUS") {
        // 获取设备状态信息：温度，目标温度，加热状态，模式，运行时�?
        String statusResponse = "TEMP:" + String(currentTemp) + 
                              ",TARGET:" + String(targetTemp) + 
                              ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                              ",MODE:" + String(ovenMode ? "1" : "0") + 
                              ",UPTIME:" + String(millis() / 1000);
        tcpClient.println(statusResponse);
        Serial.println("TCP客户端状态响应已发�? " + statusResponse);
    } else if (command == "GET_TEMP") {
        tcpClient.println("当前温度: " + String(currentTemp) + "°C");
    } else if (command.startsWith("SET_TEMP")) {
        // 设置目标温度：SET_TEMP 180.0
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            String tempStr = command.substring(spaceIndex + 1);
            float newTemp = tempStr.toFloat();
            if (newTemp >= 0 && newTemp <= 300) {
                targetTemp = newTemp;
                tcpClient.println("目标温度已设置为: " + String(targetTemp) + "°C");
                Serial.println("TCP客户端目标温度已设置: " + String(targetTemp) + "°C");
            } else {
                tcpClient.println("温度设置错误: 目标温度值必须在0-300°C范围内");
            }
        }
    } else if (command == "HEAT_ON") {
        heatingEnabled = true;
        tcpClient.println("加热功能已开启");
        Serial.println("TCP客户端加热功能已开启");
    } else if (command == "HEAT_OFF") {
        heatingEnabled = false;
        tcpClient.println("加热功能已关闭");
        Serial.println("TCP客户端加热功能已关闭");
    } else if (command == "OVEN_MODE") {
        ovenMode = true;
        tcpClient.println("已切换为烤箱模式: 加热逻辑已调整");
        Serial.println("TCP客户端已切换为烤箱模式");
    } else if (command == "TOASTER_MODE") {
        ovenMode = false;
        tcpClient.println("已切换为烤面包机模式: 加热逻辑已调整");
        Serial.println("TCP客户端已切换为烤面包机模式");
    } else if (command == "PING") {
        tcpClient.println("PONG");
    } else {
        tcpClient.println("未知TCP命令，请使用以下支持的命令");
        tcpClient.println("GET_STATUS, GET_TEMP, SET_TEMP [温度值]");
        tcpClient.println("HEAT_ON, HEAT_OFF, OVEN_MODE, TOASTER_MODE, PING");
    }
}

// =========================================
// 系统运行状态监控变�?
// =========================================

// 内存检查定时器：每5秒检查一次内存状�?
unsigned long lastMemoryCheck = 0;
const unsigned long MEMORY_CHECK_INTERVAL = 5000; // 内存检查间�? 5�?
// 温度读取定时器：记录上次温度读取时间
unsigned long lastTemperatureReadTime = 0;

// =========================================
// 温度控制变量
// =========================================

// 温度变化检测：跟踪目标温度变化，实现防抖功�?
float lastTargetTemp = targetTemp;
unsigned long lastTempChangeTime = 0;
const unsigned long TEMP_CHANGE_DEBOUNCE = 2000; // 温度变化防抖时间: 2�?
// Web服务器处理定时器：记录上次Web服务器处理时�?
unsigned long lastWebServerHandle = 0;
void loop() {
    unsigned long currentTime = millis();
    // 如果处于Captive Portal模式，则处理DNS请求和Web服务器客户端
    if (isCaptivePortalMode) {
        // Captive Portal模式: 处理DNS请求、Web服务器客户端和超时检�?
        dnsServer.processNextRequest();
        webServer.handleClient();
        checkCaptivePortalTimeout();
    } else {
        // 非Portal模式下按间隔处理Web服务器请�?
        if (currentTime - lastWebServerHandle >= WEB_SERVER_HANDLE_INTERVAL) {
            webServer.handleClient();
            lastWebServerHandle = currentTime;
        }
    }
    
    // 处理OTA升级请求
    handleOTA();
    // 处理设备发现请求
    handleDiscovery();
    // 处理TCP客户端连�?
    handleTCPConnection();
    // 处理串口命令
    handleSerialCommands();
    // 温度读取与加热控制主逻辑循环
    readTemperature();
    controlHeater();
    // 更新LED状�?
    updateLED();
    // 温度变化检�?
    if (targetTemp != lastTargetTemp) {
        // 目标温度发生变化: 防抖检查后执行温度变化处理
        if (currentTime - lastTempChangeTime > TEMP_CHANGE_DEBOUNCE) {
            Serial.println("目标温度变化: " + String(lastTargetTemp) + "°C -> " + String(targetTemp) + "°C");
            beepTemperatureChange();
            lastTargetTemp = targetTemp;
        }
        lastTempChangeTime = currentTime;
    }
    
        // 内存检查 - 5秒检查一次内存使用情况
    if (currentTime - lastMemoryCheck >= MEMORY_CHECK_INTERVAL) {
        checkMemoryUsage();
        lastMemoryCheck = currentTime;
    }
    
    // 主循环延�? 保证系统稳定运行，避免占用过多CPU资源
    delay(1);
}

// =========================================
// 内存使用监控函数
// =========================================

void checkMemoryUsage() {
    // ESP8266内存监控: 检查可用堆内存和碎片率
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
    uint32_t heapFragmentation = ESP.getHeapFragmentation();
    // 内存警告检�? 当可用堆内存低于20KB时发出警�?
    if (freeHeap < 20000) { // 内存警告阈�? 20KB
        Serial.println("内存警告：可用堆内存不足 - " + String(freeHeap) + " bytes");
        Serial.println("最大可用内存块大小: " + String(maxFreeBlock) + " bytes");
        Serial.println("堆内存碎片率: " + String(heapFragmentation) + "%");
    }
    
    // 温度监控统计: 定期显示温度读取性能数据，帮助优化系统性能
    if (temperatureReadCount > 0) {
        Serial.println("温度读取性能统计: 平均读取时间 " + String(temperatureReadAvgTime) + "ms, 总读取次�?" + String(temperatureReadCount));
        // 重置统计计数�? 清空性能统计数据，准备下一轮统计周�?
        temperatureReadCount = 0;
        temperatureReadAvgTime = 0;
    }
}

// 带性能监控的温度读取函�? 测量温度读取时间，统计读取性能，帮助优化系统响应速度
float readTemperatureWithMonitoring() {
    unsigned long startTime = micros();
    float temp = readTemperatureManual();
    unsigned long endTime = micros();
    unsigned long readTime = (endTime - startTime) / 1000; // 将时间转换为毫秒，便于性能监控
    // 性能统计更新: 如果温度读取有效，则更新平均读取时间和总读取次�?
    if (temp >= 0) {
        temperatureReadAvgTime = (temperatureReadAvgTime * temperatureReadCount + readTime) / (temperatureReadCount + 1);
        temperatureReadCount++;
    }
    return temp;
}
