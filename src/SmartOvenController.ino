// =========================================
// 智能电烤箱控制器 v0.2.0
// =========================================
// 版本: 0.2.0
// 功能: 强制门户配网 + 设备自动发现 + OTA升级 + MAX6675手动SPI实现
// 更新: 修复MAX6675库兼容性问题，实现手动SPI通信
// =========================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <SPI.h>
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

// 设备信息
const String DEVICE_TYPE = "oven";
const String DEVICE_ID = "oven-" + String(ESP.getChipId());
const String DEVICE_NAME = "SmartOven";
const String FIRMWARE_VERSION = "0.2.0";

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

// =========================================
// MAX6675手动SPI通信函数
// =========================================

// 读取MAX6675原始数据（16位）
uint16_t readMAX6675RawData() {
  uint16_t data = 0;
  
  // 启用芯片
  digitalWrite(THERMO_CS, LOW);
  delayMicroseconds(1);
  
  // 读取16位数据（MSB优先）
  for (int i = 15; i >= 0; i--) {
    digitalWrite(THERMO_CLK, HIGH);  // 时钟上升沿
    delayMicroseconds(1);
    
    if (digitalRead(THERMO_DO)) {   // 读取数据位
      data |= (1 << i);
    }
    
    digitalWrite(THERMO_CLK, LOW);  // 时钟下降沿
    delayMicroseconds(1);
  }
  
  // 禁用芯片
  digitalWrite(THERMO_CS, HIGH);
  
  return data;
}

// 读取温度值（手动SPI实现）
float readTemperatureManual() {
  uint16_t rawData = readMAX6675RawData();
  
  // 检查错误标志位（D2位为0表示正常）
  if (!(rawData & 0x04)) {
    uint16_t tempBits = rawData >> 3;  // 右移3位获取温度数据
    float temperature = tempBits * 0.25;  // 每个单位0.25°C
    
    // 检查温度范围是否合理
    if (temperature >= -50.0 && temperature <= 400.0) {
      return temperature;
    } else {
      Serial.println("温度传感器读数超出范围");
      return -1.0;
    }
  } else {
    Serial.println("温度传感器读取错误 - 检查热电偶连接");
    return -1.0;
  }
}

// 设备发现
bool discoveryEnabled = true;
const unsigned long DISCOVERY_INTERVAL = 10000; // 10秒广播一次
unsigned long lastDiscoveryTime = 0;

// LED状态控制
bool ledState = false;
unsigned long lastLedUpdate = 0;
const unsigned long LED_BLINK_INTERVAL = 500; // LED闪烁间隔

// =========================================
// EEPROM配置存储
// =========================================
struct Config {
    char ssid[32];
    char password[64];
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
    Serial.print("签名: ");
    Serial.println(config.signature);
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
    
    if (strcmp(config.signature, "SMARTOVEN") == 0) {
        wifiSSID = String(config.ssid);
        wifiPassword = String(config.password);
        Serial.println("配置验证成功，加载有效配置");
        return true;
    } else {
        Serial.println("配置验证失败，签名不匹配");
        // 清空配置
        wifiSSID = "";
        wifiPassword = "";
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
    
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
    
    Serial.println("强制门户已启动");
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
    // 如果没有保存的WiFi配置
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("没有WiFi配置，需要启动强制门户");
        return true;
    }
    
    // 在setup()中，我们还没有尝试连接WiFi，所以WiFi.status()可能是未连接状态
    // 这里应该直接尝试连接WiFi，而不是直接启动强制门户
    Serial.println("有WiFi配置，尝试连接WiFi");
    return false;
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
    
    if (isCaptivePortalMode) {
        response = "SMARTOVEN_CAPTIVE:" + DEVICE_TYPE + ":" + DEVICE_ID + ":" + 
                  WiFi.softAPIP().toString() + ":" + DEVICE_NAME + ":" + 
                  String(currentTemp) + ":" + String(targetTemp) + ":" + FIRMWARE_VERSION;
    } else {
        response = "SMARTOVEN:" + DEVICE_TYPE + ":" + DEVICE_ID + ":" + 
                  WiFi.localIP().toString() + ":" + DEVICE_NAME + ":" + 
                  String(currentTemp) + ":" + String(targetTemp) + ":" + FIRMWARE_VERSION;
    }
    
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(response.c_str());
    udp.endPacket();
    
    Serial.println("发送发现响应: " + response);
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

// =========================================
// Web服务器处理
// =========================================
void setupWebServer() {
    webServer.on("/", HTTP_GET, handleRoot);
    webServer.on("/scanwifi", HTTP_GET, handleScanWiFi);
    webServer.on("/status", HTTP_GET, handleStatus);
    webServer.on("/control", HTTP_POST, handleControl);
    webServer.on("/savewifi", HTTP_POST, handleSaveWiFi);
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
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
}

// 处理所有未定义的路由，实现自动跳转
void handleNotFound() {
    if (isCaptivePortalMode) {
        // 在强制门户模式下，将所有未定义的路由重定向到首页
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "Redirect to configuration page");
    } else {
        webServer.send(404, "text/plain", "Not found");
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

void handleControl() {
    if (webServer.hasArg("target_temp")) {
        targetTemp = webServer.arg("target_temp").toFloat();
    }
    if (webServer.hasArg("heating")) {
        heatingEnabled = webServer.arg("heating") == "true";
    }
    
    webServer.send(200, "text/plain", "OK");
}

// =========================================
// 温度控制功能
// =========================================
void readTemperature() {
    currentTemp = readTemperatureManual();
    if (currentTemp < 0) {
        Serial.println("温度传感器读取错误");
    } else {
        Serial.println("当前温度: " + String(currentTemp) + "°C");
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
    
    if (currentTime - lastLedUpdate > LED_BLINK_INTERVAL) {
        if (isCaptivePortalMode) {
            // 强制门户模式：快速闪烁
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED状态: 强制门户模式 - 闪烁 (亮)");
            } else {
                Serial.println("LED状态: 强制门户模式 - 闪烁 (灭)");
            }
        } else if (WiFi.status() == WL_CONNECTED) {
            // WiFi连接成功：常亮
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED状态: WiFi连接成功 - 常亮");
        } else {
            // WiFi连接失败：慢速闪烁
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED状态: WiFi连接失败 - 慢速闪烁 (亮)");
            } else {
                Serial.println("LED状态: WiFi连接失败 - 慢速闪烁 (灭)");
            }
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
    
    Serial.println("初始化完成");
}

// =========================================
// 主循环
// =========================================
void loop() {
    // 处理强制门户
    if (isCaptivePortalMode) {
        dnsServer.processNextRequest();
        webServer.handleClient();
        checkCaptivePortalTimeout();
    }
    
    // 处理OTA升级
    handleOTA();
    
    // 处理设备发现
    handleDiscovery();
    
    // 温度控制
    readTemperature();
    controlHeater();
    
    // LED状态更新
    updateLED();
    
    delay(100);
}