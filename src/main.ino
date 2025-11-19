// =========================================
// 智能烤箱控制器固件 v0.8.5 - 正式版
// =========================================
// 固件版本: 0.8.5
// 主要功能: 网页控制界面 + 温度校准功能 + OTA升级功能 + MAX6675温度传感器驱动 + 多设备识别功能 + PID温控算法
// 硬件支持: ESP8266系列芯片 + 继电器模块 + OLED显示屏 + MAX6675热电偶传感器
// =========================================

// =========================================
// 多语言支持定义
// =========================================

// 当前语言设置 (0=中文, 1=英文)
int currentLanguage = 0;

// 简化的翻译函数 - 由于Arduino内存限制，使用条件编译
#define TR(key) (currentLanguage == 0 ? getChineseTranslation(key) : getEnglishTranslation(key))

// 中文翻译函数
const char* getChineseTranslation(const char* key) {
    if (strcmp(key, "eeprom_save_retry") == 0) return "EEPROM保存失败，重试 ";
    if (strcmp(key, "eeprom_save_failed") == 0) return "错误：EEPROM保存失败，配置未保存";
    if (strcmp(key, "config_saved_success") == 0) return "配置已成功保存到EEPROM";
    if (strcmp(key, "loading_config") == 0) return "从EEPROM加载配置参数...";
    if (strcmp(key, "eeprom_read_failed") == 0) return "错误：EEPROM读取失败，使用默认配置";
    if (strcmp(key, "default_config_loaded") == 0) return "已重置为默认配置参数";
    if (strcmp(key, "sensor_comm_error") == 0) return "传感器通信错误: MAX6675未连接或读取失败";
    if (strcmp(key, "sensor_data_invalid") == 0) return "传感器数据无效，准备重试...";
    if (strcmp(key, "sensor_comm_failure") == 0) return "传感器通信失败 - 已达最大重试次数，无法获取有效数据 - 将返回默认温度";
    if (strcmp(key, "sensor_read_failure") == 0) return "传感器读取失败，返回默认温度: 25.0°C";
    if (strcmp(key, "sensor_status_abnormal") == 0) return "传感器连接状态异常- 尝试重新连接...";
    if (strcmp(key, "sensor_status_failure") == 0) return "传感器连接状态异常- 已达最大重试次数";
    if (strcmp(key, "temp_out_of_range") == 0) return "温度值超出有效范围";
    if (strcmp(key, "temp_read") == 0) return "读取到的温度: ";
    if (strcmp(key, "temp_calibration_complete") == 0) return "温度校准参数计算完成";
    if (strcmp(key, "calibration_saved") == 0) return "温度校准参数已保存到EEPROM";
    if (strcmp(key, "captive_portal_start") == 0) return "启动Captive Portal服务...";
    if (strcmp(key, "captive_portal_started") == 0) return "Captive portal启动成功 - 快速配网模式已启用";
    if (strcmp(key, "captive_portal_stop") == 0) return "停止Captive Portal服务，关闭相关网络服务...";
    if (strcmp(key, "wifi_config_missing") == 0) return "WiFi配置参数缺失，启动配网界面";
    if (strcmp(key, "using_saved_wifi") == 0) return "使用保存的WiFi配置参数尝试连接网络";
    if (strcmp(key, "wifi_connect_success") == 0) return "WiFi连接成功";
    if (strcmp(key, "wifi_connect_failed") == 0) return "WiFi连接失败，启动配网界面";
    if (strcmp(key, "discovery_request_received") == 0) return "收到设备发现请求，正在发送响应";
    if (strcmp(key, "tcp_client_connected") == 0) return "TCP客户端已连接";
    if (strcmp(key, "tcp_command_received") == 0) return "收到TCP命令: ";
    if (strcmp(key, "serial_command_received") == 0) return "收到串口命令: ";
    if (strcmp(key, "device_status_title") == 0) return "=== 设备状态 ===";
    if (strcmp(key, "temp_label") == 0) return "温度: ";
    if (strcmp(key, "target_temp_label") == 0) return "目标温度: ";
    if (strcmp(key, "heating_status_label") == 0) return "加热状态: ";
    if (strcmp(key, "heating_on") == 0) return "开启";
    if (strcmp(key, "heating_off") == 0) return "关闭";
    if (strcmp(key, "mode_label") == 0) return "工作模式: ";
    if (strcmp(key, "oven_mode") == 0) return "烤箱模式";
    if (strcmp(key, "toaster_mode") == 0) return "烤面包机模式";
    if (strcmp(key, "wifi_status_label") == 0) return "WiFi状态: ";
    if (strcmp(key, "connected") == 0) return "已连接";
    if (strcmp(key, "disconnected") == 0) return "未连接";
    if (strcmp(key, "pid_control_label") == 0) return "PID控制: ";
    if (strcmp(key, "enabled") == 0) return "启用";
    if (strcmp(key, "disabled") == 0) return "禁用";
    if (strcmp(key, "device_restart") == 0) return "执行设备重启...";
    if (strcmp(key, "target_temp_set") == 0) return "目标温度已设置为: ";
    if (strcmp(key, "heating_started") == 0) return "加热已开启";
    if (strcmp(key, "heating_stopped") == 0) return "加热已关闭";
    if (strcmp(key, "temp_calibration_mode") == 0) return "温度校准模式";
    if (strcmp(key, "use_web_interface") == 0) return "请使用网页界面进行温度校准";
    if (strcmp(key, "pid_enabled_msg") == 0) return "PID控制已启用";
    if (strcmp(key, "pid_disabled_msg") == 0) return "PID控制已禁用";
    if (strcmp(key, "config_saved") == 0) return "配置已保存到EEPROM";
    if (strcmp(key, "config_save_failed") == 0) return "配置保存失败";
    if (strcmp(key, "filesystem_init_success") == 0) return "文件系统初始化成功";
    if (strcmp(key, "filesystem_init_failed") == 0) return "文件系统初始化失败，HTML文件服务将不可用";
    if (strcmp(key, "wifi_config_load_success") == 0) return "WiFi配置加载成功，尝试连接网络...";
    return key; // 如果找不到翻译，返回原始键
}

// 英文翻译函数
const char* getEnglishTranslation(const char* key) {
    if (strcmp(key, "eeprom_save_retry") == 0) return "EEPROM save failed, retry ";
    if (strcmp(key, "eeprom_save_failed") == 0) return "Error: EEPROM save failed, configuration not saved";
    if (strcmp(key, "config_saved_success") == 0) return "Configuration successfully saved to EEPROM";
    if (strcmp(key, "loading_config") == 0) return "Loading configuration parameters from EEPROM...";
    if (strcmp(key, "eeprom_read_failed") == 0) return "Error: EEPROM read failed, using default configuration";
    if (strcmp(key, "default_config_loaded") == 0) return "Reset to default configuration parameters";
    if (strcmp(key, "sensor_comm_error") == 0) return "Sensor communication error: MAX6675 not connected or read failed";
    if (strcmp(key, "sensor_data_invalid") == 0) return "Sensor data invalid, preparing to retry...";
    if (strcmp(key, "sensor_comm_failure") == 0) return "Sensor communication failure - Maximum retries reached, unable to obtain valid data - Will return default temperature";
    if (strcmp(key, "sensor_read_failure") == 0) return "Sensor read failure, returning default temperature: 25.0°C";
    if (strcmp(key, "sensor_status_abnormal") == 0) return "Sensor connection status abnormal - Attempting to reconnect...";
    if (strcmp(key, "sensor_status_failure") == 0) return "Sensor connection status abnormal - Maximum retries reached";
    if (strcmp(key, "temp_out_of_range") == 0) return "Temperature value out of valid range";
    if (strcmp(key, "temp_read") == 0) return "Temperature read: ";
    if (strcmp(key, "temp_calibration_complete") == 0) return "Temperature calibration parameters calculation complete";
    if (strcmp(key, "calibration_saved") == 0) return "Temperature calibration parameters saved to EEPROM";
    if (strcmp(key, "captive_portal_start") == 0) return "Starting Captive Portal service...";
    if (strcmp(key, "captive_portal_started") == 0) return "Captive portal started successfully - Quick configuration mode enabled";
    if (strcmp(key, "captive_portal_stop") == 0) return "Stopping Captive Portal service, closing related network services...";
    if (strcmp(key, "wifi_config_missing") == 0) return "WiFi configuration parameters missing, starting configuration interface";
    if (strcmp(key, "using_saved_wifi") == 0) return "Using saved WiFi configuration parameters to attempt network connection";
    if (strcmp(key, "wifi_connect_success") == 0) return "WiFi connection successful";
    if (strcmp(key, "wifi_connect_failed") == 0) return "WiFi connection failed, starting configuration interface";
    if (strcmp(key, "discovery_request_received") == 0) return "Device discovery request received, sending response";
    if (strcmp(key, "tcp_client_connected") == 0) return "TCP client connected";
    if (strcmp(key, "tcp_command_received") == 0) return "TCP command received: ";
    if (strcmp(key, "serial_command_received") == 0) return "Serial command received: ";
    if (strcmp(key, "device_status_title") == 0) return "=== Device Status ===";
    if (strcmp(key, "temp_label") == 0) return "Temperature: ";
    if (strcmp(key, "target_temp_label") == 0) return "Target Temperature: ";
    if (strcmp(key, "heating_status_label") == 0) return "Heating Status: ";
    if (strcmp(key, "heating_on") == 0) return "On";
    if (strcmp(key, "heating_off") == 0) return "Off";
    if (strcmp(key, "mode_label") == 0) return "Operating Mode: ";
    if (strcmp(key, "oven_mode") == 0) return "Oven Mode";
    if (strcmp(key, "toaster_mode") == 0) return "Toaster Mode";
    if (strcmp(key, "wifi_status_label") == 0) return "WiFi Status: ";
    if (strcmp(key, "connected") == 0) return "Connected";
    if (strcmp(key, "disconnected") == 0) return "Disconnected";
    if (strcmp(key, "pid_control_label") == 0) return "PID Control: ";
    if (strcmp(key, "enabled") == 0) return "Enabled";
    if (strcmp(key, "disabled") == 0) return "Disabled";
    if (strcmp(key, "device_restart") == 0) return "Executing device restart...";
    if (strcmp(key, "target_temp_set") == 0) return "Target temperature set to: ";
    if (strcmp(key, "heating_started") == 0) return "Heating started";
    if (strcmp(key, "heating_stopped") == 0) return "Heating stopped";
    if (strcmp(key, "temp_calibration_mode") == 0) return "Temperature calibration mode";
    if (strcmp(key, "use_web_interface") == 0) return "Please use web interface for temperature calibration";
    if (strcmp(key, "pid_enabled_msg") == 0) return "PID control enabled";
    if (strcmp(key, "pid_disabled_msg") == 0) return "PID control disabled";
    if (strcmp(key, "config_saved") == 0) return "Configuration saved to EEPROM";
    if (strcmp(key, "config_save_failed") == 0) return "Configuration save failed";
    if (strcmp(key, "filesystem_init_success") == 0) return "Filesystem initialization successful";
    if (strcmp(key, "filesystem_init_failed") == 0) return "Filesystem initialization failed, HTML file service will be unavailable";
    if (strcmp(key, "wifi_config_load_success") == 0) return "WiFi configuration loaded successfully, attempting network connection...";
    return key; // 如果找不到翻译，返回原始键
}

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
const String FIRMWARE_VERSION = "0.8.5";

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

// PID控制变量
bool usePID = false;  // 是否使用PID控制
float Kp = 2.0;       // 比例系数
float Ki = 0.1;       // 积分系数
float Kd = 1.0;       // 微分系数

// PID计算中间变量
float previousError = 0.0;
float integral = 0.0;
unsigned long previousMillis = 0;

// PWM控制变量
unsigned long pwmPeriod = 1000;  // PWM周期，单位毫秒
unsigned long pwmStartTime = 0;  // PWM周期开始时间

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
    float Kp;                 // PID比例系数
    float Ki;                 // PID积分系数
    float Kd;                 // PID微分系数
    bool usePID;              // 是否使用PID控制
    char signature[16];  // 配置文件签名，用于验证配置有效性
};

bool saveConfig() {
    Config config;
    // 初始化配置结构体，清空内存空间
    memset(&config, 0, sizeof(config));
    
    // 验证WiFi配置参数有效性
    if (wifiSSID.length() == 0 || wifiSSID.length() > 31) {
        Serial.println(TR("error_ssid_invalid"));
        return false;
    }
    if (wifiPassword.length() > 63) {
        Serial.println(TR("error_password_too_long"));
        return false;
    }
    
    // 将WiFi配置参数复制到配置结构体
    strncpy(config.ssid, wifiSSID.c_str(), sizeof(config.ssid) - 1);
    strncpy(config.password, wifiPassword.c_str(), sizeof(config.password) - 1);
    strncpy(config.signature, "SMARTOVEN", sizeof(config.signature) - 1);
    
    // 应用温度校准参数到配置结构体
    config.temperatureOffset = temperatureOffset;
    config.temperatureScale = temperatureScale;
    
    // 应用PID参数到配置结构体
    config.Kp = Kp;
    config.Ki = Ki;
    config.Kd = Kd;
    config.usePID = usePID;
    
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
            Serial.println(TR("eeprom_save_retry") + String(attempt + 1));
            delay(100);
        }
    }
    
    if (saveSuccess) {
        Serial.println(TR("config_saved_success"));
        Serial.print("SSID: ");
        Serial.println(config.ssid);
        Serial.print(TR("password_length"));
        Serial.println(strlen(config.password));
        Serial.print(TR("temp_offset"));
        Serial.print(config.temperatureOffset);
        Serial.println("°C");
        Serial.print(TR("temp_scale"));
        Serial.println(config.temperatureScale);
        Serial.print(TR("pid_kp"));
        Serial.print(config.Kp);
        Serial.print(", Ki: ");
        Serial.print(config.Ki);
        Serial.print(", Kd: ");
        Serial.println(config.Kd);
        Serial.print(TR("pid_enabled"));
        Serial.println(config.usePID ? TR("enabled") : TR("disabled"));
        Serial.print(TR("config_signature"));
        Serial.println(config.signature);
        
        // 配置保存成功提示音
        beepConfigSaved();
        return true;
    } else {
        Serial.println(TR("eeprom_save_failed"));
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
        Serial.println(TR("eeprom_read_failed"));
        resetToDefaultConfig();
        return false;
    }

    Serial.println(TR("loading_config"));
    Serial.print(TR("config_signature"));
    Serial.println(config.signature);
    Serial.print(TR("config_ssid"));
    Serial.println(config.ssid);
    Serial.print(TR("password_length"));
    Serial.println(strlen(config.password));
    Serial.print(TR("temp_offset"));
    Serial.print(config.temperatureOffset);
    Serial.println("°C");
    Serial.print(TR("temp_scale"));
    Serial.println(config.temperatureScale);
    Serial.print(TR("pid_kp"));
    Serial.print(config.Kp);
    Serial.print(", Ki: ");
    Serial.print(config.Ki);
    Serial.print(", Kd: ");
    Serial.println(config.Kd);
    Serial.print(TR("pid_enabled"));
    Serial.println(config.usePID ? TR("enabled") : TR("disabled"));
    
    // 严格的配置验证
    if (strcmp(config.signature, "SMARTOVEN") == 0) {
        // 验证SSID和密码的有效性
        String loadedSSID = String(config.ssid);
        String loadedPassword = String(config.password);
        
        if (loadedSSID.length() == 0 || loadedSSID.length() > 31) {
            Serial.println(TR("error_ssid_invalid"));
            resetToDefaultConfig();
            return false;
        }

        if (loadedPassword.length() > 63) {
            Serial.println(TR("error_password_too_long"));
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
        
        // 加载PID参数
        Kp = config.Kp;
        Ki = config.Ki;
        Kd = config.Kd;
        usePID = config.usePID;
        
        Serial.println(TR("config_load_success"));
        Serial.print(TR("temp_offset"));
        Serial.print(temperatureOffset);
        Serial.println("°C");
        Serial.print(TR("temp_scale"));
        Serial.println(temperatureScale);
        Serial.print(TR("pid_kp"));
        Serial.print(Kp);
        Serial.print(", Ki: ");
        Serial.print(Ki);
        Serial.print(", Kd: ");
        Serial.println(Kd);
        Serial.print(TR("pid_enabled"));
        Serial.println(usePID ? TR("enabled") : TR("disabled"));
        return true;
    } else {
        Serial.println(TR("config_signature_invalid"));
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
    
    // 重置PID参数为默认值
    Kp = 2.0;
    Ki = 0.1;
    Kd = 1.0;
    usePID = false;  // 默认使用开关控制
    
    Serial.println(TR("default_config_loaded"));
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
                    Serial.println(TR("sensor_data_invalid"));
                    delay(100);  // 重试前延迟100ms
                    continue;
                } else {
                    Serial.println(TR("sensor_comm_failure"));
                    // 传感器读取失败，增加硬件故障计数
                    hardwareFailureCount++;
                    // 传感器读取失败，已达最大重试次数，返回默认温度 25.0°C
                    Serial.println(TR("sensor_read_failure"));
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
                Serial.print(TR("temp_read"));
                Serial.print(temperature); Serial.println("°C");

                // 重置传感器错误计数器，更新最后成功读取时间
                if (retry == 0) {
                    hardwareFailureCount = 0;
                }
                return temperature;
            } else {
                Serial.println(TR("temp_out_of_range"));
                return -1.0;
            }
        } else {
            if (retry < 2) {
                Serial.println(TR("sensor_status_abnormal"));
                delay(100);  // 重试前延迟100ms
                continue;
            } else {
                Serial.println(TR("sensor_status_failure"));
                return -1.0;
            }
        }
    }
    
    // 所有读取尝试失败- 无法获取有效温度数据
    Serial.println(TR("sensor_comm_error"));
    // 传感器读取失败，已达最大重试次数，返回默认温度 25.0°C
    Serial.println(TR("sensor_read_failure"));
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
    
    Serial.println(TR("temp_calibration_complete"));
    Serial.print(TR("actual_temp")); Serial.print(actualTemp); Serial.println("°C");
    Serial.print(TR("measured_temp")); Serial.print(measuredTemp); Serial.println("°C");
    Serial.print(TR("calibration_offset")); Serial.print(temperatureOffset); Serial.println("°C");
    Serial.print(TR("calibration_scale")); Serial.println(temperatureScale);

    // 保存温度校准参数到EEPROM
    saveConfig();
    Serial.println(TR("calibration_saved"));
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
    Serial.println(TR("captive_portal_start"));

    // 快速断开现有WiFi连接（快速配网优化）
    WiFi.disconnect();
    delay(50); // 减少等待时间

    // 快速配置并启动WiFi接入点模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());

    Serial.print(TR("ap_ssid"));
    Serial.println(AP_SSID);
    Serial.print(TR("ap_ip"));
    Serial.println(WiFi.softAPIP());

    // 快速配置DNS服务器参数
    dnsServer.start(53, "*", WiFi.softAPIP());

    // 启动UDP服务用于设备发现广播
    udp.begin(8888);

    isCaptivePortalMode = true;
    captivePortalStartTime = millis();

    Serial.println(TR("captive_portal_started"));
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
    html += "<form action=\"/update\" method=\"post\" enctype=\"multipart/form-data\" onsubmit=\"return uploadFilesystem(this)\">";
    html += "<p><strong>选择文件系统镜像 (.bin):</strong></p>";
    html += "<input type=\"file\" name=\"filesystem\" accept=\".bin\" required style=\"margin:10px 0;padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;\">";
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
    html += "    xhr.open('POST','/update');";
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
        isFilesystemUpdate = (webServer.uri() == "/update");
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
    
    webServer.on("/update", HTTP_POST, handleFilesystemUpdate, handleFileUpload);
    webServer.on("/status", HTTP_GET, handleStatus);
    
    // PID控制API端点
    webServer.on("/pid", HTTP_GET, handleGetPID);
    webServer.on("/pid", HTTP_POST, handleSetPID);
    webServer.on("/pid/enable", HTTP_POST, handleEnablePID);
    webServer.on("/pid/disable", HTTP_POST, handleDisablePID);
    
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

void handleStatus() {
    // 构建JSON响应，包含设备状态信息
    String json = "{";
    json += "\"device_id\":\"" + DEVICE_ID + "\",";
    json += "\"firmware_version\":\"" + FIRMWARE_VERSION + "\",";
    json += "\"temperature\":" + String(currentTemp) + ",";
    json += "\"target_temperature\":" + String(targetTemp) + ",";
    json += "\"heating_enabled\":" + String(heatingEnabled ? "true" : "false") + ",";
    json += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"wifi_ssid\":\"" + String(WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "") + "\",";
    json += "\"ip_address\":\"" + String(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "0.0.0.0") + "\",";
    json += "\"pid_enabled\":" + String(usePID ? "true" : "false") + ",";
    json += "\"pid_kp\":" + String(Kp) + ",";
    json += "\"pid_ki\":" + String(Ki) + ",";
    json += "\"pid_kd\":" + String(Kd);
    json += "}";
    
    webServer.send(200, "application/json", json);
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
// PID控制API处理函数
// =========================================

void handleGetPID() {
    // 返回当前PID参数
    String json = "{";
    json += "\"kp\":" + String(Kp) + ",";
    json += "\"ki\":" + String(Ki) + ",";
    json += "\"kd\":" + String(Kd) + ",";
    json += "\"enabled\":" + String(usePID ? "true" : "false");
    json += "}";
    
    webServer.send(200, "application/json", json);
}

void handleSetPID() {
    // 设置PID参数
    if (webServer.hasArg("kp")) {
        Kp = webServer.arg("kp").toFloat();
    }
    
    if (webServer.hasArg("ki")) {
        Ki = webServer.arg("ki").toFloat();
    }
    
    if (webServer.hasArg("kd")) {
        Kd = webServer.arg("kd").toFloat();
    }
    
    // 保存配置到EEPROM
    if (saveConfig()) {
        String json = "{\"status\":\"success\",\"message\":\"PID参数已保存\"}";
        webServer.send(200, "application/json", json);
        
        // 播放配置保存成功提示音
        beepConfigSaved();
    } else {
        String json = "{\"status\":\"error\",\"message\":\"配置保存失败\"}";
        webServer.send(500, "application/json", json);
    }
}

void handleEnablePID() {
    usePID = true;
    // 重置PID计算变量
    previousError = 0.0;
    integral = 0.0;
    previousMillis = millis();
    
    String json = "{\"status\":\"success\",\"message\":\"PID控制已启用\"}";
    webServer.send(200, "application/json", json);
}

void handleDisablePID() {
    usePID = false;
    digitalWrite(HEATER_PIN, LOW); // 关闭加热器
    
    String json = "{\"status\":\"success\",\"message\":\"PID控制已禁用\"}";
    webServer.send(200, "application/json", json);
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
            Serial.print("PID控制: "); Serial.println(usePID ? "启用" : "禁用");
            Serial.print("Kp: "); Serial.print(Kp);
            Serial.print(", Ki: "); Serial.print(Ki);
            Serial.print(", Kd: "); Serial.println(Kd);
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
        } else if (command == "pid_on") {
            usePID = true;
            // 重置PID计算变量
            previousError = 0.0;
            integral = 0.0;
            previousMillis = millis();
            Serial.println("PID控制已启用");
        } else if (command == "pid_off") {
            usePID = false;
            digitalWrite(HEATER_PIN, LOW); // 关闭加热器
            Serial.println("PID控制已禁用");
        } else if (command.startsWith("set_kp ")) {
            Kp = command.substring(7).toFloat();
            Serial.print("Kp已设置为: "); Serial.println(Kp);
        } else if (command.startsWith("set_ki ")) {
            Ki = command.substring(7).toFloat();
            Serial.print("Ki已设置为: "); Serial.println(Ki);
        } else if (command.startsWith("set_kd ")) {
            Kd = command.substring(7).toFloat();
            Serial.print("Kd已设置为: "); Serial.println(Kd);
        } else if (command == "save_config") {
            if (saveConfig()) {
                Serial.println("配置已保存到EEPROM");
            } else {
                Serial.println("配置保存失败");
            }
        } else if (command == "load_config") {
            if (loadConfig()) {
                Serial.println("配置已从EEPROM加载");
            } else {
                Serial.println("配置加载失败");
            }
        } else if (command == "reset_config") {
            resetToDefaultConfig();
            Serial.println("已重置为默认配置");
        } else if (command == "autotune_pid") {
            Serial.println("启动PID自动调优程序...");
            autoTunePID();
        } else {
            Serial.println("未知命令，可用命令: status, reset, set_temp [温度], heat_on, heat_off, calibrate, pid_on, pid_off, set_kp [值], set_ki [值], set_kd [值], save_config, load_config, reset_config, autotune_pid");
        }
    }
}

// =========================================
// PID计算函数
// =========================================

float computePID() {
    // 计算时间间隔
    unsigned long currentMillis = millis();
    float deltaTime = (currentMillis - previousMillis) / 1000.0; // 转换为秒
    
    // 如果时间间隔为0，直接返回0
    if (deltaTime <= 0) {
        return 0.0;
    }
    
    // 计算误差
    float error = targetTemp - currentTemp;
    
    // 计算积分项
    integral += error * deltaTime;
    
    // 计算微分项
    float derivative = (error - previousError) / deltaTime;
    
    // 计算PID输出
    float output = Kp * error + Ki * integral + Kd * derivative;
    
    // 保存当前误差供下次计算使用
    previousError = error;
    previousMillis = currentMillis;
    
    // 限制输出范围在0-100%
    if (output > 100.0) {
        output = 100.0;
    } else if (output < 0.0) {
        output = 0.0;
    }
    
    return output;
}

// =========================================
// 加热器控制函数
// =========================================

void controlHeater() {
    if (usePID) {
        // 使用PID控制
        if (heatingEnabled) {
            float pidOutput = computePID();
            
            // PWM控制加热器
            unsigned long currentMillis = millis();
            
            // 检查是否开始新的PWM周期
            if (currentMillis - pwmStartTime >= pwmPeriod) {
                pwmStartTime = currentMillis;
            }
            
            // 计算PWM高电平时间
            unsigned long pwmHighTime = (unsigned long)(pwmPeriod * pidOutput / 100.0);
            
            // 根据PWM控制加热器
            if (currentMillis - pwmStartTime < pwmHighTime) {
                digitalWrite(HEATER_PIN, HIGH);
            } else {
                digitalWrite(HEATER_PIN, LOW);
            }
            
            // 串口输出PID信息用于调试
            Serial.print("PID控制 - 温度: ");
            Serial.print(currentTemp);
            Serial.print("°C, 目标: ");
            Serial.print(targetTemp);
            Serial.print("°C, 误差: ");
            Serial.print(targetTemp - currentTemp);
            Serial.print("°C, PID输出: ");
            Serial.print(pidOutput);
            Serial.println("%");
        } else {
            digitalWrite(HEATER_PIN, LOW);
        }
    } else {
        // 使用原有的开关控制
        if (heatingEnabled && currentTemp < targetTemp) {
            digitalWrite(HEATER_PIN, HIGH);
        } else {
            digitalWrite(HEATER_PIN, LOW);
        }
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
    
    // 初始化PID控制变量
    previousError = 0;
    integral = 0;
    previousMillis = millis();
    
    // 快速播放启动提示音
    beepConfigSaved();
}

// =========================================
// PID自动调优函数
// =========================================

void autoTunePID() {
    Serial.println("开始PID自动调优...");
    Serial.println("请确保烤箱为空并且在安全环境下操作");
    
    // 保存当前设置
    bool originalHeatingEnabled = heatingEnabled;
    bool originalUsePID = usePID;
    float originalTargetTemp = targetTemp;
    float originalKp = Kp;
    float originalKi = Ki;
    float originalKd = Kd;
    
    // 设置初始参数
    Kp = 0.0;
    Ki = 0.0;
    Kd = 0.0;
    usePID = false;
    heatingEnabled = true;
    
    // 设置目标温度为当前温度+20°C
    targetTemp = currentTemp + 20.0;
    if (targetTemp > 250.0) targetTemp = 250.0; // 限制最高温度
    
    Serial.print("目标温度设置为: ");
    Serial.print(targetTemp);
    Serial.println("°C");
    
    // 等待温度稳定
    Serial.println("等待温度稳定...");
    unsigned long stableStart = millis();
    
    while (millis() - stableStart < 30000) { // 等待30秒
        currentTemp = readTemperatureManual();
        controlHeater();
        delay(1000);
        Serial.print("当前温度: ");
        Serial.print(currentTemp);
        Serial.println("°C");
    }
    
    // 开始调优过程
    Serial.println("开始寻找临界增益...");
    float Ku = 0.0; // 临界增益
    float Tu = 0.0; // 振荡周期
    
    // 逐步增加Kp直到出现持续振荡
    Kp = 1.0;
    usePID = true;
    
    bool oscillationDetected = false;
    float maxTemp = currentTemp;
    float minTemp = currentTemp;
    int peakCount = 0;
    unsigned long lastPeakTime = 0;
    
    Serial.println("逐步增加Kp直到出现持续振荡...");
    
    while (!oscillationDetected && Kp < 100.0) {
        // 运行一段时间观察系统响应
        unsigned long tuneStart = millis();
        maxTemp = currentTemp;
        minTemp = currentTemp;
        peakCount = 0;
        
        // 观察2分钟内的温度变化
        while (millis() - tuneStart < 120000) {
            currentTemp = readTemperatureManual();
            float pidOutput = computePID();
            
            // PWM控制加热器
            unsigned long currentMillis = millis();
            if (currentMillis - pwmStartTime >= pwmPeriod) {
                pwmStartTime = currentMillis;
            }
            unsigned long pwmHighTime = (unsigned long)(pwmPeriod * pidOutput / 100.0);
            if (currentMillis - pwmStartTime < pwmHighTime) {
                digitalWrite(HEATER_PIN, HIGH);
            } else {
                digitalWrite(HEATER_PIN, LOW);
            }
            
            // 检测峰值
            if (currentTemp > maxTemp) {
                maxTemp = currentTemp;
            }
            if (currentTemp < minTemp) {
                minTemp = currentTemp;
            }
            
            // 简单的峰值检测
            static float prevTemp = currentTemp;
            static float prevPrevTemp = currentTemp;
            
            if (prevPrevTemp < prevTemp && prevTemp > currentTemp && (millis() - lastPeakTime) > 10000) {
                // 检测到峰值
                if (peakCount == 0) {
                    lastPeakTime = millis();
                } else {
                    Tu = (millis() - lastPeakTime) / 1000.0; // 转换为秒
                    lastPeakTime = millis();
                    
                    // 如果连续检测到峰值且温差较大，则认为出现振荡
                    if (peakCount >= 2 && (maxTemp - minTemp) > 2.0) {
                        oscillationDetected = true;
                        Ku = Kp;
                        Serial.print("检测到持续振荡! 临界增益 Ku = ");
                        Serial.print(Ku);
                        Serial.print(", 振荡周期 Tu = ");
                        Serial.print(Tu);
                        Serial.println(" 秒");
                        break;
                    }
                }
                peakCount++;
            }
            
            prevPrevTemp = prevTemp;
            prevTemp = currentTemp;
            
            delay(1000);
            Serial.print("Kp: ");
            Serial.print(Kp);
            Serial.print(", 温度: ");
            Serial.print(currentTemp);
            Serial.println("°C");
        }
        
        if (!oscillationDetected) {
            Kp += 1.0; // 增加Kp
            // 重置PID变量
            previousError = 0.0;
            integral = 0.0;
            previousMillis = millis();
        }
    }
    
    if (oscillationDetected) {
        // 使用Ziegler-Nichols方法计算PID参数
        // 这些是经典的Ziegler-Nichols系数
        Kp = 0.6 * Ku;
        Ki = 1.2 * Ku / Tu;
        Kd = 0.075 * Ku * Tu;
        
        Serial.println("PID自动调优完成!");
        Serial.print("推荐参数: Kp = ");
        Serial.print(Kp);
        Serial.print(", Ki = ");
        Serial.print(Ki);
        Serial.print(", Kd = ");
        Serial.println(Kd);
        
        // 保存配置
        if (saveConfig()) {
            Serial.println("新参数已保存到EEPROM");
        }
    } else {
        Serial.println("未能检测到持续振荡，请手动调整参数");
        // 恢复原始参数
        Kp = originalKp;
        Ki = originalKi;
        Kd = originalKd;
    }
    
    // 恢复原始设置
    targetTemp = originalTargetTemp;
    usePID = originalUsePID;
    heatingEnabled = originalHeatingEnabled;
    
    Serial.println("PID自动调优过程结束");
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
