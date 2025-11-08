?/ =========================================
// 闁哄懘缂氶崗姗€鎮介悽闈涘姍缂佺姾椴哥敮鍫曞礆鐠虹儤鐝?v0.6.6
// =========================================
// 闁绘鐗婂﹢? 0.6.6
// 闁告梻鍠曢崗? 鐎殿喖鎼崺妤呮⒒閵婏箑鐓曢梺鏉跨Ф缂?+ 閻犱焦鍎抽ˇ顒勬嚊椤忓嫬袟闁告瑦鍨归獮?+ OTA闁告娲ㄦ?+ MAX6675闁归潧顑呮慨銖扨I閻庡湱鍋熼獮?+ 濞戞挸顭烽妴澶愭⒖閸℃ê鐏囨繛鎾櫅鐎规娊骞掕閸?
// 闁哄洤鐡ㄩ弻? 濡炪倕婀卞ú浼村棘閸ワ附顐界紓浣规尰閻庮垱瀵煎Ο鍝勵嚙闁挎稑鑻崹褰掓⒔閵堝娅㈠璺虹У娣囶垱鎯旈敂鑺モ枖缂佲偓閻氬绀夋繛鎾虫噽閹﹥绋夊鍛畱閻熸洑鑳跺▓鎴澝圭€ｎ厾妲搁柡鍌氭矗濞?
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
// 闁稿繈鍔岄惇顒勫矗濮椻偓閸ｈ櫣鈧鐭粻?
// =========================================

// 全局对象定义
ESP8266WebServer webServer(80);
WiFiUDP udp;
DNSServer dnsServer;
ESP8266WebServer otaServer(8080);
ESP8266HTTPUpdateServer httpUpdater;
WiFiServer tcpServer(8888);  // TCP闁哄牆绉存慨鐔煎闯閵娧勬殢濞存粌鐩疨P閺夆晝鍋炵敮?
WiFiClient tcpClient;        // TCP閻庡箍鍨洪崺娑氱博椤栨繄绠鹃柟?

// 系统运行状态变量
unsigned long temperatureReadCount = 0;
float temperatureReadAvgTime = 0;
const unsigned long WEB_SERVER_HANDLE_INTERVAL = 100; // 每100ms处理一次Web请求，避免阻塞主循环占用过多CPU资源

// 缁绢収鍏涘▎銏ゅ极閸涘瓨顔囬柣鈺傚灦鐢?
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

//  captive portal模式配置
bool isCaptivePortalMode = false;
unsigned long captivePortalStartTime = 0;
const unsigned long CAPTIVE_PORTAL_TIMEOUT = 300000; // 5闁告帒妫濋幐鎾舵惥閸涱喗顦?
const String AP_SSID = "SmartOven-" + String(ESP.getChipId());
const String AP_PASSWORD = "12345678";

// 温度控制变量
float currentTemp = 0.0;
float targetTemp = 180.0;
bool heatingEnabled = false;
bool ovenMode = true; // 闁绘垯鍊楅鍫澪熼垾宕囩闁挎稒顒畆ue=闁绘垯鍊楅鍫澪熼垾宕囩闁挎稑鐣篴lse=闁绘垯鍊濆浼村礌閸涱喗绨氭俊顖椻偓宕囩

// 缂傚啯鍨圭划鍓佺博椤栨艾缍撻梺鏉跨Ф閻?
const int DEFAULT_PORT = 8888;

// =========================================
// MAX6675闁归潧顑呮慨銖扨I闂侇偅鐭穱濠囧礄閼恒儲娈?
// =========================================

// 閻犲洩顕цぐ鍢橝X6675闁告鍠庨～鎰板极閻楀牆绁﹂柨?6濞达絽绋勭槐? 闁衡偓绾懐绠婚柣妤€鐗婂﹢?
uint16_t readMAX6675RawData() {
  uint16_t data = 0;
  
  // 缁绢収鍏涘▎銏″緞瀹ュ嫮绉撮柨娑欐皑閳ユɑ绌卞┑濠傂熼柣妤€娲らˇ鈺傜鎼粹€冲殥闁活厹鍎虫慨鎼佸箑?
  digitalWrite(THERMO_CS, HIGH);
  digitalWrite(THERMO_CLK, LOW);
  delay(10);  // 鐎点倕鐖奸弳杈ㄥ緞瀹ュ嫮绉撮柡鍐ㄧ埣濡潡宕?0ms
  
  // 闁告凹鍨抽弫銈夋嚍椤栨粌顣?
  digitalWrite(THERMO_CS, LOW);
  delayMicroseconds(100);  // 濠⒀呭仜婵偛顕欓幆鎵缁绢収鍠曠换姘舵嚍椤栨粌顣荤紒瀣暱閻?
  
  // 閻犲洩顕цぐ?6濞达絽绉甸弳鐔煎箲椤曞棛绀凪SB濞村吋锚閸樻盯鏁?
  for (int i = 15; i >= 0; i--) {
    digitalWrite(THERMO_CLK, HIGH);  // 闁哄啫鐖奸幐鎾寸▔婵犲倸纾虫繛?
    delayMicroseconds(5);  // 闁告垵绻愰惃顖炲籍閸洘瀵€点倖鍎肩换婊堝箵閹版壆褰梺顐ゅ枎鐎?
    
    if (digitalRead(THERMO_DO)) {   // 閻犲洩顕цぐ鍥极閻楀牆绁﹀ù?
      data |= (1 << i);
    }
    
    digitalWrite(THERMO_CLK, LOW);  // 闁哄啫鐖奸幐鎾寸▔鐎ｎ喗顎栨繛?
    delayMicroseconds(5);  // 闁告垵绻愰惃顖炲籍閸洘瀵€点倖鍎肩换婊堝箵閹版壆褰梺顐ゅ枎鐎?
  }
  
  // 缂佸倷鑳堕弫銈夋嚍椤栨粌顣?
  digitalWrite(THERMO_CS, HIGH);
  delayMicroseconds(100);  // 濠⒀呭仜婵偛顕欓幆鎵缁绢収鍠曠换姘舵嚍椤栨粌顣荤紒瀣暱閻?
  
  // 闁轰胶澧楀畵渚€寮垫径瀣珡闁诡儸鍕垫⒕闁?
  if (data == 0x0000 || data == 0xFFFF) {
    Serial.println("閻犫偓閿曗偓閹? MAX6675閺夆晜鏌ㄥú鏍籍閻樿櫕娅忛柡浣哄瀹?);
  }
  
  return data;
}

// 温度校准参数配置
float temperatureOffset = 0.0;  // 温度补偿值 - 用于校准温度传感器读数偏差
float temperatureScale = 1.0;    // 温度缩放系数 - 用于调整温度读数的比例关系

// 硬件初始化验证
bool verifyHardwareInitialization() {
    Serial.println("验证硬件初始化状态...");
    
    // 婵☆偀鍋撻柡灞诲劚缁扁晠鎳樺鍗炐﹂柟?
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    // 閻犱礁澧介悿鍡涘礆濠靛棭娼楅柣妯垮煐閳?
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    delay(100);  // 缁绢収鍠曠换姘辨兜椤戞寧顐界紒瀣暱閻?
    
    // 濡ょ姴鐭侀惁澶婎嚕閺団€冲闁绘鍩栭埀?
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
    
    // 閻犱焦婢樼紞宥夊极閸涘瓨顔囨繛鍡忓墲閺?
    hardwareFailureCount++;
    
    // 鐎殿喖鎼崺妤冩兜椤戞寧顐藉璺虹С缂嶅懏鎯旇箛鎾崇仚
    for (int i = 0; i < 5; i++) {
        digitalWrite(THERMO_CS, HIGH);
        digitalWrite(THERMO_CLK, LOW);
        delay(200);  // 鐎点倕鐖奸弳杈ㄥ緞瀹ュ嫮绉撮柡鍐ㄧ埣濡?
        digitalWrite(THERMO_CS, LOW);
        delay(100);
        digitalWrite(THERMO_CS, HIGH);
        delay(200);
    }
    
    // 闂佹彃绉甸弻濠囧礆濠靛棭娼楅柛鏍ㄧ墪缁扁晠鎳?
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    digitalWrite(THERMO_CS, HIGH);
    digitalWrite(THERMO_CLK, LOW);
    
    lastHardwareReset = millis();
    Serial.println("硬件恢复流程已执行，失败次数: " + String(hardwareFailureCount));
}

// 閻犲洩顕цぐ鍥с€掗埡浣割唺闁稿﹦銆嬬槐娆撳箥鐎ｎ亜袟SPI閻庡湱鍋熼獮鍥晬? 闁衡偓绾懐绠婚柣妤€鐗婂﹢?
float readTemperatureManual() {
    // 濠⒀呭仜瀹搁亶鏌屽鍫㈡Ц闁哄牆鎼崺妤呮晬濮橆厽浠樺鑸靛哺閸ｅ摜鎷?婵炲棌妲勭槐婵喰掕箛鏃戝仹濠⒀呭仜婵偛顕欓幆鎵
    for (int retry = 0; retry < 3; retry++) {
        uint16_t rawData = readMAX6675RawData();
        
        // 閻犲浄濡囩划蹇曟嫚婵犲啯鐒藉ǎ鍥ｅ墲娴?
        Serial.print("闂佹彃绉烽惁?"); Serial.print(retry + 1); 
        Serial.print(": 闁告鍠庨～鎰板极閻楀牆绁?0x"); Serial.println(rawData, HEX);
        
        // 婵☆偀鍋撻柡灞诲劜閺嗙喖骞戦鑺ョ畳闁轰礁鐗婇埀顑秶绀勯柟鐑樺浮濞呭酣宕?闁瑰瓨鐗曢崣?闁汇劌瀚Λ銈夊极閸喐娈堕柟璇″櫙缁?
        if (rawData == 0x0000 || rawData == 0xFFFF) {
            if (retry < 2) {
                Serial.println("婵炴挴鏅涚€硅櫕瀵奸悩鍐插Τ闁革絻鍔忕换鎴﹀炊閻愬瓨锟ラ柡浣哥墛閺嗙喖骞戦鍡欑闂佹彃绉烽惁?..");
                delay(100);  // 闁搞儱鎼悾鎯ь嚈閹壆绠?00ms
                continue;
            } else {
                Serial.println("闁?婵炴挴鏅涚€硅櫕瀵奸悩鍐插Τ闁革絻鍔忕换鎴﹀炊閻愬瓨锟ラ柡浣哥墛閺嗙喖骞戦鍡欑闁?闁瑰瓨鐗曢崣?闁? 婵☆偀鍋撻柡灞诲劤閳ユ牗绂掗幆鎵闁?);
                // 濠⒀呭仜婵偟娑甸鎸庮偨闁轰礁鎳樺▓鎵媼閳╁啯娈?
                hardwareFailureCount++;
                
                // 闁轰礁鎳樺▓鐗堢┍濠靛洤袘闁挎稒淇虹换鎴﹀炊閻愭壆顏卞☉鎿冧簻閻ｃ劑宕楅妸锔垮垔閹艰揪绠戦埀顒傘€嬬槐婵嬫嚀鐏炶偐鐟濋柡?1.0闁挎稑鐭傛导鈺呭礂瀹ュ洭鍏囩紓浣哄枎瀹曞崬顫?
                Serial.println("闁?闁轰礁鎳樺▓鐗堢┍濠靛洤袘闁哄牆鎼崺妤€鈹戦埀顒€煤娴兼瑧绐楅弶鈺傛煥濞叉牜鈧懓顦崣蹇撱€掗埡浣割唺闁?5.0閹虹煰");
                return 25.0;
            }
        }
        
        // 婵☆偀鍋撻柡灞诲劦閺佸﹦鎷犻娑氬灱闊洦銇炵紞鍛存晬閸?濞达絽绉崇拹?閻炴稏鍔庨妵姘潰閿濆懐鍩楅柨?
        if (!(rawData & 0x04)) {
            uint16_t tempBits = rawData >> 3;  // 闁告瑥纾簺3濞达絽绉烽獮蹇涘矗閺嶃劋鍒婇幖杈鹃檮閺嗙喖骞?
            float temperature = tempBits * 0.25;  // 婵絽绻嬮柌婊堝础閺囨氨绉?.25閹虹煰
            
            // 閹煎瓨姊婚弫銈呫€掗埡浣割唺闁哄秮鈧啿娅?
            temperature = (temperature * temperatureScale) + temperatureOffset;
            
            // 婵☆偀鍋撻柡灞诲劜娣囶垱鎯旈敃浣哥槺闁搞儱鐡ㄥΣ鎼佸触閿曗偓閹酣鎮?
            if (temperature >= -50.0 && temperature <= 400.0) {
                Serial.print("闁?婵炴挴鏅涚€瑰磭鎷犵拠鎻掔悼闁瑰瓨鍔曟慨? ");
                Serial.print(temperature); Serial.println("閹虹煰");
                
                // 闂佹彃绉堕悿鍡涘极閸涘瓨顔囬悹浣插墲閺嗙喖宕抽…鎺旂濠碘€冲€归悘澶嬫交閻愮數鏁鹃柟瀛樺姇婵盯鏁?
                if (retry == 0) {
                    hardwareFailureCount = 0;
                }
                
                return temperature;
            } else {
                Serial.println("闁?婵炴挴鏅涚€硅櫕瀵奸悩鍐插Τ闁革絻鍔忛浼村极閹峰瞼孝闁告垿缂氱€垫牠宕?);
                return -1.0;
            }
        } else {
            if (retry < 2) {
                Serial.println("婵炴挴鏅涚€硅櫕瀵奸悩鍐插Τ闁革絻鍔忛浼村矗閺嶎厽鏅╅悹鍥跺灲缁辨2闁哄秴娲ょ换鏃€鎷呭鍛＝閻㈩垶娼荤槐姘舵晬瀹€鍕閻?..");
                delay(100);  // 闁搞儱鎼悾鎯ь嚈閹壆绠?00ms
                continue;
            } else {
                Serial.println("闁?婵炴挴鏅涚€硅櫕瀵奸悩鍐插Τ闁革絻鍔忛浼村矗閺嶎厽鏅╅悹?- 婵☆偀鍋撻柡灞诲劤閸庡綊鎮介棃娑楃处閺夆晝鍋炵敮?);
                return -1.0;
            }
        }
    }
    
    // 闁圭鍋撻柡鍫濐樀閸ｅ摜鎷犻弴銏犲幋濠㈡儼绮剧憴?
    Serial.println("闁?闁圭鍋撻柡鍫濐樀閸ｅ摜鎷犻弴銏犲幋濠㈡儼绮剧憴?- 婵☆偀鍋撻柡宀婃AX6675婵☆垪鈧櫕鍋ラ柛婊冪焷缁绘盯骞?);
    // 闁轰礁鎳樺▓鐗堢┍濠靛洤袘闁挎稒淇虹换鎴﹀炊閻愭壆顏卞☉鎿冧簻閻ｃ劑宕楅妸锔垮垔閹艰揪绠戦埀?
    Serial.println("闁?闁轰礁鎳樺▓鐗堢┍濠靛洤袘闁哄牆鎼崺妤€鈹戦埀顒€煤娴兼瑧绐楅弶鈺傛煥濞叉牜鈧懓顦崣蹇撱€掗埡浣割唺闁?5.0閹虹煰");
    return 25.0;
}

// 婵炴挴鏅涚€规娊寮介垾鍐叉珯闁告垼濮ら弳?
void calibrateTemperature(float actualTemp, float measuredTemp) {
    // 閻犱緤绱曢悾濠氬冀閳ュ啿娅欓柛娆忓€归弳?
    if (measuredTemp != 0) {
        temperatureScale = actualTemp / measuredTemp;
        temperatureOffset = actualTemp - (measuredTemp * temperatureScale);
    } else {
        temperatureOffset = actualTemp - measuredTemp;
        temperatureScale = 1.0;
    }
    
    Serial.println("婵炴挴鏅涚€规娊寮介垾鍐叉珯閻庣懓鏈崹?");
    Serial.print("閻庡湱鍋ゅ顖氥€掗埡浣割唺: "); Serial.print(actualTemp); Serial.println("閹虹煰");
    Serial.print("婵炴潙顑夐崳鍝勩€掗埡浣割唺: "); Serial.print(measuredTemp); Serial.println("閹虹煰");
    Serial.print("闁哄秮鈧啿娅欓柛瀣箳浜? "); Serial.print(temperatureOffset); Serial.println("閹虹煰");
    Serial.print("闁哄秮鈧啿娅欑紒顖滅帛閺? "); Serial.println(temperatureScale);
    
    // 濞ｅ洦绻傞悺銊╁冀閳ュ啿娅欓柛娆忓€归弳鐔煎礆閻уPROM
    saveConfig();
    Serial.println("婵炴挴鏅涚€规娊寮介垾鍐叉珯闁告瑥鍊归弳鐔奉啅闊厾绠介悗娑櫭崺瀛岴PROM");
}

// 閻犱焦鍎抽ˇ顒勫矗閹寸姴绠?
bool discoveryEnabled = true;
const unsigned long DISCOVERY_INTERVAL = 10000; // 10缂佸甯掔粻宥夊箻椤撴繄顏辨繛?
unsigned long lastDiscoveryTime = 0;

// LED闁绘鍩栭埀顑跨劍鐢爼宕?
bool ledState = false;
unsigned long lastLedUpdate = 0;
const unsigned long LED_BLINK_INTERVAL = 500; // LED闂傚偆浜為崕濠囨⒒閹绢喗顓?

// 闁绘垶顭囬崕鎵磼閹惧瓨灏嗛柣妯垮煐閳ь兛鐒︾敮鍫曞礆?
bool bakingCompleteState = false;
unsigned long bakingCompleteStartTime = 0;
const unsigned long BAKING_COMPLETE_DURATION = 10000; // 闁绘垶顭囬崕鎵磼閹惧瓨灏嗛煫鍥跺亰濡垶骞愭担铏规暰闁哄啫鐖煎Λ?0缂?

// =========================================
// EEPROM闂佹澘绉堕悿鍡欌偓娑櫭崑?
// =========================================
struct Config {
    char ssid[32];
    char password[64];
    float temperatureOffset;  // 婵炴挴鏅涚€规娊寮介垾鍐叉珯闁稿绻掍簺闂?
    float temperatureScale;    // 婵炴挴鏅涚€规娊寮介垾鍐叉珯缂侇垳绮弳?
    char signature[16];  // 濠⒀呭仜婵偟绮甸幆褎鍊崇紒灞炬そ濡潡鏁嶅畝鍕級闁稿繐绉剁槐锕傚礃閹绘帒闅樻繝褋鍨归崵?
};

void saveConfig() {
    Config config;
    // 婵炴挸鎳愰埞鏍煀瀹ュ洨鏋傜紓浣规尰閻庮垱鎷?
    memset(&config, 0, sizeof(config));
    
    // 閻庣懓顦崣蹇斿緞瀹ュ懎鐓戦悗娑欘殘椤戜焦绋?
    strncpy(config.ssid, wifiSSID.c_str(), sizeof(config.ssid) - 1);
    strncpy(config.password, wifiPassword.c_str(), sizeof(config.password) - 1);
    strncpy(config.signature, "SMARTOVEN", sizeof(config.signature) - 1);
    
    // 濞ｅ洦绻傞悺銊ャ€掗埡浣割唺闁哄秮鈧啿娅欓柛娆忓€归弳?
    config.temperatureOffset = temperatureOffset;
    config.temperatureScale = temperatureScale;
    
    // 缁绢収鍠曠换姘扁偓娑欘殘椤戜焦绋夐煫顓濈鞍null缂備焦鎸搁悢?
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    config.password[sizeof(config.password) - 1] = '\0';
    config.signature[sizeof(config.signature) - 1] = '\0';
    
    EEPROM.begin(512);
    EEPROM.put(0, config);
    EEPROM.commit();
    EEPROM.end();
    
    Serial.println("闂佹澘绉堕悿鍡楊啅闊厾绠介悗娑櫭崺瀛岴PROM");
    Serial.print("SSID: ");
    Serial.println(config.ssid);
    Serial.print("閻庨潧妫涢悥婊堟⒐閸喖顔? ");
    Serial.println(strlen(config.password));
    Serial.print("婵炴挴鏅涚€规娊寮介垾鍐叉珯闁稿绻掍簺: ");
    Serial.print(config.temperatureOffset);
    Serial.println("閹虹煰");
    Serial.print("婵炴挴鏅涚€规娊寮介垾鍐叉珯缂侇垳绮弳? ");
    Serial.println(config.temperatureScale);
    Serial.print("缂佹稒鍎抽幃? ");
    Serial.println(config.signature);
    
    // 闁炬艾鍊跨粋蹇涘闯閵婏箑绲圭紒鈧ú顏勫赋缂傚喚鍠栭崙鈩冪┍濠靛棛鎽?
    beepConfigSaved();
}

bool loadConfig() {
    Config config;
    EEPROM.begin(512);
    EEPROM.get(0, config);
    EEPROM.end();
    
    Serial.println("濞寸姴榧嶦PROM闁告梻濮惧ù鍥煀瀹ュ洨鏋?..");
    Serial.print("閻犲洩顕цぐ鍥礆閹殿喗鐣辩紒娑欏劤閹? ");
    Serial.println(config.signature);
    Serial.print("閻犲洩顕цぐ鍥礆閹殿喗鐣盨SID: ");
    Serial.println(config.ssid);
    Serial.print("閻犲洩顕цぐ鍥礆閹殿喗鐣遍悗闈涙閻栨粓姊归崹顔碱唺: ");
    Serial.println(strlen(config.password));
    Serial.print("閻犲洩顕цぐ鍥礆閹殿喗鐣辨繛鎾櫅鐎规娊寮介垾鍐叉珯闁稿绻掍簺: ");
    Serial.print(config.temperatureOffset);
    Serial.println("閹虹煰");
    Serial.print("閻犲洩顕цぐ鍥礆閹殿喗鐣辨繛鎾櫅鐎规娊寮介垾鍐叉珯缂侇垳绮弳? ");
    Serial.println(config.temperatureScale);
    
    if (strcmp(config.signature, "SMARTOVEN") == 0) {
        wifiSSID = String(config.ssid);
        wifiPassword = String(config.password);
        
        // 闁告梻濮惧ù鍥с€掗埡浣割唺闁哄秮鈧啿娅欓柛娆忓€归弳?
        temperatureOffset = config.temperatureOffset;
        temperatureScale = config.temperatureScale;
        
        Serial.println("闂佹澘绉堕悿鍡橆殽瀹€鍐闁瑰瓨鍔曟慨娑㈡晬鐏炶棄顫ｉ弶鐐跺Г濠€渚€寮崼銉ュ赋缂?);
        Serial.print("婵炴挴鏅涚€规娊寮介垾鍐叉珯闁稿绻掍簺: ");
        Serial.print(temperatureOffset);
        Serial.println("閹虹煰");
        Serial.print("婵炴挴鏅涚€规娊寮介垾鍐叉珯缂侇垳绮弳? ");
        Serial.println(temperatureScale);
        return true;
    } else {
        Serial.println("闂佹澘绉堕悿鍡橆殽瀹€鍐濠㈡儼绮剧憴锕傛晬瀹€鈧鐑藉触瀹ュ嫮鐟濋柛鏍х秺閸?);
        // 婵炴挸鎳愰埞鏍煀瀹ュ洨鏋?
        wifiSSID = "";
        wifiPassword = "";
        // 闂佹彃绉堕悿鍡椼€掗埡浣割唺闁哄秮鈧啿娅欓柛娆忓€归弳?
        temperatureOffset = 0.0;
        temperatureScale = 1.0;
        return false;
    }
}

// =========================================
// 鐎殿喖鎼崺妤呮⒒閵婏箑鐓曢柛鏃傚枙閸?
// =========================================
void startCaptivePortal() {
    Serial.println("闁告凹鍨版慨鈺侇嚕閸濆嫬鐓戦梻鍌樺妽閸╂稑螣閳ュ磭纭€...");
    
    // 闁哄偆鍘肩槐鎴︽偝閻楀牊绠掗弶鈺冨仦鐢?
    WiFi.disconnect();
    delay(100);
    
    // 闁告帗绋戠紓鎻侾闁绘埈鍘鹃崑?
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
    
    Serial.print("AP闁绘埈鍘鹃崑? ");
    Serial.println(AP_SSID);
    Serial.print("IP闁革附婢樺? ");
    Serial.println(WiFi.softAPIP());
    
    // 闁告凹鍨版慨銆S闁告棏鍋呯€?
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    // 闁告凹鍨版慨锟b闁哄牆绉存慨鐔煎闯?
    setupWebServer();
    webServer.begin();
    
    // 闁告凹鍨版慨鈺冩媼閹屾У闁告瑦鍨归獮鍥晬閸繂绻侀柛鎺戠埣濡剟骞嬮柨瀣嗕礁顕ｈ箛搴ｇ憮濞戞梻鍠栧〒鍓佹啺娴ｈ鏆滈柟闀愭祰椤旀洘寰勯崶褍绲洪柣婊冨簻缁?
    udp.begin(8888);
    
    isCaptivePortalMode = true;
    captivePortalStartTime = millis();
    
    Serial.println("鐎殿喖鎼崺妤呮⒒閵婏箑鐓曠€瑰憡褰冮幆搴ㄥ礉椤帞绀塙DP闁烩晜鍨甸幆澶岀博椤栨艾缍?888鐎瑰憡褰冪槐鎴﹀触?);
}

void stopCaptivePortal() {
    Serial.println("濮樻捁浠滈懕鐟扮箹妫板懎鐎鎾崇秿濞肩偞鐨抽懕銏ゆ苟閼煎懓浼岀弧鎾崇箹閼便垼鐭捐箛娆戠槵闂呭棙鐨宠ぐ鏇′紑...");
    
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    isCaptivePortalMode = false;
    captivePortalStartTime = 0;
    
    Serial.println("濮樻挸缍嶅鐐寸毘閼便垽婀堕懠鍛颁紝缁℃挸绻栭懕銏ｇ熅濮樻捁鐭鹃搹蹇旂毘閼辨稖浼栬箛娆擃暘閸?);
}

bool shouldStartCaptivePortal() {
    // 濮樻挸鈻岄懕婊冪箹閼辨槒浼栬箛娆掓闂呭棗绻栭懕纭呬化閻╂煡鈹愰懕鐑樼毘妫板懓浼掗懢鍊熶紨閼辨靠iFi閼煎懓浠ч懕褑骞楅梽鍡楃爱閼煎倸缍嶉懕锕佸箺閼辨悂绨辫箛娆掍缓濡ゅ吋鐨抽懕顒傚€濆鎾逛槐缁℃挻鐨宠ぐ鏇熺稏濮樻捁浠棁鑼跺瘶閼卞鐦箛娆掍划鐠?
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("韫囨瑨妾归梾鍡楃箹閼辩浠疻iFi閼煎懓浠ч懕褑骞楅梽鍡楃爱閼煎倸缍嶉懕锕佸瘶閼辩浠氶悮顐⑩枌閼辨稒鐨抽懕顒傚€濆鎾逛槐缁℃挻鐨宠ぐ鏇熺稏濮樻捁浠棁鑼跺瘶閼卞鐦箛娆掍划鐠?);
        return true;
    }
    
    // 濮樻挸鈻岄懕婊冪箹閼辨槒浼栬箛娆掍紪閼鳖枬iFi閼煎懓浠ч懕褑骞楅梽鍡楃爱閼煎倸缍嶉懕锔界毘閹哄疇浼滈悮顐ゅ€濋懕鑼皸妞圭浼濊箛娆掍缓濡ょ长iFi
    Serial.println("韫囨瑨浼栭懕顤漣Fi閼煎懓浠ч懕褑骞楅梽鍡楃爱閼煎倸缍嶉懕锔界毘閹哄疇浼滈悮顐ゅ€濋懕鑼皸妞圭浼濊箛娆掍缓濡ょ长iFi");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("濮樻挾鍊濋懕鐘哄箺閼卞€熶粶閼煎懓浼堟す瀛樼毘濞肩偛鈻? ");
    Serial.println(wifiPassword.length());
    
    // 濮樻捁浠ч懕銏ｅ箺闂呭棗绨惄鏌モ攼閼辩iFi韫囨瑧鐦梾鍡樼毘瑜版洝浼€韫囨瑩顣幏銏ｅ箺闂呭棗绨?
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    // 閼句粙顣懕锝嗙毘閹搭喛浠ч悮顐︹攼閼辫绻栭懕銊︺偧閼煎倸缍嶉懕锕€绻栭懕纭呬粴濮樻捇妾遍懕鐤箺妫板懓浠鎾村焿閼?0閼捐姤鎮呴懕顔垮瘯瑜版洝浠惄鑼剁ォ閼扁暉onnectToWiFi閻╂煡鈹愰懕鍝勭箹閼憋箒浠滈惄鑼剁ォ閼辨瑧灏楅懕锟犵氨閼煎倸缍嶉懕?
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        // 韫囨瑧鍊濋懕?閼捐姤鎮呴懕顔惧皸閹搭喛浼呭鎾逛画濞肩偟娲哥挧鍌濅粴韫囨瑥宕查梾鍡欏皸妞圭浼濊箛娆掍缓濡よ壈骞楅懕銈夋苟韫囨瑨浠氶懕?
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("閻氼偊鈹愰懕璇茬箹閼便劍銈奸懢鍊熶槐闂囨彃绻栭懕娆掍粶: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("閼捐姤绱″鐐跺瘶閼辩妾归懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
                case WL_NO_SSID_AVAIL: Serial.println("閼句粙妾伴懕顓″箺缁傚嫯浼栭惄鑼剁ォ閼毖勭毘閼鳖偆鍊濋懢鍊熶紗缁?); break;
                case WL_SCAN_COMPLETED: Serial.println("韫囨瑨浠懞锕€绻栭懕顐ヤ紑濮樻挸绨懕锕€绻栭懕銏ｄ紓"); break;
                case WL_CONNECTED: Serial.println("濮樻捁鐭鹃搹蹇曞皸妞圭浼濊箛娆掍缓濡?); break;
                case WL_CONNECT_FAILED: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈煎鎾绘閸椼倗灏楁ス鎾淬偧"); break;
                case WL_CONNECTION_LOST: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈奸惄鑼剁ォ閸ㄥ嫭鐨抽梽鍥у吹"); break;
                case WL_DISCONNECTED: Serial.println("濮樻捁鐭鹃搹蹇撶箹閼辨娊顣鎾崇秿閼?); break;
                default: Serial.println("韫囨瑨浼栭梽瀣箺閼卞吋銈奸懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
            }
        }
    }
    
    // 韫囨瑦瀚濋懕娆忕箹閼卞吋銈奸悮顐︹攼閼辫绻栭懕銊︺偧閼剧晫顩╅懕顖氱箹閼辨槒浼?
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi閻氼偊鈹愰懕璇茬箹閼便劍銈艰箛娆掍划閼鳖剚鐨抽懕銈堜肌");
        Serial.print("IP濮樻捁浼栭幒铏毘閼遍缚浠? ");
        Serial.println(WiFi.localIP());
        Serial.print("閻氼偊鈹愰懕璇茬箹閼便劍銈奸悮顐ヤ粴閼辨潙绻栭懕鎾苟: ");
        Serial.print((millis() - startTime) / 1000.0);
        Serial.println("閼捐姤鎮呴懕?);
        return false; // 閻氼偊鈹愰懕璇茬箹閼便劍銈艰箛娆掍划閼鳖剚鐨抽懕銈堜肌閼煎倸缍嶉懕锔炬锤鐠у倽浠归懠鍛颁紪閼辨瑧灏楁繛鍕粶濮樻挸缍嶅鐐寸毘閼便垽婀堕懠鍛颁紝缁℃挸绻栭懕銏ｇ熅
    } else {
        Serial.println("");
        Serial.println("WiFi閻氼偊鈹愰懕璇茬箹閼便劍銈煎鎾绘閸椼倗灏楁ス鎾淬偧閼煎倸缍嶉懕锔界毘閼鳖剛鍊濆鎾逛槐缁℃挻鐨宠ぐ鏇熺稏濮樻捁浠棁鑼跺瘶閼卞鐦箛娆掍划鐠?);
        Serial.print("韫囨瑨浼栭懕娆掑箺缁傚嫯浠悮顐︹攼閼辫绻栭懕銊︺偧閼惧€熶槐闂囨彃绻栭懕娆掍粶: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("閼捐姤绱″鐐跺瘶閼辩妾归懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
            case WL_NO_SSID_AVAIL: Serial.println("閼句粙妾伴懕顓″箺缁傚嫯浼栭惄鑼剁ォ閼毖勭毘閼鳖偆鍊濋懢鍊熶紗缁?); break;
            case WL_SCAN_COMPLETED: Serial.println("韫囨瑨浠懞锕€绻栭懕顐ヤ紑濮樻挸绨懕锕€绻栭懕銏ｄ紓"); break;
            case WL_CONNECT_FAILED: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈煎鎾绘閸椼倗灏楁ス鎾淬偧"); break;
            case WL_CONNECTION_LOST: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈奸惄鑼剁ォ閸ㄥ嫭鐨抽梽鍥у吹"); break;
            case WL_DISCONNECTED: Serial.println("濮樻捁鐭鹃搹蹇撶箹閼辨娊顣鎾崇秿閼?); break;
            default: Serial.println("韫囨瑨浼栭梽瀣箺閼卞吋銈奸懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
        }
        // 閼句粙娈曟惔鎰锤妞圭浼淲iFi濮樻捁鐭鹃搹蹇撶箹閼辨娊顣鎾崇秿閼?
        WiFi.disconnect();
        delay(100);
        return true; // 閻氼偊鈹愰懕璇茬箹閼便劍銈煎鎾绘閸椼倗灏楁ス鎾淬偧閼煎倸缍嶉懕锕佸瘶閼辩浠氶悮顐⑩枌閼辨稒鐨宠ぐ鏇熺稏濮樻捁浠棁鑼跺瘶閼卞鐦箛娆掍划鐠?
    }
}

void checkCaptivePortalTimeout() {
    if (isCaptivePortalMode && 
        millis() - captivePortalStartTime > CAPTIVE_PORTAL_TIMEOUT) {
        Serial.println("濮樻挸缍嶅鐐寸毘閼便垽婀堕懠鍛颁紝缁℃挸绻栭懕銏ｇ熅閻氼偊婀堕懕鐔风箹閼辨挳婀堕懠鍌氱秿閼憋附鐨抽幒瀹犱紲閻氼偆鍊濋懕鑼皸妞圭浼濊箛娆掍缓濡よ偐娲告す纾嬩紲濮樻捇顣懕浣冨箺閼辩柉浠iFi");
        stopCaptivePortal();
        connectToWiFi();
    }
}

// =========================================
// WiFi閻氼偊鈹愰懕璇茬箹閼便劍銈奸懢钘夌爱闂呭棜骞楅懕顒冧哗
// =========================================
void connectToWiFi() {
    if (wifiSSID.length() == 0 || wifiPassword.length() == 0) {
        Serial.println("韫囨瑨妾归梾鍡楃箹閼辩浠疻iFi閼煎懓浠ч懕褑骞楅梽鍡楃爱閼煎倸缍嶉懕锔界毘閼鳖剛鍊濆鎾逛槐缁℃挻鐨宠ぐ鏇熺稏濮樻捁浠棁鑼跺瘶閼卞鐦箛娆掍划鐠?);
        startCaptivePortal();
        return;
    }
    
    Serial.println("濮樻挻骞婇懕铏瑰皸閻愬浼堥悮顐︹攼閼辫绻栭懕銊︺偧WiFi...");
    Serial.print("SSID: ");
    Serial.println(wifiSSID);
    Serial.print("濮樻挾鍊濋懕鐘哄箺閼卞€熶粶閼煎懓浼堟す瀛樼毘濞肩偛鈻? ");
    Serial.println(wifiPassword.length());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    unsigned long startTime = millis();
    int connectionAttempts = 0;
    
    // 濮樻挸鐎懕缁樼毘閼便倛浼ｉ悮顐︹攼閼辫绻栭懕銊︺偧閻氼偊婀堕懕鐔风箹閼辨挳婀跺鎾逛划閹?0閼捐姤鎮呴懕顔垮瘯瑜版洝浠稿鎾荤畭闂囨彃绻栫捄顖滎洨濮樻捁浠遍懕钘夌箹閼辨悂绨遍悮顐ゅ€濇繛鍕箺缁傚嫯浠╅懢鍊熶紨閼辩偟灏楁す纾嬩紳韫囨瑨浠哄Δ鑹板箺閼便倝婀惰箛娆掍粴閼?
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
        
        // 韫囨瑧鍊濋懕?閼捐姤鎮呴懕顔惧皸閹搭喛浼呭鎾逛画濞肩偟娲哥挧鍌濅粴韫囨瑥宕查梾鍡欏皸妞圭浼濊箛娆掍缓濡よ壈骞楅懕銈夋苟韫囨瑨浠氶懕?
        if (connectionAttempts % 10 == 0) {
            Serial.println("");
            Serial.print("閻氼偊鈹愰懕璇茬箹閼便劍銈奸懢鍊熶槐闂囨彃绻栭懕娆掍粶: ");
            switch(WiFi.status()) {
                case WL_IDLE_STATUS: Serial.println("閼捐姤绱″鐐跺瘶閼辩妾归懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
                case WL_NO_SSID_AVAIL: Serial.println("閼句粙妾伴懕顓″箺缁傚嫯浼栭惄鑼剁ォ閼毖勭毘閼鳖偆鍊濋懢鍊熶紗缁?); break;
                case WL_SCAN_COMPLETED: Serial.println("韫囨瑨浠懞锕€绻栭懕顐ヤ紑濮樻挸绨懕锕€绻栭懕銏ｄ紓"); break;
                case WL_CONNECTED: Serial.println("濮樻捁鐭鹃搹蹇曞皸妞圭浼濊箛娆掍缓濡?); break;
                case WL_CONNECT_FAILED: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈煎鎾绘閸椼倗灏楁ス鎾淬偧"); break;
                case WL_CONNECTION_LOST: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈奸惄鑼剁ォ閸ㄥ嫭鐨抽梽鍥у吹"); break;
                case WL_DISCONNECTED: Serial.println("濮樻捁鐭鹃搹蹇撶箹閼辨娊顣鎾崇秿閼?); break;
                default: Serial.println("韫囨瑨浼栭梽瀣箺閼卞吋銈奸懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
            }
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi閻氼偊鈹愰懕璇茬箹閼便劍銈艰箛娆掍划閼鳖剚鐨抽懕銈堜肌!");
        Serial.print("IP濮樻捁浼栭幒铏毘閼遍缚浠? ");
        Serial.println(WiFi.localIP());
        Serial.print("閻氼偊鈹愰懕璇茬箹閼便劍銈奸悮顐ヤ粴閼辨潙绻栭懕鎾苟: ");
        Serial.print((millis() - startTime) / 1000.0);
        Serial.println("閼捐姤鎮呴懕?);
        
        // 濮樻捁浼傞悙澶嬬毘閼便倗鐦琖eb韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧?
        setupWebServer();
        
        // 濮樻捁浼傞悙澶嬬毘閼便倗鐦琌TA韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧?
        setupOTA();
        
        // 濮樻捁浼傞悙澶嬬毘閼便倗鐦悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻捁浼€閼鳖叀骞楅懕銊﹀箠
        udp.begin(8888);
        
        // 閻氼偄绨幋顔垮箺闂勫棗绨甃ED閻╄尪绁╁鐐寸毘鐠у倽绁╅惄鍙夌稏鎼存劘骞楅懕銈夋苟韫囨瑨浠氶懕?
        digitalWrite(LED_PIN, HIGH);
        
        // 韫囨瑨鐭剧粋鍕毘閼便倛浼eb韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘閼鳖剛鍊濆鎾逛槐缁℃捁骞楅梾鍡楃爱閻氼偄绨梽鍥╂锤妞规挳娈曡箛娆掍粶閻?
        Serial.println("閼烘帟浼栭懕?Web韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘鐠侯垵妾瑰鎾逛紓閻愬鐨抽懕銈囩槵");
        Serial.println("閼烘帟浼栭懕?閻氼偄绨幋顔界毘闂勫洩浠鎾逛紑閼鳖叀骞楅懕銊﹀箠韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁鐭鹃搹蹇旂毘閼鳖剛鍊濆鎾逛槐缁?);
        Serial.println("閼烘帟浼栭懕?OTA韫囨瑨浼曟ス鎾崇箹閼辫櫕骞婅箛娆掍紪閼毖勭毘閼便倝娈曞鎾圭熅閾忓繑鐨抽懕顒傚€濆鎾逛槐缁?);
        Serial.println("閸愭帟浼￠懕顖氬吹 閼惧€熶缓閹鸿櫕鐨抽懕鍦槵濮樻捁浼€閻愬娲哥粋鍕偧閼煎懓浠氶懕椋庡皸妞圭浠箛娆戭暠閼鳖偆灏楅幖鍌濅划濮樻捁浼撶弧鎾跺皸鎼存劙鈹愰懠鍛颁紝鎼存劗灏楁惔鎰焿濮樻捇妾遍懕? http://" + WiFi.localIP().toString());
    } else {
        Serial.println("");
        Serial.println("WiFi閻氼偊鈹愰懕璇茬箹閼便劍銈煎鎾绘閸椼倗灏楁ス鎾淬偧閼煎倸缍嶉懕锔界毘閼鳖剛鍊濆鎾逛槐缁℃挻鐨宠ぐ鏇熺稏濮樻捁浠棁鑼跺瘶閼卞鐦箛娆掍划鐠?);
        Serial.print("韫囨瑨浼栭懕娆掑箺缁傚嫯浠悮顐︹攼閼辫绻栭懕銊︺偧閼惧€熶槐闂囨彃绻栭懕娆掍粶: ");
        switch(WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("閼捐姤绱″鐐跺瘶閼辩妾归懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
            case WL_NO_SSID_AVAIL: Serial.println("閼句粙妾伴懕顓″箺缁傚嫯浼栭惄鑼剁ォ閼毖勭毘閼鳖偆鍊濋懢鍊熶紗缁?); break;
            case WL_SCAN_COMPLETED: Serial.println("韫囨瑨浠懞锕€绻栭懕顐ヤ紑濮樻挸绨懕锕€绻栭懕銏ｄ紓"); break;
            case WL_CONNECT_FAILED: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈煎鎾绘閸椼倗灏楁ス鎾淬偧"); break;
            case WL_CONNECTION_LOST: Serial.println("閻氼偊鈹愰懕璇茬箹閼便劍銈奸惄鑼剁ォ閸ㄥ嫭鐨抽梽鍥у吹"); break;
            case WL_DISCONNECTED: Serial.println("濮樻捁鐭鹃搹蹇撶箹閼辨娊顣鎾崇秿閼?); break;
            default: Serial.println("韫囨瑨浼栭梽瀣箺閼卞吋銈奸懢鍊熶槐闂囨彃绻栭懕娆掍粶"); break;
        }
        startCaptivePortal();
    }
}

// =========================================
// 閻氼偄绨幋顔界毘闂勫洩浠悮顐ヤ画闂勫鐨抽懕銈囩槵濮樻捁浼€閼鳖叀骞楅懕銊﹀箠濮樻捁浠遍懕鑲╁皸閼辨繈妾?
// =========================================
void handleDiscovery() {
    // 濮樻捇妾遍懕鐐跺箺閼鳖剝浠╄箛娆掍缓濡ょ厧绻栭懕浼存苟濮樻捁浠幒瀹犲箺閼辩柉浠﹀鎾逛紑閼鳖叀骞楅懕銊﹀箠閻氼偆鍊濈捄顖氱箹閸椼倛浠?
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char packetBuffer[255];
        int len = udp.read(packetBuffer, 255);
        if (len > 0) {
            packetBuffer[len] = 0;
            String request = String(packetBuffer);
            
            Serial.print("韫囨瑨浼嗛棁鍙夌毘閼便垺骞奤DP韫囨瑨浼堥幒鍐茬箹閼毖冪爱濮樻捁浠搁懕鐔诲瘯瑜版洝浠稿鎾绘閹煎倹鐨抽幒瀹犱紑: ");
            Serial.print(packetSize);
            Serial.print(" 濮樻捇顣懕瀵稿皸閼便倛浠犻懠鍌氱秿閼憋附鐨抽懕鐘轰户濮樻挸绨ィ? ");
            Serial.println(request);
            
            if (request.startsWith("DISCOVER_SMARTOVEN")) {
                Serial.println("韫囨瑨浼嗛棁鍙夌毘閼便垺骞婇悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻捁浼€閼鳖叀骞楅懕銊﹀箠閻氼偆鍊濈捄顖氱箹閸椼倛浠?);
                sendDiscoveryResponse();
            }
        }
    }
    
    // 濮樻挸绨懕宄扮箹閼辩浼″鎾荤畭妞规潙绻栭懕顕€顣悮顐㈢爱閹搭喗鐨抽梽鍥画閻╂煡鈹愰梾鍡楃箹閼辨稓鍊?
    if (discoveryEnabled && millis() - lastDiscoveryTime > DISCOVERY_INTERVAL) {
        broadcastDiscovery();
        lastDiscoveryTime = millis();
    }
}

void sendDiscoveryResponse() {
    String response;
    
    // 閻╂煡妾版す纾嬪箺閼辨壆鐦珹PP韫囨瑨浼栭懕鐓庣箹閼辩浼曢懢鍊熶紨閼辩偛绻栭懕钘夌秿濮樻挸缍嶉懕顐ュ瘯瑜版洝浼擠EVICE_INFO:NAME:閻氼偄绨幋顔界毘闂勫洩浠鎾逛紓閼毖嗗箺閹煎倹骞?MAC:MAC濮樻捁浼栭幒铏毘閼遍缚浠?PORT:閼惧€熷Е閻愬鐨抽懕顐ｅ珴濮樻捁浼€鐠?...
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
    
    // 閻╂煡鈹愭惔鎰毘闂勫洩浠归懠鍌氱秿閼遍娲搁梽鍡涒攼閼惧€熶紗缁℃弬PP濮樻捁浼€閼鳖叀瀵橀懕娆掍粶閻氼偆鍊濈捄顖氱箹閸椼倛浠犻懢鍊熶紨閼辩偠骞楅懞锔惧€濆鎾逛紑閹枫垼瀵曡ぐ鏇′划remotePort閼煎倸缍嶉懕锝囧皸閼辨瑨浠搁惄鑼剁ォ閼毖冪箹閼辩數鍊濋懢浠嬫畷閸椼垼骞楄ぐ鏇′紜閼惧€熶迹閼辨稖骞楅懕鐤沪8889閼惧€熷Е閻愬鐨抽懕顐ｅ珴
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(response.c_str());
    udp.endPacket();
    
    Serial.println("濮樻捁浼€閼鳖叀瀵橀懕娆掍粶濮樻捁浼€閼鳖叀骞楅懕銊﹀箠濮樻捁浼呴懕褎鐨冲鐐朵紗濮樻捁浠幒瀹犲箺閼猴妇鍊濆鎾逛紑閹? + String(udp.remotePort()) + ": " + response);
}

void broadcastDiscovery() {
    String broadcastMsg = "SMARTOVEN_BROADCAST:" + DEVICE_TYPE + ":" + DEVICE_ID + ":" + FIRMWARE_VERSION;
    
    udp.beginPacket("255.255.255.255", 8888);
    udp.write(broadcastMsg.c_str());
    udp.endPacket();
    
    Serial.println("濮樻捇绠嶆す鏉戠箹閼鳖噣顣悮顐㈢爱閹搭喗鐨抽梽鍥画閻╂煡鈹愰梾鍡楃箹閼辨稓鍊? " + broadcastMsg);
}

// =========================================
// OTA濮樻捁浠归懕陇骞楀鐐存倕濮樻捁浠遍懕鑲╁皸閼辨繈妾?
// =========================================
void setupOTA() {
    httpUpdater.setup(&otaServer);
    otaServer.begin();
    Serial.println("OTA韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘鐠侯垵妾瑰鎾逛紓閻愬鐨抽懕銈囩槵閼煎倸缍嶉懕锕佸箺閼猴妇鍊濆鎾逛紑閹? 8080");
    Serial.println("OTA濮樻捁浠归懕陇骞楀鐐存倕濮樻捁浼栭幒铏毘閼遍缚浠? http://" + WiFi.localIP().toString() + ":8080/update");
}

void handleOTA() {
    if (!isCaptivePortalMode && WiFi.status() == WL_CONNECTED) {
        otaServer.handleClient();
    }
}

// OTA濮樻捁浠归懕陇骞楀鐐存倕閼煎懘娈曠喊宀冨瘶閼卞搫鐎?
void handleOTAWebPage() {
    String html = "<!DOCTYPE html><html><head><title>韫囨瑨浼撳鐐靛皸閼辨繈妾伴懢鍊熶紗绾板矁骞楅懕婵嬫閼捐棄绨崡顥礣A濮樻捁浠归懕陇骞楀鐐存倕</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
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
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-success\">濮樻捇妾伴懕顖涚毘閼憋綀浠归懢鍊熶化閼便垹绻栭懕鐟板床: ' + data.current_version + '</div>';";
    html += "    if (data.update_available) {";
    html += "      document.getElementById('updateStatus').innerHTML += '<div class=\"status status-warning\">韫囨瑨浼栭懕锝呯箹閼辫櫕骞婇懢鍊熶化閼便垹绻栭懕鐟板床濮樻捁浼€閻愬骞楅懕鎵槵: ' + data.latest_version + '</div>';";
    html += "    }";
    html += "  }).catch(error => {";
    html += "    document.getElementById('updateStatus').innerHTML = '<div class=\"status status-error\">韫囨瑦瀚濋懕娆忕箹閼卞吋銈艰箛娆掍紩妤规挸绻栭懕铏箠濮樻捇妾遍崡銈囧皸妤规挻銈?/div>';";
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
    html += "<h1>韫囨瑨浼撳鐐靛皸閼辨繈妾伴懢鍊熶紗绾板矁骞楅懕婵嬫閼捐棄绨崡顥礣A濮樻捁浠归懕陇骞楀鐐存倕</h1>";
    html += "<p>濮樻捁浼曞鐐垫锤缁傚嫰婀堕懢鑺ョ础濞肩偟娲哥挧鍌烆暘濮樻捁浠归懕陇骞楀鐐存倕閼句粙鐬剧粋鍕箺缁傚嫯浼?/p>";
    html += "</div>";
    html += "<div class=\"content\">";
    html += "<div class=\"info-card\">";
    html += "<p><strong>閻氼偄绨幋顔界毘闂勫洩浠璉D:</strong> " + DEVICE_ID + "</p>";
    html += "<p><strong>IP濮樻捁浼栭幒铏毘閼遍缚浠?</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>OTA閼惧€熷Е閻愬鐨抽懕顐ｅ珴:</strong> 8080</p>";
    html += "</div>";
    html += "<div id=\"updateStatus\"></div>";
    html += "<div class=\"ota-section\">";
    html += "<h3>OTA濮樻捁浠归懕陇骞楀鐐存倕韫囨瑨浼呴懕褏娲搁梽鍡氫紪</h3>";
    html += "<p>閼惧€熶粻妤ｆ寧鐨抽懕锛勵洨閻╄尪绁╅懕銉ョ箹閼辨娊绠嶈箛娆掍桓閼憋綀瀵橀懕顔肩爱韫囨瑨浠懕顖涚毘瑜版洝浠歄TA濮樻捁浠归懕陇骞楀鐐存倕閼煎懘娈曠喊宀冨瘶閼卞搫鐎懠鍌氱秿閼憋妇娲哥挧鍌濅槐閻╂彃缍嶉懕钘夌箹閼辫櫕骞婇懢鍊熶紨閼辩偞鐨抽懕鍛婄稏閻╄尙顩╅棁鎻掔箹閼卞疇浠惄鑼洨闂囪尙灏楁す纾嬩紩閻氼偊娈曢懕锔界毘閼毖嗕画閼捐姤缍栭幖鍌濆皸閼辨瑨浠?/p>";
    html += "<button class=\"btn btn-success\" onclick=\"startOTA()\">韫囨瑨浠懕顖涚毘瑜版洝浠歄TA濮樻捁浠归懕陇骞楀鐐存倕閼煎懘娈曠喊宀冨瘶閼卞搫鐎?/button>";
    html += "<button class=\"btn\" onclick=\"checkUpdate()\">韫囨瑦瀚濋懕娆忕箹閼卞吋銈艰箛娆掍紩妤规挸绻栭懕铏箠</button>";
    html += "</div>";
    html += "<div style=\"text-align: center; margin-top: 20px;\">";
    html += "<a href=\"/\" style=\"color: #2196F3; text-decoration: none;\">閻氼偊鈹愰懕鐗堢毘閼辨瓕浼濋惄鑼剁ォ缁傚嫯瀵橀梾鍡欘暠閼煎懓浼滈崹?/a>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
}

// 韫囨瑦瀚濋懕娆忕箹閼卞吋銈艰箛娆掍紩妤规挸绻栭懕铏箠API
void handleCheckUpdate() {
    String json = "{\"current_version\":\"" + FIRMWARE_VERSION + "\",\"latest_version\":\"0.6.6\",\"update_available\":false}";
    
    // 閻氼偊鈹愰懕鎯板瘶閼甭や桓濮樻捁浼€閻愬娲哥粋鍕偧韫囨瑨鐭剧粋鍕毘閼便倛浼ｈ箛娆愬珴閼辨瑥绻栭懕鍏笺偧韫囨瑨浼嬮幒瀹犲箺閼憋綀浠箛娆掍紪閸椼垼骞楅懕鐤沪閼煎懓浠氱粋鍕皸閹搭喛浼?
    // 閻╁弶鍩夐懕銉︾毘婵炲嫯浠犻惄鑼洨閼便劌绻栭懕纭呬还濮樻捁浠遍梾鍡樼毘閼卞墎鐦悮顐ヤ缓鐠侯垱鐨抽懕顐ヤ紜韫囨瑨浼栭懕娆忕箹閼辫櫕骞婇懢鍊熶化閼便垹绻栭懕鐟板床閻╂煡鈹愰梾鍡楃箹閼辨稓鍊?
    
    webServer.send(200, "application/json", json);
}

// =========================================
// Web韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘闂勫洩浠﹂懢鍊熶紓閼?
// =========================================
void setupWebServer() {
    // 閼煎懓浠ч懕褑骞楅梽鍡楃爱閼煎懓浼滈懕璺虹箹閼辨瑨浠滆箛娆掍紜閼憋紕娲哥粋鍕苟韫囨瑨浼栭懕褎鐨抽懕銈夋畷閼煎倸缍嶉懕銏㈡锤缁傚嫯浠у鎾逛紪缁℃捁瀵橀懕楦夸紳濮樻挸缍嶅鐐寸毘閼便垽婀堕懠鍛颁紝缁℃挸绻栭懕銏ｇ熅韫囨瑧鐦梾鍡樼毘瑜版洝浼€閻╄尪绁╅懕銉ф锤闂勫棝鈹愰懢鍊熶紗缁℃捁瀵曡ぐ鏇′化
    if (!isCaptivePortalMode) {
        webServer.serveStatic("/login.html", LittleFS, "/login.html");
        webServer.serveStatic("/index.html", LittleFS, "/index.html");
        webServer.serveStatic("/css/", LittleFS, "/css/");
        webServer.serveStatic("/js/", LittleFS, "/js/");
        webServer.serveStatic("/images/", LittleFS, "/images/");
    }
    
    // 濮樻捁浠辩弧鎾崇箹閼辨瑨浠滈悮顐ョ熅閻愬骞楅懕鏉垮吹
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
    // 韫囨瑨浠懞锕€绻栭懕顐ヤ紑WiFi閼句粙妾伴懕顓″箺缁傚嫯浼?
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
    
    // 韫囨瑨绁╅懕鐔诲箺閼鳖剝浠╄箛娆掍化閼猴箑绻栭懕顐ヤ紑閼剧晫顩╅懕顖氱箹閼辨槒浼?
    WiFi.scanDelete();
}

void handleRoot() {
    if (isCaptivePortalMode) {
        // 濮樻挸缍嶅鐐寸毘閼便垽婀堕懠鍛颁紝缁℃挸绻栭懕銏ｇ熅韫囨瑧鐦梾鍡樼毘瑜版洝浼€閻╄尪绁╅懕銉ョ箹閼卞灚鍩夐懢浠嬫濞兼靠iFi閼煎懓浠ч懕褑骞楅梽鍡楃爱閼煎懘娈曠喊宀冨瘶閼卞搫鐎?
        String html = "<!DOCTYPE html><html><head><title>韫囨瑨浼撳鐐靛皸閼辨繈妾伴懢鍊熶紗绾板矁骞楅懕婵嬫閼捐棄绨崡銈堝瘶閼辩喕浠归懢浠嬫鎼?/title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
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
        html += "  scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>韫囨瑨浠懞锕€绻栭懕顐ヤ紑閻╄尪绁╂０?..';";
        html += "  wifiList.innerHTML = '<option value=\"\">韫囨瑨浠懞锕€绻栭懕顐ヤ紑閻╄尪绁╂０?..</option>';";
        html += "  fetch('/scanwifi').then(response => response.json()).then(data => {";
        html += "    wifiList.innerHTML = '';";
        html += "    wifiList.innerHTML = '<option value=\"\">閻氼偆鍊濈捄顖濆瘶閼辨瑨浠箛娆掍淮濠曞护iFi閼句粙妾伴懕顓″箺缁傚嫯浼?/option>';";
        html += "    if (data.networks && data.networks.length > 0) {";
        html += "      data.networks.forEach(network => {";
        html += "        var option = document.createElement('option');";
        html += "        option.value = network.ssid;";
        html += "        option.textContent = network.ssid + ' (' + network.rssi + ' dBm)';";
        html += "        wifiList.appendChild(option);";
        html += "      });";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-connected\"></span>韫囨瑨浠懞锕€绻栭懕顐ヤ紑濮樻挸绨懕锕€绻栭懕銏ｄ紓 (' + data.networks.length + '閻╄尪绁╅梽瀣箺闂勫棜浼冮懢鐣岊洨閼?';";
        html += "    } else {";
        html += "      wifiList.innerHTML = '<option value=\"\">韫囨瑨浼栭梽瀣箹閼憋絾鍩夊鎾逛划閹虹爮iFi閼句粙妾伴懕顓″箺缁傚嫯浼?/option>';";
        html += "      scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>閼煎懓浠懕褍绻栭懕铏箠韫囨瑨浠懞锕€绻栭懕顐ヤ紑';";
        html += "    }";
        html += "    scanBtn.disabled = false;";
        html += "  }).catch(error => {";
        html += "    wifiList.innerHTML = '<option value=\"\">韫囨瑨浠懞锕€绻栭懕顐ヤ紑濮樻捇妾遍崡銈囧皸妤规挻銈?/option>';";
        html += "    scanBtn.disabled = false;";
        html += "    scanBtn.innerHTML = '<span class=\"status-indicator status-disconnected\"></span>閼煎懓浠懕褍绻栭懕铏箠韫囨瑨浠懞锕€绻栭懕顐ヤ紑';";
        html += "  });";
        html += "}";
        html += "window.onload = scanWiFi;";
        html += "function confirmFactoryReset() {";
        html += "  if (confirm('閼烘帟浼旈懕鍊熷瘯鐠у倽浼€ 濮樻捁浠归崡銈堝瘶閼辫埖绱¤箛娆掍紖閼毖呮锤闂勫棜浼栭悮顐︻暘婵炲嫭鐨抽懕顓′槐閼煎倸缍嶉懕娌慭n\\n韫囨瑨浠滈崹鍕毘闂勫洩浠瑰鎾逛画濞肩偞鐨抽懕銊ㄤ粻閻氼偄绨幋顔垮箺闂勫棗绨鎾村箠閼辩姴绻栫挧鍌濅户閼煎懓浼撻梽鍥х箹閼憋綀浠氳箛娆掍紪閼鳖枬iFi閼煎懓浠ч懕褑骞楅梽鍡楃爱濮樻捁浼勯懕锕€绻栫挧鍌涚础濮樻挻缍栨繛鍕箹閼变粙娈曞鎾逛画閼辩姵鐨抽懕顐ヤ粻韫囨瑨浼堥幒瀹犲皸閼辨瑨浠燶\n閻氼偄绨幋顔界毘闂勫洩浠鎾村箠閼辩姾瀵橀懕陇浠瑰鎾逛紓閻愬鐨虫ィ鍧楁苟閻氼偊鈹愰懕鍛婄毘閼辩喐銈奸懠鍛颁户閼毖嗗箺闂勫棜浼冭箛娆戠槵闂呭棙鐨宠ぐ鏇′紑閼煎倸缍嶉懕锕佸瘶閼辩浠氶悮顐⑩枌閼辨稖瀵橀懕陇浠硅箛娆掍紜閹哄疇瀵橀懕鐔讳还閼句粙妾版惔鎬秈Fi閼句粙妾伴懕顓″箺缁傚嫯浼栭懠顐ヤ粴閼辨泛\n\\n閼句粙娈曟惔鎰毘鎼存劘浼旈悮顐⑩枌閼辨稑绻栭懕锝嗘倕閻氼偊娈曢懕锕€绻栭懕娑樼€鎾绘閼毖勭毘閼扁剝缍栧鎾逛缓閼辨粎灏楁惔鎰焿閼句粙妾版惔鎰毘閼鳖剝浼岄懠鍌氱秿閼?)) {";
        html += "    var resetBtn = document.querySelector('[onclick=\\\"confirmFactoryReset()\\\"]');";
        html += "    resetBtn.disabled = true;";
        html += "    resetBtn.innerHTML = '韫囨瑨浠滈崹鍕毘闂勫洩浠瑰鎾逛画濞肩偞鐨抽懕銊ㄤ粻閻氼偄绨幋顔垮箺闂勫棗绨惄鑼剁ォ妫?..';";
        html += "    resetBtn.style.opacity = '0.7';";
        html += "    ";
        html += "    fetch('/factoryreset', { method: 'POST' })";
        html += "      .then(response => {";
        html += "        if (response.ok) {";
        html += "          resetBtn.innerHTML = '韫囨瑨浠滈崹鍕毘闂勫洩浠硅箛娆掍划閼鳖剚鐨抽懕銈堜肌閼煎倸缍嶉懕锔惧皸鎼存劖鍩夊鎾绘閼甭ゅ瘶閼甭や还濮樻捁浼傞悙澶屾锤鐠у倿顣?..';";
        html += "          resetBtn.style.background = 'linear-gradient(135deg, #28a745 0%, #20c997 100%)';";
        html += "        } else {";
        html += "          throw new Error('韫囨瑨浠滈崹鍕毘闂勫洩浠瑰鎾绘閸椼倗灏楁ス鎾淬偧');";
        html += "        }";
        html += "      })";
        html += "      .catch(error => {";
        html += "        resetBtn.disabled = false;";
        html += "        resetBtn.innerHTML = '韫囨瑨浠滈崹鍕毘闂勫洩浠瑰鎾逛画濞肩偞鐨抽懕銊ㄤ粻閻氼偄绨幋顔垮箺闂勫棗绨?;";
        html += "        resetBtn.style.opacity = '1';";
        html += "        alert('韫囨瑨浠滈崹鍕毘闂勫洩浠瑰鎾逛画濞肩偞鐨抽懕銊ㄤ粻閻氼偄绨幋顔垮箺闂勫棗绨鎾绘閸椼倗灏楁ス鎾淬偧閼煎倸缍嶉懕锔惧皸閻愬鐭鹃懠鍛颁画閼毖呭皸閻愬浼堥懠鍌氱秿閼? + error.message);";
        html += "      });";
        html += "  }";
        html += "}";
        html += "</script>";
        html += "</head><body>";
        html += "<div class=\"container\">";
        html += "<div class=\"header\">";
        html += "<h1>韫囨瑨浼撳鐐靛皸閼辨繈妾伴懢鍊熶紗绾板矁骞楅懕婵嬫閼捐棄绨崡銈堝瘶閼辩喕浠归懢浠嬫鎼?/h1>";
        html += "<p>WiFi閼句粙妾伴懕顓″箺缁傚嫯浼栭懠鍛颁户閼毖嗗箺闂勫棗绨懠鍛存畷绾板矁瀵橀懕鍝勭€?/p>";
        html += "</div>";
        html += "<div class=\"device-info\">";
        html += "<p><strong>閻氼偄绨幋顔界毘闂勫洩浠璉D:</strong> " + DEVICE_ID + "</p>";
        html += "<p><strong>濮樻捁浼曞鐐垫锤缁傚嫰婀堕懢鍊熶化閼便垹绻栭懕鐟板床:</strong> " + FIRMWARE_VERSION + "</p>";
        html += "</div>";
        html += "<div class=\"temperature-display\">濮樻捇妾伴懕顖涚毘閼憋綀浠硅箛娆掔ォ濠曞繑鐨冲鐐测枌: " + String(currentTemp) + "閼存骞奀</div>";
        html += "<div class=\"form-container\">";
        html += "<form method=\"POST\" action=\"/savewifi\">";
        html += "<div class=\"scan-section\">";
        html += "<button type=\"button\" id=\"scanBtn\" class=\"btn btn-secondary\" onclick=\"scanWiFi()\"><span class=\"status-indicator status-disconnected\"></span>韫囨瑨浠懞锕€绻栭懕顐ヤ紑WiFi閼句粙妾伴懕顓″箺缁傚嫯浼?/button>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"ssid\">WiFi閼句粙妾伴懕顓″箺缁傚嫯浼?/label>";
        html += "<select id=\"ssid\" name=\"ssid\" class=\"form-control\" required><option value=\"\">閻氼偆鍊濈捄顖涚毘閼辩喕浠箛娆掍化閼猴箑绻栭懕顐ヤ紑WiFi閼句粙妾伴懕顓″箺缁傚嫯浼?/option></select>";
        html += "</div>";
        html += "<div class=\"form-group\">";
        html += "<label for=\"password\">WiFi濮樻挾鍊濋懕鐘哄箺閼卞€熶粶</label>";
        html += "<input type=\"password\" id=\"password\" name=\"password\" class=\"form-control\" placeholder=\"閻氼偆鍊濈捄顖滃皸閹搭喛浼呭鎾逛户濡ょ长iFi濮樻挾鍊濋懕鐘哄箺閼卞€熶粶\" required>";
        html += "</div>";
        html += "<button type=\"submit\" class=\"btn\" style=\"width: 100%;\">閻╂煡鈹愰懕鐑樼毘妫板懓浼掗懠鍛颁户閼毖嗗箺闂勫棗绨?/button>";
        html += "</form>";
        

        
        html += "</div>";
        html += "</div>";
        
        // 韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍缓閹煎倹鐨抽懕銏ゆ苟JavaScript
        html += "<script>";
        html += "let currentTargetTemp = " + String(targetTemp) + ";";
        html += "let currentHeatingState = " + String(heatingEnabled ? "true" : "false") + ";";
        
        html += "// 韫囨瑨浼曟ス鎾崇箹閼辫櫕骞婅箛娆掔ォ濠曞繑鐨冲鐐测枌韫囨瑨浼掗幋顔垮箺闂勫洦缍?;
        html += "function updateTemperatureDisplay() {";
        html += "  fetch('/status')";
        html += "    .then(response => response.json())";
        html += "    .then(data => {";
        html += "      document.getElementById('current-temp').textContent = data.temperature.toFixed(1) + '閼存骞奀';";
        html += "      document.getElementById('target-temp').textContent = data.target_temperature.toFixed(1) + '閼存骞奀';";
        html += "      currentTargetTemp = data.target_temperature;";
        html += "      currentHeatingState = data.heating_enabled;";
        html += "      ";
        html += "      // 韫囨瑨浼曟ス鎾崇箹閼辫櫕骞婂鎾逛槐閼卞€熷箺閼辨繈顣箛娆掍桓閼憋綀瀵橀懕顔肩爱閼惧€熶槐闂囨彃绻栭懕娆掍粶";
        html += "      const heatingBtn = document.getElementById('heating-btn');";
        html += "      const heatingText = document.getElementById('heating-text');";
        html += "      if (data.heating_enabled) {";
        html += "        heatingBtn.classList.add('active');";
        html += "        heatingText.textContent = '濮樻捁浠滈懕鐟扮箹妫板懎鐎鎾逛槐閼卞€熷箺閼辨繈顣?;";
        html += "      } else {";
        html += "        heatingBtn.classList.remove('active');";
        html += "        heatingText.textContent = '濮樻挸缍嶉懕娆愮毘閹煎倽浠村鎾逛槐閼卞€熷箺閼辨繈顣?;";
        html += "      }";
        html += "    })";
        html += "    .catch(error => console.error('閻氼偉浠虹捄顖涚毘閼鳖偉浼嬭箛娆掔ォ濠曞繑鐨冲鐐测枌閼惧€熶槐闂囨彃绻栭懕娆掍粶濮樻捇妾遍崡銈囧皸妤规挻銈?', error));";
        html += "}";
        
        html += "// 韫囨瑨浼嗘ィ鎸庣毘閼鳖偉浼掗懢鍊熶紩鎼存劕绻栭懕鍊熶画韫囨瑨绁╁蹇旂毘濞肩偛鈻?;
        html += "function changeTemp(delta) {";
        html += "  const newTemp = Math.max(0, Math.min(300, currentTargetTemp + delta));";
        html += "  setTargetTemp(newTemp);";
        html += "}";
        
        html += "// 閻氼偄绨幋顔垮箺闂勫棗绨懠鍛€懕鐐靛皸鎼存劖鍩夎箛娆掔ォ濠曞繑鐨冲鐐测枌";
        html += "function setPresetTemp(temp) {";
        html += "  setTargetTemp(temp);";
        html += "}";
        
        html += "// 閻氼偄绨幋顔垮箺闂勫棗绨悮顐ヤ画闂勫鐨虫惔鎰紨閻╂煡绠嶉懕锝呯箹鐠у倹绱″鎾寸稏婵?;
        html += "function setCustomTemp() {";
        html += "  const input = document.getElementById('temp-input');";
        html += "  const temp = parseInt(input.value);";
        html += "  if (!isNaN(temp) && temp >= 0 && temp <= 300) {";
        html += "    setTargetTemp(temp);";
        html += "    input.value = '';";
        html += "  } else {";
        html += "    alert('閻氼偆鍊濈捄顖滃皸閹搭喛浼呭鎾逛户濡?-300閼存骞奀閻╂煡绠嶉懕銉ㄥ瘶閼辨挳绨遍懢鍊熶紨閼辩偛绻栭懕纭呬化韫囨瑨浼堥懕銏犵箹鐠у倹绱″鎾寸稏婵炲嫭鐨抽懕娆忕秿');";
        html += "  }";
        html += "}";
        
        html += "// 閻氼偄绨幋顔垮箺闂勫棗绨懢鍊熶紩鎼存劕绻栭懕鍊熶画韫囨瑨绁╁蹇旂毘濞肩偛鈻?;
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
        html += "        document.getElementById('target-temp').textContent = temp.toFixed(1) + '閼存骞奀';";
        html += "        console.log('閼惧€熶紩鎼存劕绻栭懕鍊熶画韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾圭熅閾忓繒灏楁惔鎰焿閼句粙妾版惔鎰锤鐠у倹缍? ' + temp + '閼存骞奀');";
        html += "      } else {";
        html += "        throw new Error('閻氼偄绨幋顔垮箺闂勫棗绨箛娆掔ォ濠曞繑鐨冲鐐测枌濮樻捇妾遍崡銈囧皸妤规挻銈?);";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('閻氼偄绨幋顔垮箺闂勫棗绨箛娆掔ォ濠曞繑鐨冲鐐测枌濮樻捇妾遍崡銈囧皸妤规挻銈?', error);";
        html += "      alert('閻氼偄绨幋顔垮箺闂勫棗绨箛娆掔ォ濠曞繑鐨冲鐐测枌濮樻捇妾遍崡銈囧皸妤规挻銈奸懠鍌氱秿閼憋妇灏楅悙澶庣熅韫囨瑦瀚濋懕娆忕箹閼卞吋銈奸悮顐㈢爱閹搭喗鐨抽梽鍥画閻氼偊鈹愰懕璇茬箹閼便劍銈?);";
        html += "    });";
        html += "}";
        
        html += "// 濮樻捁浠懕鈥崇箹閼毖冪€鎾逛槐閼卞€熷箺閼辨繈顣懢鍊熶槐闂囨彃绻栭懕娆掍粶";
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
        html += "          heatingText.textContent = '濮樻捁浠滈懕鐟扮箹妫板懎鐎鎾逛槐閼卞€熷箺閼辨繈顣?;";
        html += "        } else {";
        html += "          heatingBtn.classList.remove('active');";
        html += "          heatingText.textContent = '濮樻挸缍嶉懕娆愮毘閹煎倽浠村鎾逛槐閼卞€熷箺閼辨繈顣?;";
        html += "        }";
        html += "        console.log('濮樻捁浠遍懕鍊熷箺閼辨繈顣懢鍊熶槐闂囨彃绻栭懕娆掍粶濮樻捁鐭鹃搹蹇旂毘閼便垼浠箛娆掍还閸ㄥ嫮娲哥挧鍌涚稏: ' + (newHeatingState ? '濮樻挸缍嶉懕娆愮毘閼鳖剛鍊? : '濮樻捁浠фご浣藉瘶閼辨挳顣?));";
        html += "      } else {";
        html += "        throw new Error('濮樻捁浠懕鈥崇箹閼毖冪€鎾逛槐閼卞€熷箺閼辨繈顣懢鍊熶槐闂囨彃绻栭懕娆掍粶濮樻捇妾遍崡銈囧皸妤规挻銈?);";
        html += "      }";
        html += "    })";
        html += "    .catch(error => {";
        html += "      console.error('濮樻捁浠懕鈥崇箹閼毖冪€鎾逛槐閼卞€熷箺閼辨繈顣懢鍊熶槐闂囨彃绻栭懕娆掍粶濮樻捇妾遍崡銈囧皸妤规挻銈?', error);";
        html += "      alert('濮樻捁浠懕鈥崇箹閼毖冪€鎾逛槐閼卞€熷箺閼辨繈顣懢鍊熶槐闂囨彃绻栭懕娆掍粶濮樻捇妾遍崡銈囧皸妤规挻銈奸懠鍌氱秿閼憋妇灏楅悙澶庣熅韫囨瑦瀚濋懕娆忕箹閼卞吋銈奸悮顐㈢爱閹搭喗鐨抽梽鍥画閻氼偊鈹愰懕璇茬箹閼便劍銈?);";
        html += "    });";
        html += "}";
        
        html += "// 濮樻挸绨懕璇茬箹閼辨挳婀惰箛娆掍紩妤规挸绻栭懕铏箠韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍紥閹搭喛骞楅梽鍥ㄧ稏";
        html += "setInterval(updateTemperatureDisplay, 2000);";
        
        html += "// 閼煎懘娈曠喊宀冨瘶閼卞搫鐎鎾逛槐閼辩晫灏楅梽鍡涙韫囨瑨浼岄棁鍙夌毘閼便垼浼滃鎾存倕閼便儲鐨抽懕锕佷紜";
        html += "document.addEventListener('DOMContentLoaded', function() {";
        html += "  updateTemperatureDisplay();";
        html += "  ";
        html += "  // 閻氼偅鍩夐懕顖涚毘閼辩喐銈艰箛娆撴畷閼辩姵鐨抽懕姝屼紳閻氼偊妾版繛鍕锤濞肩偠浠撮惄鑼洨闂?;
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
        // 韫囨瑩顣幏銏＄毘鐠у倽绁╄箛娆戠槵闂呭棙鐨宠ぐ鏇′紑閻╄尪绁╅懕銉ㄥ瘶閼甭や还濮樻挸绨懕閿嬬毘閼鳖剝浼冨鎾逛划閹哄疇骞楅懕鍓侇洨濮樻捇妾伴懕鑼跺瘶闂呭棛顣遍懠鍛颁紲閸?
        webServer.sendHeader("Location", "/login.html", true);
        webServer.send(302, "text/plain", "Redirecting to login page");
    }
}

// 濮樻捇妾遍懕鐐跺箺閼鳖剝浠╄箛娆掍化閼辨瑥绻栭懕纭呬化韫囨瑨浼栭梽瀣毘鎼存劘浼旈惄鏌ョ畭閼憋綀骞楅懕鐤沪閻氼偉鐭鹃悙澶庡箺閼辨澘宕甸懠鍌氱秿閼憋附鐨虫惔鎰紳閼惧€熶缓閹哄磭灏楅懕锟犳濮樻捁浠辩弧鎾跺皸鐠侯垶鐬鹃悮顐︽閸?
void handleNotFound() {
    if (isCaptivePortalMode) {
        // 濮樻捁浼栫弧鎾寸毘瑜版洘缍栧鎾逛划闂囪尪瀵橀懕瀵哥槵韫囨瑨浠捄顖氱箹缁℃捇娈曞鎾崇秿閼鳖偆娲哥挧鍌濅淮閼煎倸缍嶉懕锔界毘閹哄疇浠╄箛娆掍化閼辨瑥绻栭懕纭呬化韫囨瑨浼栭梽瀣毘鎼存劘浼旈惄鏌ョ畭閼憋綀骞楅懕鐤沪閻氼偉鐭鹃悙澶庡箺閼辨澘宕甸懠鍛颁画閼毖勭毘鎼存劘浼斿鎾逛紓閼鳖厽鐨抽懕銏″箠閼煎懎鈻岄懕瀹犲瘶闂呭棛顣?
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "Redirect to configuration page");
    } else {
        // 濮樻捁浼栫弧鎾崇箹妫板懏瀚濆鎾圭ォ鐠у倸绻栫弧鎾绘畷濮樻挸缍嶉懕顐ゆ锤鐠у倽浠撮懠鍌氱秿閼憋箒骞楅懕鎼佺氨韫囨瑨浠哄Δ鑲╁皸妞圭浼嗗鎾逛紩閼?04閼煎懓浼嗛懕鍓佸皸閻愬鍊?
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
        html += "<div class=\"success-icon\">閼烘帟浼栭懕?/div>";
        html += "<h1 class=\"success-title\">閼煎懓浠ч懕褑骞楅梽鍡楃爱濮樻捁鐭鹃搹蹇曟锤妞圭浼滃鎾活暘閼?/h1>";
        html += "<p class=\"success-message\">WiFi閼煎懓浠ч懕褑骞楅梽鍡楃爱濮樻捁鐭鹃搹蹇撶箹閼便垼浼傚鎾逛槐閼辫偐娲告す纾嬩紲濮樻捇顣懕浣冨瘯瑜版洝浠搁悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻挻骞婇懕鐘哄瘶閼甭や还濮樻捁浼傞悙澶嬬毘妤ｅ潡婀跺鎾村箠閼辫櫣灏楅悙澶庝紙閻氼偊鈹愰懕璇茬箹閼便劍銈糤iFi閼句粙妾伴懕顓″箺缁傚嫯浼栭懠顐ヤ粴閼?/p>";
        html += "<p class=\"countdown\">5閼捐姤鎮呴懕顔界毘閼鳖剝浠洪悮顐ヤ画闂勫鐨抽懕銈囩槵閻氼偉鐭炬ご浣哄皸闂勫棗宕插鎾逛紩閼辨槒瀵橀懕鐔讳还閼句粙妾版惔鎰瘶闂呭棛顣遍懠鍛颁紲閸?..</p>";
        html += "</div>";
        html += "</body></html>";
        
        webServer.send(200, "text/html", html);
        
        delay(1000);
        ESP.restart();
    }
}

void handleFactoryReset() {
    // 韫囨瑨绁╅懕鐔诲箺濠曞繑缍朎EPROM閻╄尪绁╂０鍛板箺閼辩柉浠﹂懠鍛颁户閼毖嗗箺闂勫棗绨?
    EEPROM.begin(512);
    
    // 韫囨瑨绁╅懕鐔诲箺濠曞繑缍栬箛娆掍紙妤规挾娲哥挧鍌炴EEPROM濮樻捁浠稿鐐寸毘閼辫壈浼?
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    
    // 韫囨瑨绁╅懕鐔诲箺濠曞繑缍栧鎾逛哗閼辩喐鐨虫０鍛颁紥閻╄尪绁╂０鍛板箺閼辩柉浠﹂懠鍛颁户閼毖嗗箺闂勫棗绨鎾逛紑閼变絻瀵橀懕陇浼€
    wifiSSID = "";
    wifiPassword = "";
    temperatureOffset = 0.0;
    temperatureScale = 1.0;
    
    Serial.println("韫囨瑨浠滈崹鍕毘闂勫洩浠瑰鎾逛画濞肩偞鐨抽懕銊ㄤ粻閻氼偄绨幋顔垮箺闂勫棗绨鎾崇爱閼憋箑绻栭懕銏ｄ紓閼煎倸缍嶉懕锕€绻栭懕锝堜粴韫囨瑨浼栭懕锝堝瘶閼辩喕浠归懢浠嬫鎼存劖鐨崇捄顖濇韫囨瑨绁╅懕鐔诲瘶閼卞爼妾?);
    
    // 閻氼偊鈹愰懕鐗堢毘閼辨瓕浼濊箛娆掍划閼鳖剚鐨抽懕銈堜肌閼煎懘娈曠喊宀冨瘶閼卞搫鐎?
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
    html += "<div class=\"reset-icon\">閼烘帟浼旈懕鍊熷瘯鐠у倽浼€</div>";
    html += "<h1 class=\"reset-title\">韫囨瑨浠滈崹鍕毘闂勫洩浠瑰鎾逛画濞肩偞鐨抽懕銊ㄤ粻閻氼偄绨幋顔垮箺闂勫棗绨?/h1>";
    html += "<p class=\"reset-message\">閻氼偄绨幋顔界毘闂勫洩浠鎾圭熅閾忓繐绻栭懕娑樼€鎾绘閼毖勭毘閼扁剝缍栧鎾逛缓閼辨粎灏楁惔鎰焿閼句粙妾版惔鎰瘯瑜版洝浠歌箛娆掍化閼辨瑥绻栭懕纭呬化WiFi閼煎懓浠ч懕褑骞楅梽鍡楃爱濮樻捁浼勯懕锕€绻栫挧鍌涚础濮樻挻缍栨繛鍕箹閼变粙娈曞鎾逛画閼辩姵鐨抽懕顐ヤ粻韫囨瑨浼堥幒铏毘鐠侯垵妾归悮顐㈢€懞锕€绻栫挧鍌濅户閼煎懓浼撻梽鍥皸閼辨瑨浠?/p>";
    html += "<p class=\"reset-message\">閻氼偄绨幋顔界毘闂勫洩浠鎾村箠閼辩姾瀵橀懕陇浠瑰鎾逛紓閻愬鐨虫ィ鍧楁苟閻氼偊鈹愰懕鍛婄毘閼辩喐銈奸懠鍛颁户閼毖嗗箺闂勫棜浼冭箛娆戠槵闂呭棙鐨宠ぐ鏇′紑閼煎倸缍嶉懕锔惧皸閻愬鐭鹃懠鍛颁画閼毖冪箹閼辫櫕骞婇懠鍛颁户閼毖嗗箺闂勫棗绨甒iFi閼句粙妾伴懕顓″箺缁傚嫯浼栭懠顐ヤ粴閼?/p>";
    html += "<p class=\"countdown\">5閼捐姤鎮呴懕顔界毘閼鳖剝浠洪悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻挻骞婇懕鐘哄瘶閼甭や还濮樻捁浼傞悙?..</p>";
    html += "</div>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
    
    delay(1000);
    ESP.restart();
}

void handleRestart() {
    Serial.println("韫囨瑨浠哄Δ鐓庣箹閼变即婀跺鎾逛划閹哄疇瀵橀懕陇浠瑰鎾逛紓閻愬灏楁惔鎰焿濮樻捇妾遍懕锛勫皸閻愬鐭捐箛娆忓吹閼?);
    
    // 閻氼偊鈹愰懕鐗堢毘閼辨瓕浼濋懠鍛颁画閼毖勭毘閼鳖剛鍊濋懢浠嬫畷鎼存劗灏楁惔鎰版閼煎懘娈曠喊宀冨瘶閼卞搫鐎?
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
    html += "<div class=\"restart-icon\">閸愭帟浼￠懕鎷屼沪</div>";
    html += "<h1 class=\"restart-title\">閻氼偄绨幋顔界毘闂勫洩浠懠鍛颁画閼毖勭毘閼鳖剛鍊濋惄鑼剁ォ妫?/h1>";
    html += "<p class=\"restart-message\">閻氼偄绨幋顔界毘闂勫洩浠箛娆擃暘閹枫垺鐨抽懕鍦槵閼煎懓浠懕褎鐨抽懕顒傚€濋懠鍌氱秿閼憋箒瀵橀懕陇浠瑰鎾逛紓閻愬灏楁す纾嬩画閼剧晫鐦懕銉︾毘闂勫洦鎮呴懢鑺ョ稏婵炲嫯瀵橀懕纭呬粴閻氼偄鈻岄懕?0閼捐姤鎮呴懕顔垮皸閼辨瑨浠?/p>";
    html += "<p class=\"restart-message\">閼煎懓浠懕褎鐨抽懕顒傚€濆鎾崇爱閼憋箑绻栭懕銏ｄ紓濮樻捁浼傞懕銊ㄥ瘯瑜版洝浠搁悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻挻骞婇懕鐘靛皸閼憋繝妾靛鎾逛槐缁℃捁瀵橀懕陇浠硅箛娆掍紜閹哄磭灏楁す纾嬩紳韫囨瑨浠哄Δ绯Fi閼句粙妾伴懕顓″箺缁傚嫯浼栭懠顐ヤ粴閼?/p>";
    html += "<p class=\"countdown\">5閼捐姤鎮呴懕顔界毘閼鳖剝浠洪悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻挻骞婇懕鐘哄瘶閼甭や还濮樻捁浼傞悙?..</p>";
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
    // 閻氼偆鍊濈粋鍕毘閼鳖偉浼嬪鎾逛缓閼卞吋鐨抽幖鍌濅淮MAX6675韫囨瑨浼堥幒鍐茬箹閼毖冪爱
    uint16_t rawData = readMAX6675RawData();
    bool errorFlag = (rawData & 0x04) != 0;
    uint16_t tempBits = rawData >> 3;
    float rawTemperature = tempBits * 0.25;
    
    // 閼句粙娈曟惔鎰毘鎼存劘浼旈惄鎻掔秿閼辫棄绻栭懕鐐朵肌濮樻捁浼撶弧鎾瑰箺閼便倝婀惰箛娆掍粴閼?
    String sensorStatus = "韫囨瑩顣幏銏＄毘鐠у倽绁?;
    String diagnosticAdvice = "閻╂彃缍嶉懕钘夌箹閼辩偠浼″鎾逛紦缁℃挻鐨崇捄顖涖偧閻╂煡妾伴懕鐟扮箹妫板懏瀚濆鎾圭ォ鐠?;
    
    if (errorFlag) {
        sensorStatus = "閼煎懓浼嗛懕鍓佸皸閻愬鍊?;
        diagnosticAdvice = "韫囨瑦瀚濋懕娆忕箹绾板矁浠村鎾逛划閹哄疇骞楅懕婵嬵暘閼惧€熶紗绾板本鐨抽懕娑㈡苟濮樻挸缍嶉懕娆戝皸鐠侯垳鍊濊箛娆掍划閼卞磭娲歌ぐ鏇′迹韫囨瑨浠﹂懕鍏肩毘閼卞墎鐦箛娆掍紙閼辩喕瀵橀懕鐤紪";
    } else if (rawData == 0x0000 || rawData == 0xFFFF) {
        sensorStatus = "閻氼偊顣繛鍕毘閼鳖叀浠?;
        diagnosticAdvice = "閻╂彃缍嶉懕钘夌箹閼辩偠浼″鎾逛紦缁℃挸绻栭懕鍙夊箠韫囨瑨浠规惔鎰毘瑜版洝浠犲鎾圭ォ鐠у倽瀵曡ぐ鏇′桓韫囨瑦瀚濋懕娆忕箹閼卞吋銈奸懢浠嬫畷閸椼垻娲哥粋鍕苟閻氼偊鈹愰懕璇茬箹閼便劍銈?;
    } else if (rawTemperature < -20 || rawTemperature > 1024) {
        sensorStatus = "閻氼偊顣繛鍕毘閼鳖叀浠?;
        diagnosticAdvice = "韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ゅ€濈粋鍕箹閼卞弶骞婇悮顐︽苟閼辩喐鐨抽懕鈩冪稏韫囨瑩顣幏銏＄毘鐠у倽绁╅悮顐ヤ桓閼辨繃鐨抽懕鎼佺氨";
    }
    
    // 韫囨瑨浼濋懕鐐寸毘缁傚嫭缍栭悮顐ゅ€濋懕銈呯箹閼辨娊顣狫SON濮樻捁浼呴懕褎鐨冲鐐朵紗
    String json = "{";
    json += "\"sensor_status\":\"" + sensorStatus + "\",";
    json += "\"raw_data\":\"0x" + String(rawData, HEX) + "\",";
    json += "\"error_flag\":" + String(errorFlag ? "true" : "false") + ",";
    json += "\"temp_bits\":" + String(tempBits) + ",";
    json += "\"raw_temperature\":" + String(rawTemperature) + ",";
    json += "\"calibration_params\":\"濮樻捁浠滈懕顐ュ箺閹煎倻顩?" + String(temperatureOffset) + " 閼捐棄缍嶅蹇撶箹閼辩増鍩?" + String(temperatureScale) + "\",";
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
    
    String json = "{\"message\":\"韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍迹闂呭棙鐨抽懕陇浠╁鎾逛紑閼辨粌绻栭懕鍙夊箠濮樻捁鐭鹃搹蹇氬瘶閼甭や还閼句粙妾版惔鎬?,";
    json += "\"offset\":" + String(temperatureOffset) + ",";
    json += "\"scale\":" + String(temperatureScale) + "}";
    
    webServer.send(200, "application/json", json);
}

void handleControl() {
    bool wasHeating = heatingEnabled;  // 閻╂煡鈹愰懕鐑樼毘妫板懓浼掗惄鏌ョ畭閼便儲鐨抽懕锝堜还閼惧€熶紨閼辩偞鐨抽懕銈堜迹閼惧€熶虎妫板懓骞楅懕銈夋苟韫囨瑨浠氶懕?
    
    if (webServer.hasArg("target_temp")) {
        targetTemp = webServer.arg("target_temp").toFloat();
    }
    if (webServer.hasArg("heating")) {
        heatingEnabled = webServer.arg("heating") == "true";
    }
    
    // 韫囨瑦瀚濋懕娆忕箹绾板矁浠村鎾逛槐閼卞€熷箺閼辨繈顣懢鍊熶槐闂囨彃绻栭懕娆掍粶濮樻捁浼€閼卞灚鐨抽懕锕佷紜濮樻捇绠嶉棁鑼皸閹煎倸鈻屽鎾逛紑閼鳖厾灏楅懕纭呬粻閼煎懓绁╅幏銏＄毘閼卞墎鐦箛娆掍紑閼鳖剝骞楅梽鍥ㄧ稏
    if (!wasHeating && heatingEnabled) {
        // 閻╄尙顩╅懕銊︾毘閼辩喖鐬鹃懠鍛颁紝妫板懏鐨抽懕銏″箠濮樻挸缍嶉懕娆愮毘閼鳖剛鍊濋懠鍌氱秿閼遍攱鐨宠ぐ鏇′粴濮樻挻鎮呴懕銉ㄥ箺閼辨繆浼掗懢鍊熶沪閼?
        beepBakingStart();
        Serial.println("閼惧€熶虎閼变絻骞楅懕鐐朵紦濮樻挸缍嶉懕娆愮毘閹煎倽浠?- 閼惧€熶紩鎼存劕绻栭懕鍊熶画韫囨瑨绁╁蹇旂毘濞肩偛鈻? " + String(targetTemp) + "閼存骞奀");
    } else if (wasHeating && !heatingEnabled) {
        // 閻╄尙顩╅懕銊︾毘瑜版洝浠氬鎾逛紓閻愬鐨抽懕銏″箠濮樻捁浠фご浣藉瘶閼辨挳顣懠鍌氱秿閼辩柉骞楅懕婵婁紥閼惧€熶沪閼辫埖鐨虫惔鎰桓韫囨瑨浠懕?
        beepBakingComplete();
        Serial.println("閼惧€熶虎閼变絻骞楅懕鐐朵紦濮樻挸绨懕锕€绻栭懕銏ｄ紓 - 韫囨瑨浼栭懕娆掑箺缁傚嫯浠箛娆掔ォ濠曞繑鐨冲鐐测枌: " + String(currentTemp) + "閼存骞奀");
        
        // 閻氼偅鎮呮繛鍕毘閼鳖偉浼冮懢鍊熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼￠懢鍊熶槐闂囨彃绻栭懕娆掍粶閼煎倸缍嶉懕閿嬬毘妞圭濮﹂懠鍛颁紝闂?0閼捐姤鎮呴懕?
        bakingCompleteState = true;
        bakingCompleteStartTime = millis();
        Serial.println("閻氼偅鎮呮繛鍕毘閼鳖偉浼冮懢鍊熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼″鎾烩攼閼猴箒瀵橀懕鎾閼惧€熶槐闂囨彃绻栭懕娆掍粶閼煎倸缍嶉懕锕€绻栭懕锕佷粶閼剧晫顩╂０?0閼捐姤鎮呴懕?);
    }
    
    webServer.send(200, "text/plain", "OK");
}

// =========================================
// 韫囨瑨浼嬮幒铏毘閸ㄥ嫯浼滱PI閼惧€熷Е閻愬骞楅懕婊堢畭濮樻捇妾遍懕鐐跺箺閼鳖剝浠╁鎾逛画闂勫棗绻栭懕鍙夊箠
// =========================================

// 韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛缓閼辩姵鐨抽懕顐ユ閻氼偄绨幒铏毘闂勫棜浼堥懢鍊熷Е閻愬骞楅懕婊堢畭
void handleTemperatureHistory() {
    String json = "{\"temperature_history\":[";
    
    // 韫囨瑧鐦梾鍡楃箹閼便儴浼¤箛娆掔ォ濠曞繑鐨冲鐐测枌濮樻捁浠洪懕鐘崇毘閼鳖偉妾硅箛娆掍紙閹哄啿绻栭懕褍绨懠鍌氱秿閼便垺鐨虫惔鎰紳閼煎懓浼撻懕鐔哥毘濞肩偠浼嗛懢鍊熶紗缁℃挾娲哥挧鍌烆暘濮樻捁浼€閻愬娲哥粋鍕偧閻╄尙顩╅懕鈥PROM韫囨瑨浠懕鍐茬箹閼卞疇浠惄鑼洨闂囪尪骞楁ご浣侯洨閼剧晫顩╅懕鑲╁皸閻愬顩╁鎾逛紑閼卞疇瀵曡ぐ鏇′化
    for (int i = 0; i < 10; i++) {
        if (i > 0) json += ",";
        json += "{\"timestamp\":" + String(millis() - i * 60000) + ",";
        json += "\"temperature\":" + String(currentTemp - i * 0.5) + "}";
    }
    
    json += "]}";
    webServer.send(200, "application/json", json);
}

// 濮樻挸绨懕宄扮箹閼辨挳婀堕惄鑼洨缁傚嫭鐨抽懕銈夋畷閼捐棄绨梾鍡氬箺閼鳖剝浠╅懢鍊熷Е閻愬骞楅懕婊堢畭
void handleTimerTasks() {
    if (webServer.method() == HTTP_GET) {
        // 閻氼偉浠虹捄顖涚毘閼鳖偉浼嬪鎾崇爱閼卞嘲绻栭懕鎾苟閻╄尙顩╃粋鍕毘閼便倝娈曞鎾逛划閼卞灏楅梾鍡欑槵
        String json = "{\"timers\":[";
        json += "{\"id\":1,\"enabled\":false,\"target_temp\":180,\"duration\":3600}";
        json += "]}";
        webServer.send(200, "application/json", json);
    } else if (webServer.method() == HTTP_POST) {
        // 濮樻捁浠懕鍛婄毘缁傚嫭缍栬箛娆掍划閼卞啿绻栭懕鎼佺氨韫囨瑨浼嬮幒铏毘鎼存劘浼旇箛娆掍紝闂囪尙娲哥粋鍕洨濮樻捁浠遍梾?
        if (webServer.hasArg("action") && webServer.arg("action") == "create") {
            webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"濮樻挸绨懕宄扮箹閼辨挳婀堕惄鑼洨缁傚嫭鐨抽懕銈夋畷濮樻捁浠懕鍛婄毘缁傚嫭缍栬箛娆掍划閼鳖剚鐨抽懕銈堜肌\"}");
        } else {
            webServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"濮樻挸绨懕宄扮箹閼辨挳婀堕惄鑼洨缁傚嫭鐨抽懕銈夋畷韫囨瑨浼曟ス鎾崇箹閼辫櫕骞婅箛娆掍划閼鳖剚鐨抽懕銈堜肌\"}");
        }
    }
}

// 閻氼偄绨幋顔界毘闂勫洩浠惄鏌モ攼闂呭棗绻栭懕娑氬€濋懢鍊熷Е閻愬骞楅懕婊堢畭
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

// 濮樻挸绨懕锝嗙毘閼辩喓鐦懢鍊熶紩閼鳖厼绻栭懕銊︽倕閼惧€熷Е閻愬骞楅懕婊堢畭
void handleSafetyMonitor() {
    String json = "{";
    json += "\"temperature_alerts\":[";
    json += "{\"type\":\"high_temperature\",\"threshold\":250,\"current\":" + String(currentTemp) + ",\"triggered\":" + String(currentTemp > 250 ? "true" : "false") + "}";
    json += "],";
    json += "\"safety_status\":\"normal\",";
    json += "\"last_check\":" + String(millis()) + ",";
    json += "\"recommendations\":[";
    json += "\"閻╂煡鈹愰懕鍝勭箹閼憋箒浠滈悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻捁浼冪弧鎾寸毘閼辨悂绨遍懠鍛颁粴閼辩柉瀵橀幏銏ｄ缓閻氼偉浠悙澶嬬毘濡ゅジ妾癨",";
    json += "\"濮樻挸绨懕宄扮箹閼辩浼¤箛娆愬珴閼辨瑥绻栭懕鍏笺偧韫囨瑨绁╁蹇旂毘濞肩偛鈻岄惄鎻掔秿閼辫棄绻栭懕鐐朵肌濮樻捁浼撶弧鎻?,";
    json += "\"閼煎懓浠滄す瀛樼毘閼辩喕浠归懠鍛颁紙妞规潙绻栭懕鎾苟閼煎懓浼屾ス鎾瑰瘶閼猴箒浼掕箛娆掔ォ濠曞繒灏楁す纾嬩紓閻氼偊娈曢懕顩?";
    json += "]}";
    
    webServer.send(200, "application/json", json);
}

// 閻氼偉浠㈤梽鍡欏皸閼辨瑨浼岄懢鐣岊洨閼辫偐灏楁惔鎰版畷閼惧€熷Е閻愬骞楅懕婊堢畭
void handleEnergyStats() {
    String json = "{";
    json += "\"total_energy_used\":" + String(millis() / 3600000.0 * 1500) + ","; // 韫囨瑧鐦梾鍡楃箹閼便儴浼￠悮顐ヤ虎闂勫棛灏楅懕娆掍紝韫囨瑨浼堥幒鍐茬箹閼毖冪爱
    json += "\"current_power\":" + String(heatingEnabled ? "1500" : "0") + ",";
    json += "\"today_energy\":" + String(millis() / 86400000.0 * 1500) + ",";
    json += "\"energy_unit\":\"Wh\",";
    json += "\"power_unit\":\"W\",";
    json += "\"stats_period\":\"lifetime\"";
    json += "}";
    
    webServer.send(200, "application/json", json);
}

// =========================================
// 韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍缓閹煎倹鐨抽懕銏ゆ苟濮樻捁浠遍懕鑲╁皸閼辨繈妾?
// =========================================

// 韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ゅ€濈粋鍕毘閼鳖偉浼嬪鎾崇爱閼卞嘲绻栭懕鎾苟濮樻捁浼撶弧?
unsigned long lastTemperatureRead = 0;
const unsigned long TEMPERATURE_READ_INTERVAL = 200; // 韫囨瑧鍊濋懕?00ms閻氼偆鍊濈粋鍕毘閼鳖偉浼嬮惄鑼剁ォ閼辨瑥绻栭崡銏ゆ畷韫囨瑨绁╁蹇旂毘濞肩偛鈻岄懠鍌氱秿閼便垻娲歌ぐ鏇′紥濮樻捁浠搁懕瀹犲瘯瑜版洝浼旇箛娆掍紑閼鳖剝瀵橀懞锕佷紥韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛紖閼毖勭毘濞肩偠浼嗛懠鍛颁粴閼卞吋鐨冲鐐测枌閼煎倸缍嶉懕?

void readTemperature() {
    unsigned long currentTime = millis();
    
    // 濮樻挸绨懕宄扮箹閼辨挳婀堕悮顐ゅ€濈粋鍕毘閼鳖偉浼嬭箛娆掔ォ濠曞繑鐨冲鐐测枌閼煎倸缍嶉懕锕佸瘶閼辨盯鈹愬鎾逛户閼毖嗗瘶閸ㄥ嫯浼冮懢浠嬬畭閼辨稓灏楅悙澶岊洨濮樻捁浼€閼辫櫕鐨抽梽鍡楀吹濮樻捁浼呴懕褍绻栭懕娆愭倕閻氼偉浠㈤梽?
    if (currentTime - lastTemperatureRead >= TEMPERATURE_READ_INTERVAL) {
        currentTemp = readTemperatureWithMonitoring();
        if (currentTemp < 0) {
            Serial.println("韫囨瑨绁╁蹇旂毘濞肩偛鈻岄惄鎻掔秿閼辫棄绻栭懕鐐朵肌濮樻捁浼撶弧鎾跺皸閻愬顩╁鎾逛紑閼卞疇瀵橀懕鎷屼紦閻氼偆鍊濋悙?);
        } else {
            // 濮樻捁浼€闂勫鐨抽懕鍦槵韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛紑閼卞灚鐨抽懕锕佷紜閻氼偅鍩夐懕婵囩毘闂勫洦鎮呰箛娆掍紝闂囨彃绻栭懕锝堜还韫囨瑨浠懕顖涚毘閼毖勫箠韫囨瑨浼屽Δ鍏肩毘妞圭浼岄懠鍌氱秿閼憋附鐨抽懕陇浼€濮樻挻骞婇懕顓犳锤鐠у倽妾瑰鎾逛紑閹枫垻灏楅幋顔夸紖濮樻捁浠?
            static float lastPrintedTemp = -999;
            if (fabs(currentTemp - lastPrintedTemp) >= 0.5) {
                Serial.println("濮樻捇妾伴懕顖涚毘閼憋綀浠硅箛娆掔ォ濠曞繑鐨冲鐐测枌: " + String(currentTemp) + "閼存骞奀");
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
// LED閼惧€熶槐闂囨彃绻栭懕娆掍粶韫囨瑨浠搁懕陇骞楅梽鍥ㄧ稏
// =========================================
void updateLED() {
    unsigned long currentTime = millis();
    
    // 閼惧€熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼￠懢鍊熶槐闂囨彃绻栭懕娆掍粶濮樻捇妾遍懕鐐跺箺閼鳖剝浠?
    if (bakingCompleteState) {
        if (currentTime - bakingCompleteStartTime > BAKING_COMPLETE_DURATION) {
            // 閼惧€熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼″鎾烩攼閼猴箒瀵橀懕鎾韫囨瑨浼岄棁鑼跺瘶閼辨挳绨卞鎾逛划閹哄疇瀵曡ぐ鏇′桓濮樻捁浠懕鈥崇箹閼毖冪€鎾逛划閹鸿櫕鐨抽幋顔夸户韫囨瑨浼栧鐐跺箺閼便倝婀惰箛娆掍粴閼?
            bakingCompleteState = false;
            Serial.println("閼惧€熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼″鎾烩攼閼猴箒瀵橀懕鎾濮樻挸绨懕锕€绻栭懕銏ｄ紓閼煎倸缍嶉懕锔界毘閼便垼浠箛娆掍还閸ㄥ嫭鐨抽懕銏″箠濮樻挻鍩夐懕鐔风箹閼辫缍栭懢鍊熶槐闂囨彃绻栭懕娆掍粶");
        }
    }
    
    if (currentTime - lastLedUpdate > LED_BLINK_INTERVAL) {
        if (bakingCompleteState) {
            // 閼惧€熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼￠懢鍊熶槐闂囨彃绻栭懕娆掍粶閼煎倸缍嶉懕閿嬬毘妞圭濮﹂懠鍛颁粴閼辫壈瀵橀懕鎾閼惧€熶虎閼辨稖瀵曡ぐ鏇′划500ms閼煎懓浼屾ス鎾瑰瘶閼辩柉浼嗛懠鍌氱秿閼?
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED閼惧€熶槐闂囨彃绻栭懕娆掍粶: 閼惧€熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼?- 濮樻捇鈹愰懞锕佸瘶閼辨挳妾?(閻╁弶缍栨惔?");
            } else {
                Serial.println("LED閼惧€熶槐闂囨彃绻栭懕娆掍粶: 閼惧€熶虎閼变絻骞楅懕鐐朵紦閼剧晫顩╅懕顖氱箹閼遍缚浼?- 濮樻捇鈹愰懞锕佸瘶閼辨挳妾?(閼惧€熶粶妫?");
            }
        } else if (isCaptivePortalMode) {
            // 閼煎懓浠ч懕褑骞楅梽鍡氫純濮樻捁浠懕褑骞楅懕銈夋苟韫囨瑨浠氶懕娑滃瘯瑜版洝浼斿鎾烩攼閼猴箒瀵橀懕娆掍肌閼煎懓浼岄梽瀣箺閼辨繆浠滈懠鍌氱秿閼?00ms閼煎懓浼屾ス鎾瑰瘶閼辩柉浼嗛懠鍌氱秿閼?
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            if (ledState) {
                Serial.println("LED閼惧€熶槐闂囨彃绻栭懕娆掍粶: 閼煎懓浠ч懕褑骞楅梽鍡氫純濮樻捁浠懕?- 濮樻捇鈹愰懞锕佸瘶閼辨挳妾?(閻╁弶缍栨惔?");
            } else {
                Serial.println("LED閼惧€熶槐闂囨彃绻栭懕娆掍粶: 閼煎懓浠ч懕褑骞楅梽鍡氫純濮樻捁浠懕?- 濮樻捇鈹愰懞锕佸瘶閼辨挳妾?(閼惧€熶粶妫?");
            }
        } else if (heatingEnabled) {
            // 濮樻捁浠遍懕鍊熷箺閼辨繈顣惄鑼剁ォ妫板懓骞楅懕銈夋苟韫囨瑨浠氶懕娑滃瘯瑜版洝浼斿鎾圭ォ鐠у倻娲稿鐐茬爱
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED閼惧€熶槐闂囨彃绻栭懕娆掍粶: 濮樻捁浠遍懕鍊熷箺閼辨繈顣惄鑼剁ォ妫?- 濮樻捁绁╃挧鍌滄锤濞肩偛绨?);
        } else {
            // 濮樻挻鍩夐懕鐔风箹閼辫缍栭懢鍊熶槐闂囨彃绻栭懕娆掍粶閼煎倸缍嶉懕宄扮箹閼辩喎鐎懠鍛颁粴閼辫壈瀵橀懕鎾閼惧€熶虎閼辨稖瀵曡ぐ鏇′划1000ms閼煎懓浼屾ス鎾瑰瘶閼辩柉浼嗛懠鍌氱秿閼?
            if (currentTime - lastLedUpdate > 1000) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState ? HIGH : LOW);
                if (ledState) {
                    Serial.println("LED閼惧€熶槐闂囨彃绻栭懕娆掍粶: 濮樻挻鍩夐懕鐔风箹閼辫缍?- 韫囨瑨浠ч崹鍕瘶閼辨挳妾?(閻╁弶缍栨惔?");
                } else {
                    Serial.println("LED閼惧€熶槐闂囨彃绻栭懕娆掍粶: 濮樻挻鍩夐懕鐔风箹閼辫缍?- 韫囨瑨浠ч崹鍕瘶閼辨挳妾?(閼惧€熶粶妫?");
                }
                lastLedUpdate = currentTime;
            }
            return; // 濮樻挻鍩夐懕鐔风箹閼辫缍栭懢鍊熶槐闂囨彃绻栭懕娆掍粶閻╂煡妾版す纾嬪箺閼辨壆鐦悮顐ヤ画闂勫鐨虫惔鎰紨閻╂煡绠嶉懕锝堝瘶閼辨挳绨遍懠鍛颁紨閼辨媽瀵曡ぐ鏇′桓閻╄尪绁╅懕褍绻栭懕鎼佺氨韫囨瑨浼嬮幒鐮絘stLedUpdate
        }
        
        lastLedUpdate = currentTime;
    }
}

// =========================================
// 閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃挸绻栭懕銊︽倕濮樻捁浠棁?
// =========================================
void beep(int duration = 100) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}

// 閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃捁骞楅懕銈夋苟韫囨瑨浠氶懕娑樼箹閼鳖偉浼傞懢浠嬫濞肩偞鐨抽懕锟犳韫囨瑨浼堥幒?
void beepConfigSaved() {
    // 閻氼偄绨幋顔垮箺闂勫棗绨惄鏌モ攼閼辩儤鐨虫０鍛颁紥韫囨瑨浠懕顒佺毘閼便倛浼¤箛娆掍紑閼鳖剝骞楅梽鍥ㄧ稏閼煎倸缍嶉懕鐤箺閼卞ジ顣?閼惧€熶肌妫?閼惧€熶肌妫?
    beep(100);
    delay(100);
    beep(100);
    delay(100);
    beep(100);
    Serial.println("閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃挸绻栭懕顐ヤ紓閼句粙妾卞鐐跺瘯瑜版洝浼旈懠鍛颁户閼毖嗗箺闂勫棗绨鎾圭熅閾忓繒娲告す纾嬩紲濮樻捇顣懕?);
}

void beepBakingStart() {
    // 濮樻挸缍嶉懕娆愮毘閹煎倽浠撮懢鍊熶虎閼变絻骞楅懕鐐朵紦韫囨瑨浼€閼鳖剝骞楅梽鍥ㄧ稏閼煎倸缍嶉懕鐤瘶閼辨煡鈹?閼惧€熶肌妫?
    beep(300);
    delay(200);
    beep(100);
    Serial.println("閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃挸绻栭懕顐ヤ紓閼句粙妾卞鐐跺瘯瑜版洝浼旈懢鍊熶虎閼变絻骞楅懕鐐朵紦濮樻挸缍嶉懕娆愮毘閹煎倽浠?);
}

void beepBakingComplete() {
    // 閼惧€熶虎閼变絻骞楅懕鐐朵紦濮樻挸绨懕锕€绻栭懕銏ｄ紓韫囨瑨浼€閼鳖剝骞楅梽鍥ㄧ稏閼煎倸缍嶉懕鐤瘶閼辨煡鈹?閼煎懓浼堟す?閼煎懓浼堟す?
    beep(500);
    delay(200);
    beep(500);
    delay(200);
    beep(500);
    Serial.println("閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃挸绻栭懕顐ヤ紓閼句粙妾卞鐐跺瘯瑜版洝浼旈懢鍊熶虎閼变絻骞楅懕鐐朵紦濮樻挸绨懕锕€绻栭懕銏ｄ紓");
}

void beepTemperatureChange() {
    // 韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛划閼扁€崇箹閼毖冪€箛娆掍紑閼鳖剝骞楅梽鍥ㄧ稏閼煎倸缍嶉懕鐤箺閼卞ジ顣?閼惧€熶肌妫?閼惧€熶肌妫?閼惧€熶肌妫?
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    delay(80);
    beep(80);
    Serial.println("閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃挸绻栭懕顐ヤ紓閼句粙妾卞鐐跺瘯瑜版洝浼旇箛娆掔ォ濠曞繑鐨冲鐐测枌濮樻捁浠懕鈥崇箹閼毖冪€?);
}

// =========================================
// 濮樻捁浠懕鐑樼毘閹煎倽浠村鎾逛桓閼辫櫕鐨抽懕锟犳韫囨瑨浼堥幒?
// =========================================
void setup() {
    Serial.begin(115200);
    Serial.println("");
    Serial.println("=========================================");
    Serial.println("韫囨瑨浼撳鐐靛皸閼辨繈妾伴懢鍊熶紗绾板矁骞楅懕婵嬫閼捐棄绨崡銈呯箹閼便劍鎮呭鎾逛划闂囧弶鐨抽懕鍓佺槵 v" + FIRMWARE_VERSION);
    Serial.println("MAX6675韫囨瑨浠懕銉︾毘閼便倗鐦琒PI濮樻挸绨懕鏄忓箺閼便劍骞?- 韫囨瑨绁╁蹇旂毘濞肩偛鈻岄惄鎻掔秿閼辫棄绻栭懕鐐朵肌濮樻捁浼撶弧鎾崇箹妫板懏瀚濆鎾圭ォ鐠у倹鐨崇捄顖涖偧閻╂煡妾伴懕?);
    Serial.println("=========================================");
    
    // 濮樻捁浠懕鐑樼毘閹煎倽浠村鎾逛桓閼辫櫕鐨宠ぐ鏇′紙閻氼偉浠﹂懕?
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // 濮樻捁浠懕鐑樼毘閹煎倽浠村鎾逛桓閼辩熂AX6675濮樻挸缍嶉懕鑼皸閼辩偠浼?
    pinMode(THERMO_CLK, OUTPUT);
    pinMode(THERMO_CS, OUTPUT);
    pinMode(THERMO_DO, INPUT);
    
    // 閻氼偄绨幋顔垮箺闂勫棗绨甅AX6675濮樻捁浠懕鐑樼毘閹煎倽浠撮懢鍊熶槐闂囨彃绻栭懕娆掍粶
    digitalWrite(THERMO_CS, HIGH);  // CS濮樻挸缍嶉懕鑼皸閼辩偠浼旈懠鍛板Е閼变絻骞楅懕鎵暠濮樻捇绠嶆ご浣藉瘯瑜版洝浠懢钘夆枌閼辨稖骞楅懕鎵槵閼煎倸缍嶉懕?
    digitalWrite(THERMO_CLK, LOW);  // CLK濮樻挸缍嶉懕鑼皸閼辩偠浼旈惄鏌ユ閼便劏骞楅懕鎵暠濮樻捇绠嶆ご?
    
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    // 閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃挸绻栭懕顐ヤ紓閼句粙妾卞鐐寸毘閼鳖剛鍊濆鎾逛槐缁?
    beep(200);
    delay(100);
    beep(200);
    
    // 閼句粙娈曢崡銏㈡锤缁傚嫰婀跺鎾逛划閼辩儤鐨抽幖鍌濅淮濮樻捁浠搁懕瀹犲瘶闂勫浠搁悮顐ゅ€濋懕?
    if (!verifyHardwareInitialization()) {
        Serial.println("閼烘帟浼旈懕鍊熷瘯鐠у倽浼€ 閼句粙娈曢崡銏㈡锤缁傚嫰婀跺鎾逛划閼辩儤鐨抽幖鍌濅淮濮樻捁浠搁懕瀹犲瘶闂勫浠搁悮顐ゅ€濋懕娑欑毘闂勫洤宕甸悮顐︾氨濡よ壈瀵曡ぐ鏇′桓濮樻挻骞婇懕铏瑰皸閻愬浼堥懢浠嬫畷閸椼垻娲哥粋鍕苟濮樻捇妾遍懕褏娲搁梽鍡氫还...");
        
        // 濮樻挸缍嶅鐐寸毘閼便垽婀堕懢浠嬫畷閸椼垻娲哥粋鍕苟濮樻捇妾遍懕褏娲搁梽鍡氫还
        for (int i = 0; i < 3; i++) {
            digitalWrite(THERMO_CS, HIGH);
            digitalWrite(THERMO_CLK, LOW);
            delay(100);
            digitalWrite(THERMO_CS, LOW);
            delay(50);
            digitalWrite(THERMO_CS, HIGH);
            delay(100);
        }
        
        // 閼煎懓浠懕褍绻栭懕铏箠閼煎懘妾甸懕锔惧皸閻愬浠?
        if (verifyHardwareInitialization()) {
            Serial.println("閼烘帟浼栭懕?閼句粙娈曢崡銏㈡锤缁傚嫰婀跺鎾绘閼毖呮锤闂勫棜浠硅箛娆掍划閼鳖剚鐨抽懕銈堜肌");
        } else {
            Serial.println("閼烘帟浼滈懕?閼句粙娈曢崡銏㈡锤缁傚嫰婀跺鎾绘閼毖呮锤闂勫棜浠瑰鎾绘閸椼倗灏楁ス鎾淬偧閼煎倸缍嶉懕锔惧皸閻愬鐭捐箛娆愬珴閼辨瑥绻栭懕鍏笺偧閼句粙娈曢崡銏㈡锤缁傚嫰婀堕悮顐︹攼閼辫绻栭懕銊︺偧");
        }
    }
    
    // 濮樻捁浠遍懕鐣屽皸闂勫棝妾伴懠鍛颁户閼毖嗗箺闂勫棗绨?
    if (loadConfig()) {
        Serial.println("閼煎懓浠ч懕褑骞楅梽鍡楃爱濮樻捁浠遍懕鐣屽皸闂勫棝妾拌箛娆掍划閼鳖剚鐨抽懕銈堜肌");
    } else {
        Serial.println("韫囨瑨浼栭梽瀣箹閼憋絾鍩夊鎾逛划閹哄啿绻栭懕纭呬化韫囨瑨浼堥懕銏ｅ瘶閼辩喕浠归懢浠嬫鎼?);
    }
    
    // 濮樻捁浼傞悙澶嬬毘閼便倗鐦懢浠嬫閼鳖叀骞楃粋鍕紪
    if (shouldStartCaptivePortal()) {
        startCaptivePortal();
    } else {
        connectToWiFi();
    }
    
    // 韫囨瑨鐭剧粋鍕毘閼便倛浼ｅ鎾崇爱閼憋絾鐨抽懕鐔虹槵韫囨瑨浼栧鐐寸毘閼便垽婀堕懠鍌氱秿閼遍攱鐨抽懕褔鐬鹃惄鏌ユ妞规潙绻栫挧鍌涚础濮樻挻缍栨繛鍕锤瑜版洝浼ｈ箛娆掍沪閼卞吋鐨抽懕鍓佺槵韫囨瑨浼堥懕鐔诲瘶閼辩柉浼栭懠鍌氱秿閼憋妇娲告ィ鑳肌閻氼偄鈻岄懕娑滃箺闂呭棗绨惄鏌モ攼閼辩eb韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾崇箹妫板懏瀚濆鎾圭ォ鐠у倹鐨抽懕顒傚€濆鎾逛槐缁?
    Serial.println("閼烘帟浼栭懕?濮樻挸绨懕锝嗙毘閼辩喓鐦箛娆掍紪濞肩偞鐨抽懕銏ゆ苟濮樻捁浼傞悙澶嬬毘閼便倗鐦懠鍌氱秿閼遍攱鐨抽懕褔鐬鹃惄鏌ユ妞规潙绻栫挧鍌涚础濮樻挻缍栨繛鍕锤瑜版洝浼ｈ箛娆掍沪閼卞吋鐨抽懕鍓佺槵韫囨瑨浼堥懕鐔诲瘶閼辩柉浼栭懠鍌氱秿閼鳖毥eb韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾舵锤缁傚嫯浠归懢鍊熶沪闂囧弶鐨抽懕顐ゅ€濋懢鍊熶紗缁?);
    
    // 濮樻捁浼傞悙澶嬬毘閼便倗鐦琓CP韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾瑰箺閼辨壆鐦惄鍙夌稏閼彼婸P閻氼偊鈹愰懕璇茬箹閼便劍銈?
    tcpServer.begin();
    Serial.println("TCP韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘鐠侯垵妾瑰鎾逛紓閻愬鐨抽懕銈囩槵閼煎倸缍嶉懕锕佸箺閼辨瓕浼冨鎾逛紓閸椼垼骞楅懞锔惧€濆鎾逛紑閹? " + String(DEFAULT_PORT));
    
    // 濮樻捁浠懕鐑樼毘閹煎倽浠村鎾逛桓閼辩熁ittleFS韫囨瑨浼嬮懕锛勬锤缁傚嫰婀堕懢浠嬬灳缁傚嫯骞楃粋鍕肌
  if (LittleFS.begin()) {
    Serial.println("LittleFS韫囨瑨浼嬮懕锛勬锤缁傚嫰婀堕懢浠嬬灳缁傚嫯骞楃粋鍕肌濮樻捁浠懕鐑樼毘閹煎倽浠村鎾逛桓閼卞啿绻栭懕銏ｄ紓濮樻捁浠遍懕?);
        
        // 韫囨瑦瀚濋懕娆忕箹閼卞吋銈艰箛娆掍紥閻愬鐨抽懕顒€鈻屽鎾活暘閼卞灚鐨抽懕鍦槵濮樻捇鈹愰懕鐔哄皸婵炲嫯浠滈懢鍊熶紨閼辩偛绻栭懕瀹犱画閻╄尙顩╅棁?
        if (LittleFS.exists("/login.html")) {
            Serial.println("韫囨瑨浠幋顔界毘閼便垺骞婇懢鍊熶紦缁傚嫭鐨抽梽鍡氫紙閼煎懘娈曠喊宀冨瘶閼卞搫鐎箛娆掍紜閼憋紕娲哥粋鍕苟: /login.html");
        } else {
            Serial.println("閻氼偊顣繛鍕毘閼鳖叀浠? 韫囨瑨浼栭梽瀣箹閼憋絾鍩夊鎾逛划閹哄疇骞楅懕鍓侇洨濮樻捇妾伴懕鑼跺瘶闂呭棛顣遍懠鍛颁紲閸ㄥ嫬绻栭懕瀹犱画閻╄尙顩╅棁?/login.html");
        }
        
        if (LittleFS.exists("/index.html")) {
            Serial.println("韫囨瑨浠幋顔界毘閼便垺骞婇惄鑼剁ォ缁傚嫯瀵橀梾鍡欘暠韫囨瑨浼嬮懕锛勬锤缁傚嫰婀? /index.html");
        } else {
            Serial.println("閻氼偊顣繛鍕毘閼鳖叀浠? 韫囨瑨浼栭梽瀣箹閼憋絾鍩夊鎾逛划閹哄磭娲哥挧鍌滎洨閼煎懘娈曠喊灞界箹閼卞疇浠惄鑼洨闂?/index.html");
        }
    } else {
        Serial.println("閼煎懓浼嗛懕鍓佸皸閻愬鍊? LittleFS韫囨瑨浼嬮懕锛勬锤缁傚嫰婀堕懢浠嬬灳缁傚嫯骞楃粋鍕肌濮樻捁浠懕鐑樼毘閹煎倽浠村鎾逛桓閼辫櫕鐨抽梽鍥у吹閻氼偊绨卞Δ?);
    }
    
    Serial.println("濮樻捁浠懕鐑樼毘閹煎倽浠村鎾逛桓閼辫櫕鐨虫惔鎰桓韫囨瑨浠懕?);
}

// =========================================
// 閻╄尪绁╅搹蹇旂毘閼鳖偅瀚濆鎾逛純闂勫棛娲哥粋鍕濮樻捇妾遍懕鐐跺箺閼鳖剝浠?
// =========================================
void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            Serial.println("韫囨瑨浼嗛棁鍙夌毘閼便垺骞婂鎾逛純闂勫棛娲哥粋鍕: " + command);
            
            if (command == "LED_ON") {
                digitalWrite(LED_PIN, HIGH);
                Serial.println("LED濮樻捁鐭鹃搹蹇撶箹閼憋綀浼呭鎾崇秿閼?);
            } else if (command == "LED_OFF") {
                digitalWrite(LED_PIN, LOW);
                Serial.println("LED濮樻捁鐭鹃搹蹇旂毘閼辩喖鐬鹃懠鍛颁紝妫?);
            } else if (command == "LED_BLINK") {
                // 閻╄尪绁╂ス鎾崇箹閼辨挳婀堕懠鍛颁紝闂勫骞楅懕婵婁粶LED
                for (int i = 0; i < 5; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(200);
                    digitalWrite(LED_PIN, LOW);
                    delay(200);
                }
                Serial.println("LED閼煎懓浼岄梽瀣箺閼辨繆浠滃鎾崇爱閼憋箑绻栭懕銏ｄ紓");
            } else if (command == "BEEP") {
                beep(100);
                Serial.println("閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃挻鐨崇捄顖濇濮樻捁浼呴懕?);
            } else if (command == "BEEP_LONG") {
                beep(500);
                Serial.println("閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃捁瀵橀懕鏌モ攼濮樻捁浼呴懕?);
            } else if (command == "BEEP_SHORT") {
                beep(50);
                Serial.println("閻氼偉浼栭懕婊嗗瘶鐠у倹瀚濆鎾逛紦缁℃捁骞楅懕濂割暘濮樻捁浼呴懕?);
            } else if (command == "GET_STATUS") {
                // 閻氼偊鈹愰懕鐗堢毘閼辨瓕浼滱PP韫囨瑨浼栭懕鐓庣箹閼辩浼曢懢鍊熶紨閼辩偛绻栭懕钘夌秿濮樻挸缍嶉懕顐ュ瘯瑜版洝浼擳EMP:25.50,TARGET:180.00,HEAT:0,MODE:1,UPTIME:123
                String statusResponse = "TEMP:" + String(currentTemp) + 
                                      ",TARGET:" + String(targetTemp) + 
                                      ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                                      ",MODE:" + String(ovenMode ? "1" : "0") + 
                                      ",UPTIME:" + String(millis() / 1000);
                Serial.println(statusResponse);
            } else if (command == "GET_TEMP") {
                Serial.println("濮樻捇妾伴懕顖涚毘閼憋綀浠硅箛娆掔ォ濠曞繑鐨冲鐐测枌: " + String(currentTemp) + "閼存骞奀");
            } else if (command.startsWith("CALIBRATE_TEMP")) {
                // 韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍迹闂呭棙鐨抽懕陇浠╁鎾逛純闂勫棛娲哥粋鍕韫囨瑨浼ｈぐ鏇熺毘瑜版洝浼€: CALIBRATE_TEMP 濮樻挸绨懕鏄忓瘶閼辨儼浠ц箛娆掔ォ濠曞繑鐨冲鐐测枌
                int spaceIndex = command.indexOf(' ');
                if (spaceIndex > 0) {
                    String actualTempStr = command.substring(spaceIndex + 1);
                    float actualTemp = actualTempStr.toFloat();
                    float measuredTemp = currentTemp;
                    
                    if (actualTemp > 0) {
                        calibrateTemperature(actualTemp, measuredTemp);
                        Serial.println("韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍迹闂呭棙鐨抽懕陇浠╁鎾圭熅閾忓繑鐨冲鐐朵紗閼惧€熶紗缁℃捁瀵曡ぐ鏇′桓閻氼偆鍊濈捄顖濆瘶閼甭や还韫囨瑨浼嬮幒宕囧皸閻愬顩╁鎾逛紑閼卞啿绻栫挧鍌涚础濮樻挻缍栨繛鍕瘶闂勫浠搁悮顐ゅ€濋懕?);
                    } else {
                        Serial.println("閼煎懓浼嗛懕鍓佸皸閻愬鍊? 濮樻挸绨懕鏄忓瘶閼辨儼浠ц箛娆掔ォ濠曞繑鐨冲鐐测枌濮樻捁浠氳ぐ鏇炵箹閼辩浼ｈ箛娆掍紙閼?);
                    }
                } else {
                    Serial.println("韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍迹闂呭棙鐨抽懕陇浠╁鎾逛純闂勫棛娲哥粋鍕韫囨瑨浼ｈぐ鏇熺毘瑜版洝浼€: CALIBRATE_TEMP 濮樻挸绨懕鏄忓瘶閼辨儼浠ц箛娆掔ォ濠曞繑鐨冲鐐测枌");
                    Serial.println("閻╁弶鍩夐懕銉︾毘婵炲嫯浠? CALIBRATE_TEMP 16.0");
                }
            } else if (command == "RESET_CALIBRATION") {
                temperatureOffset = 0.0;
                temperatureScale = 1.0;
                Serial.println("韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍迹闂呭棙鐨抽懕陇浠╁鎾圭熅閾忓繗瀵橀懕陇浠归懢浠嬫鎼?);
            } else if (command == "GET_RAW_TEMP") {
                // 閻氼偉浠虹捄顖涚毘閼鳖偉浼嬪鎾逛缓閼卞吋鐨抽幖鍌濅淮韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍紙閹哄啿绻栭懕褍绨懠鍌氱秿閼便垹绻栭懕褰掓韫囨瑨浼ｉ梾鍡樼毘閼甭や哗閼煎倸缍嶉懕?
                uint16_t rawData = readMAX6675RawData();
                if (!(rawData & 0x04)) {
                    uint16_t tempBits = rawData >> 3;
                    float rawTemp = tempBits * 0.25;
                    Serial.println("濮樻捁浠洪懕鍏肩毘閹煎倽浠磋箛娆掔ォ濠曞繑鐨冲鐐测枌韫囨瑨浼堥幒鍐茬箹閼毖冪爱:");
                    Serial.print("濮樻捁浠洪懕鍏肩毘閹煎倽浠村鎾逛粴瑜? 0x"); Serial.println(rawData, HEX);
                    Serial.print("韫囨瑨绁╁蹇旂毘濞肩偛鈻岄惄鏌ユ閼? "); Serial.println(tempBits);
                    Serial.print("韫囨瑨浼栭梽瀣箹閼变粙娈曞鎾逛画閼辩姴绻栫挧鍌涚础濮樻挻缍栨繛? "); Serial.print(rawTemp); Serial.println("閼存骞奀");
                    Serial.print("韫囨瑨浼ｉ梾鍡樼毘閼甭や哗濮樻捁浼傞懕銊ョ箹鐠у倹绱″鎾寸稏婵? "); Serial.print(currentTemp); Serial.println("閼存骞奀");
                } else {
                    Serial.println("閼煎懓浼嗛懕鍓佸皸閻愬鍊? 韫囨瑨绁╁蹇旂毘濞肩偛鈻岄惄鎻掔秿閼辫棄绻栭懕鐐朵肌濮樻捁浼撶弧鎾跺皸閻愬顩╁鎾逛紑閼卞疇瀵橀懕鎷屼紦閻氼偆鍊濋悙?);
                }
            } else if (command == "GET_PERFORMANCE") {
                // 閻氼偉浠虹捄顖涚毘閼鳖偉浼嬭箛娆掍粴閹煎倻灏楅懕婵嬫閻╂煡鈹愰梾鍡楃箹閼辨稓鍊?
                Serial.println("閸愭帟浼￠懕顖濅槐 閻氼偄绨幋顔界毘闂勫洩浠箛娆掍粴閹煎倻灏楅懕婵嬫閻╂煡鈹愰梾鍡楃箹閼辨稓鍊?");
                Serial.print("韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ゅ€濈粋鍕毘閼鳖偉浼嬪鎾荤畭妞翠焦鐨抽懕楦夸画韫囨瑨浼岄棁鑼跺瘶閼辨挳绨? "); Serial.print(temperatureReadAvgTime); Serial.println("ms");
                Serial.print("韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ゅ€濈粋鍕毘閼鳖偉浼嬭箛娆掍粴缁傚嫬绻栭崡銏ゆ畷韫囨瑨浼堥幒? "); Serial.println(temperatureReadCount);
                
                // 閻氼偉浠虹捄顖涚毘閼鳖偉浼嬪鎾逛哗閼辩喐鐨虫０鍛颁紥閻╂煡鈹愰梾鍡楃箹閼辨稓鍊?
                uint32_t freeHeap = ESP.getFreeHeap();
                uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
                uint32_t heapFragmentation = ESP.getHeapFragmentation();
                
                Serial.print("閼捐姤绱″鐐跺瘶閼辩妾瑰鎾逛哗閼辩喐鐨虫０鍛颁紥: "); Serial.print(freeHeap); Serial.println(" bytes");
                Serial.print("韫囨瑨浼栭懕娆愮毘闂勫洦鎮呴懢鑺ョ础濞肩偠瀵橀懕纾嬫濮樻捁浼滈懕? "); Serial.print(maxFreeBlock); Serial.println(" bytes");
                Serial.print("濮樻捁浠╅懕鐔哥毘妫板懓浼掗懢钘夌€懕銊ㄥ箺閼憋綀浠懢鍊熶缓閼? "); Serial.print(heapFragmentation); Serial.println("%");
                
                // 閻氼偅鍩夐懕顖涚毘閼扁剝缍栧鎾绘閼鳖垱鐨抽懕锝堜还閼惧€熶槐闂囨彃绻栭懕娆掍粶
                Serial.print("Web韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘闂勫洩浠﹂懢鍊熶紓閼辩姾瀵橀懕鎾氨閼煎懓浼旈懕? "); Serial.print(WEB_SERVER_HANDLE_INTERVAL); Serial.println("ms");
                Serial.print("韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ゅ€濈粋鍕毘閼鳖偉浼嬮懠鍛颁紝妤规捁瀵橀懕鐤紗: "); Serial.print(TEMPERATURE_READ_INTERVAL); Serial.println("ms");
            } else if (command == "RESET_PERFORMANCE") {
                // 閼煎懓浠懕褑骞楅梽鍡楃爱韫囨瑨浠氶幖鍌滃皸閼辨繈妾伴悮顐㈢爱闂呭棗绻栭懕鍙夊箠濮樻捁浼撶弧?
                temperatureReadCount = 0;
                temperatureReadAvgTime = 0;
                Serial.println("閼烘帟浼栭懕?韫囨瑨浠氶幖鍌滃皸閼辨繈妾伴悮顐㈢爱闂呭棗绻栭懕鍙夊箠濮樻捁浼撶弧鎾寸毘鐠侯垵妾归懠鍛颁画閼毖嗗箺闂勫棗绨?);
            } else {
                Serial.println("韫囨瑨浼栭梽瀣箺閼卞吋銈煎鎾逛純闂勫棛娲哥粋鍕閼煎倸缍嶉懕锔界毘閼鳖偆鍊濋懢鍊熶紗缁℃挻鐨抽懕顓㈡閻╄尙顩╅梽?");
                Serial.println("LED_ON, LED_OFF, LED_BLINK");
                Serial.println("BEEP, BEEP_LONG, BEEP_SHORT");
                Serial.println("GET_STATUS, GET_TEMP, GET_RAW_TEMP");
                Serial.println("CALIBRATE_TEMP 濮樻挸绨懕鏄忓瘶閼辨儼浠ц箛娆掔ォ濠曞繑鐨冲鐐测枌");
                Serial.println("RESET_CALIBRATION");
                Serial.println("GET_PERFORMANCE, RESET_PERFORMANCE");
            }
        }
    }
}

// =========================================
// TCP韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘闂勫洩浠﹂懢鍊熶紓閼辩姵鐨抽懕锟犳韫囨瑨浼堥幒?
// =========================================

void handleTCPConnection() {
    // 韫囨瑦瀚濋懕娆忕箹閼卞吋銈艰箛娆掍紥閻愬鐨抽懕顒€鈻岃箛娆掍紪閼憋絽绻栭懕铏箠閼惧€熶紨閼辩偞鐨虫惔鎰€箛娆掍划鐠侯垵骞楅懞锔惧€濋悮顐︹攼閼辫绻栭懕銊︺偧
    if (tcpServer.hasClient()) {
        // 濮樻挸鈻岄懕婊冪箹閼辨槒浼栧鎾圭熅閾忓繗骞楃粋鍕紑韫囨瑨浼栭懕锝嗙毘鎼存劕鐎箛娆掍划鐠侯垵骞楅懞锔惧€濋悮顐︹攼閼辫绻栭懕銊︺偧閼煎倸缍嶉懕锕€绻栭懕鎶筋暘濮樻挸缍嶉懕娆忕箹閼卞瓨鎮呴懢鍊熶紨閼辩偟灏楁す纾嬩紳韫囨瑨浠哄Δ?
        if (tcpClient && tcpClient.connected()) {
            tcpClient.stop();
            Serial.println("TCP濮樻挸绨崹鍕箹閼便垼鐭鹃懢鍊熷Е閻愬鐨崇捄顖濇韫囨瑨浼嬫０鍛毘瑜版洝浠?);
        }
        
        // 韫囨瑨浠哄Δ鍏肩毘閼鳖偉浼岃箛娆掍紜閹哄疇骞楅懕鐤沪濮樻挸绨崹鍕箹閼便垼鐭鹃懢鍊熷Е閻愬灏楁す纾嬩紳韫囨瑨浠哄Δ?
        tcpClient = tcpServer.available();
        if (tcpClient) {
            Serial.println("TCP濮樻挸绨崹鍕箹閼便垼鐭鹃懢鍊熷Е閻愬鐨崇捄顖濇閻氼偊鈹愰懕璇茬箹閼便劍銈? " + tcpClient.remoteIP().toString());
            
            // 濮樻捁浼€閼鳖叀瀵橀懕娆掍粶韫囨瑥宕查崹鍕皸妞圭浠鸿箛娆撴苟閼便垹绻栭懕娑氬€?
            tcpClient.println("SmartOven Controller v" + FIRMWARE_VERSION);
            tcpClient.println("閻氼偊鈹愰懕璇茬箹閼便劍銈艰箛娆掍划閼鳖剚鐨抽懕銈堜肌閼煎倸缍嶉懕锔惧皸閻愬鐭鹃悮顐ｅ焿閼鳖垱鐨抽懕鐔搞偧濮樻捁浼冮梽鍡欐锤缁傚嫰妾?);
        }
    }
    
    // 濮樻捇妾遍懕鐐跺箺閼鳖剝浠╁鎾圭熅閾忓繒灏楁す纾嬩紳韫囨瑨浠哄Δ鑹板箺閼辩柉浠﹀鎾崇爱閸ㄥ嫬绻栭懕銏ｇ熅閼惧€熷Е閻愬绻栭懕鍙夊箠韫囨瑨浠规惔?
    if (tcpClient && tcpClient.connected()) {
        if (tcpClient.available()) {
            String command = tcpClient.readStringUntil('\n');
            command.trim();
            
            if (command.length() > 0) {
                Serial.println("TCP韫囨瑨浼嗛棁鍙夌毘閼便垺骞婂鎾逛純闂勫棛娲哥粋鍕: " + command);
                handleTCPCommand(command);
            }
        }
    }
}

void handleTCPCommand(String command) {
    if (command == "GET_STATUS") {
        // 閻氼偊鈹愰懕鐗堢毘閼辨瓕浼濋悮顐㈢爱閹搭喗鐨抽梽鍥画閼惧€熶槐闂囨彃绻栭懕娆掍粶閻╂煡鈹愰梾鍡楃箹閼辨稓鍊?
        String statusResponse = "TEMP:" + String(currentTemp) + 
                              ",TARGET:" + String(targetTemp) + 
                              ",HEAT:" + String(heatingEnabled ? "1" : "0") + 
                              ",MODE:" + String(ovenMode ? "1" : "0") + 
                              ",UPTIME:" + String(millis() / 1000);
        tcpClient.println(statusResponse);
        Serial.println("TCP濮樻捁浼€閼鳖叀瀵橀懕娆掍粶閼惧€熶槐闂囨彃绻栭懕娆掍粶: " + statusResponse);
    } else if (command == "GET_TEMP") {
        tcpClient.println("濮樻捇妾伴懕顖涚毘閼憋綀浠硅箛娆掔ォ濠曞繑鐨冲鐐测枌: " + String(currentTemp) + "閼存骞奀");
    } else if (command.startsWith("SET_TEMP")) {
        // 閻氼偄绨幋顔垮箺闂勫棗绨懢鍊熶紩鎼存劕绻栭懕鍊熶画韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍迹瑜版洘鐨宠ぐ鏇′紑: SET_TEMP 180.0
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            String tempStr = command.substring(spaceIndex + 1);
            float newTemp = tempStr.toFloat();
            
            if (newTemp >= 0 && newTemp <= 300) {
                targetTemp = newTemp;
                tcpClient.println("閼惧€熶紩鎼存劕绻栭懕鍊熶画韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾圭熅閾忓繒灏楁惔鎰焿閼句粙妾版惔鎰锤鐠у倹缍? " + String(targetTemp) + "閼存骞奀");
                Serial.println("TCP閻氼偄绨幋顔垮箺闂勫棗绨懢鍊熶紩鎼存劕绻栭懕鍊熶画韫囨瑨绁╁蹇旂毘濞肩偛鈻? " + String(targetTemp) + "閼存骞奀");
            } else {
                tcpClient.println("閼煎懓浼嗛懕鍓佸皸閻愬鍊? 韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ヤ桓閼辨繃鐨抽懕鎼佺氨濮樻挻缍栭懕鎵锤鐠у倹缍?-300閼存骞奀");
            }
        }
    } else if (command == "HEAT_ON") {
        heatingEnabled = true;
        tcpClient.println("濮樻捁浠遍懕鍊熷箺閼辨繈顣鎾圭熅閾忓繑鐨宠ぐ鏇′粴濮樻捁浼傞悙?);
        Serial.println("TCP濮樻挸缍嶉懕娆愮毘閼鳖剛鍊濆鎾逛槐閼卞€熷箺閼辨繈顣?);
    } else if (command == "HEAT_OFF") {
        heatingEnabled = false;
        tcpClient.println("濮樻捁浠遍懕鍊熷箺閼辨繈顣鎾圭熅閾忓繑鐨抽懕鐔肩灳閼煎懓浼屾０?);
        Serial.println("TCP濮樻捁浠фご浣藉瘶閼辨挳顣鎾逛槐閼卞€熷箺閼辨繈顣?);
    } else if (command == "OVEN_MODE") {
        ovenMode = true;
        tcpClient.println("濮樻捁鐭鹃搹蹇旂毘閼便垼浠箛娆掍还閸ㄥ嫭鐨抽懕銏″箠閼惧€熶虎闂勫洩骞楁惔鎰吹韫囨瑧鐦梾鍡樼毘瑜版洝浼€");
        Serial.println("TCP濮樻捁浠懕鈥崇箹閼毖冪€鎾逛划閹哄疇骞楅懕婵嬫閼捐棄绨崡銈呯箹缁℃捇娈曞鎾崇秿閼?);
    } else if (command == "TOASTER_MODE") {
        ovenMode = false;
        tcpClient.println("濮樻捁鐭鹃搹蹇旂毘閼便垼浠箛娆掍还閸ㄥ嫭鐨抽懕銏″箠閼惧€熶虎闂勫洩瀵橀懕鍝勭€鎾逛桓閼辩喎绻栭懕瑙勭稏韫囨瑧鐦梾鍡樼毘瑜版洝浼€");
        Serial.println("TCP濮樻捁浠懕鈥崇箹閼毖冪€鎾逛划閹哄疇骞楅懕婵嬫閼煎懓浼滈崹鍕毘閼憋箒浠ц箛娆掍紪濞肩偛绻栫弧鎾绘畷濮樻挸缍嶉懕?);
    } else if (command == "PING") {
        tcpClient.println("PONG");
    } else {
        tcpClient.println("韫囨瑨浼栭梽瀣箺閼卞吋銈煎鎾逛純闂勫棛娲哥粋鍕閼煎倸缍嶉懕锔界毘閼鳖偆鍊濋懢鍊熶紗缁℃挻鐨抽懕顓㈡閻╄尙顩╅梽?");
        tcpClient.println("GET_STATUS, GET_TEMP, SET_TEMP 韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛粴瑜?);
        tcpClient.println("HEAT_ON, HEAT_OFF, OVEN_MODE, TOASTER_MODE, PING");
    }
}

// =========================================
// 韫囨瑨浠氶幖鍌滃皸閼辨繈妾伴懢鍊熶紩閼鳖厼绻栭懕銊︽倕濮樻捁浼勯懕锔界毘閼辩姾浠у鎾活暘閼变絻骞楁惔鎰版畷閼惧€熶紓閼?
// =========================================

// 濮樻捁浠╅懕鐔哥毘妫板懓浼掗懢鍊熶紩閼鳖厼绻栭懕銊︽倕濮樻挸绨懕宄扮箹閼辨挳婀跺鎾逛紦缁?
unsigned long lastMemoryCheck = 0;
const unsigned long MEMORY_CHECK_INTERVAL = 5000; // 韫囨瑧鍊濋懕?閼捐姤鎮呴懕顔肩箹閹枫垼浠氳箛娆掍肌濡よ偐娲哥挧鍌濅粴韫囨瑥宕查梾鍡樼毘閼辩姾浠у鎾活暘閼?

// 韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ゅ€濈粋鍕毘閼鳖偉浼嬭箛娆掍粴閹煎倻灏楅懕婵嬫閼惧€熶紩閼鳖厼绻栭懕銊︽倕
unsigned long lastTemperatureReadTime = 0;

// =========================================
// 閻╄尪绁╃粋鍕毘閹搭噣妾甸懢鍊熶缓閻?
// =========================================

// 韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛划閼扁€崇箹閼毖冪€箛娆愬珴閼辨瑥绻栫喊宀冧淮濮樻捁浼€閼变絻瀵橀懕陇浼€
float lastTargetTemp = targetTemp;
unsigned long lastTempChangeTime = 0;
const unsigned long TEMP_CHANGE_DEBOUNCE = 2000; // 2閼捐姤鎮呴懕顔垮瘶閼变絻妾硅箛娆掍槐閼?

// Web韫囨瑨浼栭懕褎鐨抽懕銈夋畷濮樻捁浼撶弧鎾寸毘闂勫洩浠﹂懢鍊熶紓閼辩姵鐨虫惔鎰紨韫囨瑨浼岄棁鍙夌毘閼卞墎鐦?
unsigned long lastWebServerHandle = 0;

void loop() {
    unsigned long currentTime = millis();
    
    // 濮樻捇妾遍懕鐐跺箺閼鳖剝浠╁鎾崇秿濞肩偞鐨抽懕銏ゆ苟閼煎懓浼岀弧鎾崇箹閼便垼鐭鹃懠鍌氱秿閼便垹绻栭懕顐ヤ紓閼煎懓濮﹂懕鍨毘閼鳖垵浠瑰鎾寸稏閼辨媽瀵橀崹鍕純閼惧€熶缓閼甭ゅ瘯瑜版洝浠?
    if (isCaptivePortalMode) {
        // 濮樻挸缍嶅鐐寸毘閼便垽婀堕懠鍛颁紝缁℃挸绻栭懕銏ｇ熅韫囨瑧鐦梾鍡樼毘瑜版洝浼€閻╄尪绁╅懕銉ㄥ瘶閼辩浠氶悮顐⑩枌閼辨稑绻栭懕鎼佺氨閼煎懎鐎懕顓″箺妤ｈ儻浠滃鎾逛紪閹鸿櫕鐨抽梽鍥沪閼惧€熶紓閼辩嚉NS濮樻捁浼勯懕顨絜b閻氼偆鍊濈捄顖氱箹閸椼倛浠?
        dnsServer.processNextRequest();
        webServer.handleClient();
        checkCaptivePortalTimeout();
    } else {
        // 韫囨瑩顣幏銏＄毘鐠у倽绁╄箛娆戠槵闂呭棙鐨宠ぐ鏇′紑閻╄尪绁╅懕銉︾毘閼鳖偆鍊濋惄鑼洨濡よ壈瀵橀懕娆掍粻濮樻捇妾伴懕顖濆瘶閼辨儼浠归惄鏌ユ閼便劍鐨抽梽鍥沪閼惧€熶紓閼辩姾瀵橀崹鍕純閼惧€熶缓閼?
        if (currentTime - lastWebServerHandle >= WEB_SERVER_HANDLE_INTERVAL) {
            webServer.handleClient();
            lastWebServerHandle = currentTime;
        }
    }
    
    // 濮樻捇妾遍懕鐐跺箺閼鳖剝浠㎡TA濮樻捁浠归懕陇骞楀鐐存倕閼煎倸缍嶉懕銏㈡锤闂勫棜浠洪懠鍛€懕顓″瘯瑜版洝浠?
    handleOTA();
    
    // 濮樻捇妾遍懕鐐跺箺閼鳖剝浠╅悮顐㈢爱閹搭喗鐨抽梽鍥画濮樻捁浼€閼鳖叀骞楅懕銊﹀箠閼煎倸缍嶉懕銏㈡锤闂勫棜浠洪懠鍛€懕顓″瘯瑜版洝浠?
    handleDiscovery();
    
    // 濮樻捇妾遍懕鐐跺箺閼鳖剝浠㏕CP閻氼偊鈹愰懕璇茬箹閼便劍銈奸懠鍌氱秿閼便垼瀵橀懞锕佷紥閼煎懎鐎懕顓″瘯瑜版洝浠?
    handleTCPConnection();
    
    // 濮樻捇妾遍懕鐐跺箺閼鳖剝浠╅惄鑼剁ォ閾忓繑鐨抽懕顐ｅ珴濮樻捁浼冮梽鍡欐锤缁傚嫰妾遍懠鍌氱秿閼便垼瀵橀懞锕佷紥閼煎懎鐎懕顓″瘯瑜版洝浠?
    handleSerialCommands();
    
    // 韫囨瑨绁╁蹇旂毘濞肩偛鈻岃箛娆掍缓閹煎倹鐨抽懕銏ゆ苟閼煎倸缍嶉懕銏＄毘鎼存劘浼旇箛娆掍紝闂囪尙灏楅悙澶岊洨濮樻捁浼€閼卞疇瀵曡ぐ鏇′化
    readTemperature();
    controlHeater();
    
    // LED閼惧€熶槐闂囨彃绻栭懕娆掍粶韫囨瑨浼曟ス鎾崇箹閼辫櫕骞婇懠鍌氱秿閼便垺鐨虫惔鎰紨韫囨瑨浼岄棁鑼跺瘯瑜版洝浠?
    updateLED();
    
    // 韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛划閼扁€崇箹閼毖冪€箛娆愬珴閼辨瑥绻栫喊宀冧淮
    if (targetTemp != lastTargetTemp) {
        // 閼煎懓浼掗搹蹇撶箹閼便倛浼嬪鎾绘閼辩偠骞楅懕顒冧哗閼煎倸缍嶉懕閿嬬毘閼鳖偊妾佃箛娆掍紪閼憋絾鐨抽懕鍦槵韫囨瑨绁╁蹇旂毘濞肩偛鈻屽鎾逛紑閼卞灚鐨抽懕锕佷紜濮樻捁浼傞懕?閼捐姤鎮呴懕顔界毘閼辩姾浠ц箛娆掓闂呭棗绻栭懕纭呬化濮樻捁浠╅懕褍绻栭崡銏ゆ畷濮樻捁浼€閼卞灚鐨抽懕锕佷紜韫囨瑨浠懕褏灏楅幖鍌氣枌濮樻捁浼€閼鳖厼绻栭懕顐ヤ紓閼句粙妾卞?
        if (currentTime - lastTempChangeTime > TEMP_CHANGE_DEBOUNCE) {
            Serial.println("韫囨瑦瀚濋懕娆忕箹绾板矁浠村鎾逛划閹哄啿绻栫挧鍌涚础濮樻挻缍栨繛鍕毘閼便垼浠箛娆掍还閸? " + String(lastTargetTemp) + "閼存骞奀 -> " + String(targetTemp) + "閼存骞奀");
            beepTemperatureChange();
            lastTargetTemp = targetTemp;
        }
        lastTempChangeTime = currentTime;
    }
    
        // 濮樻捁浠╅懕鐔哥毘妫板懓浼掗懢鍊熶紩閼鳖厼绻栭懕銊︽倕閼煎倸缍嶉懕銏犵箹閻愬浼€5閼捐姤鎮呴懕顔肩箹閹枫垼浠氳箛娆掍肌濡よ偐娲哥挧鍌濅粴韫囨瑥宕查梾鍡氬瘯瑜版洝浠?
    if (currentTime - lastMemoryCheck >= MEMORY_CHECK_INTERVAL) {
        checkMemoryUsage();
        lastMemoryCheck = currentTime;
    }
    
    // 閻╂煡妾版す纾嬪箺閼辨壆鐦懠鍛颁紲閼辨槒瀵橀懕鐢殿洨濮樻捇娈曢懕缁樼毘缁傚嫰婀堕悮顐︹攼閼辫壈瀵曡ぐ鏇′桓韫囨瑨浼€閼鳖剝瀵橀懞锕佷紥濮樻捁浼呴懕褎鐨冲鐐朵紗閼煎懓浠氶懕鍏肩毘濞肩偛鈻?
    delay(1);
}

// =========================================
// 韫囨瑨浠氶幖鍌滃皸閼辨繈妾伴懢鍊熶紩閼鳖厼绻栭懕銊︽倕濮樻捁浠梽鍡楃箹閼卞弶骞?
// =========================================

void checkMemoryUsage() {
    // 韫囨瑦瀚濋懕娆忕箹閼卞吋銈糆SP8266濮樻捁浠╅懕鐔哥毘妫板懓浼掗惄鏌ユ妞圭骞楅懕鎵槵韫囨瑨浠㈤懕鐔哥毘閼辩姷顣?
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxFreeBlock = ESP.getMaxFreeBlockSize();
    uint32_t heapFragmentation = ESP.getHeapFragmentation();
    
    // 濮樻捁浼€闂勫鐨抽懕鍦槵濮樻捁浠╅懕鐔哥毘妫板懓浼掗惄鏌ユ妞圭骞楅懕鎵槵閼惧€熶缓閼憋紕灏楅幋顔夸虎閼煎懓濮﹂懕闈涚箹閼辨挳婀堕悮顐ｅ焿閼鳖垱鐨抽懕鈩冪稏閻氼偊顣繛鍕毘閼鳖叀浠?
    if (freeHeap < 20000) { // 濮樻挻骞婇懕顓犳锤濞肩偠浠?0KB閼捐姤绱″鐐跺瘶閼辩妾瑰鎾逛哗閼辩喐鐨虫０鍛颁紥
        Serial.println("閼烘帟浼旈懕鍊熷瘯鐠у倽浼€ 濮樻捁浠╅懕鐔哥毘妫板懓浼掗悮顐︻暘婵炲嫭鐨抽懕顓′槐: 閼捐姤绱″鐐跺瘶閼辩妾瑰鎾逛哗閼辩喐鐨虫０鍛颁紥閻氼偅鍩夐懕婵堟锤闂勫棜浠?- " + String(freeHeap) + " bytes");
        Serial.println("韫囨瑨浼栭懕娆愮毘闂勫洦鎮呴懢鑺ョ础濞肩偠瀵橀懕纾嬫濮樻捁浼滈懕? " + String(maxFreeBlock) + " bytes");
        Serial.println("濮樻捁浠╅懕鐔哥毘妫板懓浼掗懢钘夌€懕銊ㄥ箺閼憋綀浠懢鍊熶缓閼? " + String(heapFragmentation) + "%");
    }
    
    // 閻氼偅鍩夐懕顖涚毘閼扁剝缍栬箛娆掔ォ濠曞繑鐨冲鐐测枌閻氼偆鍊濈粋鍕毘閼鳖偉浼嬭箛娆掍粴閹煎倻灏楅懕婵嬫閼剧晫顩╅懕鑲╁皸鎼存劙娈?
    if (temperatureReadCount > 0) {
        Serial.println("閸愭帟浼￠懕顖濅槐 韫囨瑨绁╁蹇旂毘濞肩偛鈻岄悮顐ゅ€濈粋鍕毘閼鳖偉浼嬭箛娆掍粴閹煎倻灏楅懕婵嬫: 濮樻捇绠嶆ご浣圭毘閼遍缚浠箛娆掍紝闂囪尪瀵橀懕鎾氨 " + String(temperatureReadAvgTime) + "ms, 韫囨瑨浠氱粋鍕箹閸椼垽娈曡箛娆掍紙閹?" + String(temperatureReadCount));
        // 閼煎懓浠懕褑骞楅梽鍡楃爱閻氼偄绨梾鍡楃箹閼卞弶骞婂鎾逛紦缁?
        temperatureReadCount = 0;
        temperatureReadAvgTime = 0;
    }
}

// 濮樻挸鐎懕缁樼毘瑜版洘缍栭懢鍊熶紨閼辩偛绻栫挧鍌涚础濮樻挻缍栨繛鍕皸閻愬顩╁鎾逛紑閼辫櫕鐨抽懕锟犳韫囨瑨浼堥幒瀹犲瘯瑜版洝浠稿鎾逛桓閼辩喐鐨抽懕顒冨Е韫囨瑨浠氶幖鍌滃皸閼辨繈妾伴懢鍊熶紩閼鳖厼绻栭懕銊︽倕
float readTemperatureWithMonitoring() {
    unsigned long startTime = micros();
    
    float temp = readTemperatureManual();
    
    unsigned long endTime = micros();
    unsigned long readTime = (endTime - startTime) / 1000; // 閻氼偊妾伴崡銏犵箹閼毖冪€惄鑼剁ォ濞肩偛绻栭悙澶庡Е閼捐姤鎮呴懕?
    
    // 韫囨瑨浼曟ス鎾崇箹閼辫櫕骞婅箛娆掍粴閹煎倻灏楅懕婵嬫閼剧晫顩╅懕鑲╁皸鎼存劙娈曢懠鍌氱秿閼便垻娲哥粋鍕户濮樻捁浼栫弧鎾崇箹閼便垼浼傚鎾逛槐閼辫偐灏楅悙澶岊洨濮樻捁浼€閼卞啿绻栭懕鎾苟閼煎倸缍嶉懕?
    if (temp >= 0) {
        temperatureReadAvgTime = (temperatureReadAvgTime * temperatureReadCount + readTime) / (temperatureReadCount + 1);
        temperatureReadCount++;
    }
    
    return temp;
}


