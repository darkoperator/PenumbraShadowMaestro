#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Preferences.h>

#include "pin-map.h"

// Minimal Maestro restart-at-sub helper
static inline void maestroRestartAtSub(Stream& port, uint8_t subIndex)
{
  uint8_t pkt[2] = { 0xA7, subIndex };
  port.write(pkt, sizeof(pkt));
}

// ESPNOW message (mirror of EspNowProtocol.h)
struct __attribute__((packed)) SmNowMsg {
  uint8_t type;
  uint8_t subIndex;
};
static const uint8_t kSmNowType_DomeSub = 1;

// Preferences-free for simplicity; default Maestro baud
static int maestroBaud = 9600;
static bool gEncEnabled = false;
static char gLmkHex[33] = {0};
static uint8_t gPeerMac[6] = {0};
static Preferences prefs;

static bool hexNibble(char c, uint8_t& v) {
  if (c>='0' && c<='9') { v = (uint8_t)(c - '0'); return true; }
  c = (char)toupper((unsigned char)c);
  if (c>='A' && c<='F') { v = (uint8_t)(10 + c - 'A'); return true; }
  return false;
}
static bool parseHexKey(const char* hex, uint8_t out[16]) {
  if (!hex) return false; size_t n=strlen(hex); if (n!=32) return false;
  for (int i=0;i<16;++i){uint8_t hi,lo; if(!hexNibble(hex[2*i],hi)||!hexNibble(hex[2*i+1],lo)) return false; out[i]=(uint8_t)((hi<<4)|lo);} return true;
}
static bool parseMac(const char* s, uint8_t mac[6]) {
  int v[6]; if (sscanf(s, "%x:%x:%x:%x:%x:%x", &v[0],&v[1],&v[2],&v[3],&v[4],&v[5])!=6) return false;
  for (int i=0;i<6;++i) mac[i]=(uint8_t)v[i]; return true;
}
static void formatMac(const uint8_t mac[6]) {
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}

// RX callback
static void onEspNowRecv(const uint8_t* mac, const uint8_t* data, int len)
{
  if (len < (int)sizeof(SmNowMsg)) return;
  SmNowMsg m; memcpy(&m, data, sizeof(m));
  if (m.type == kSmNowType_DomeSub) {
    maestroRestartAtSub(MAESTRO_SERIAL, m.subIndex);
  } else if (m.type == 100 /*Pair*/) {
    // Add sender as peer with current enc settings
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 0; peer.encrypt = gEncEnabled;
    if (gEncEnabled) { uint8_t lmk[16]; if (parseHexKey(gLmkHex, lmk)) memcpy(peer.lmk, lmk, 16); else peer.encrypt=false; }
    esp_now_del_peer(mac);
    if (esp_now_add_peer(&peer)==ESP_OK) {
      memcpy(gPeerMac, mac, 6);
      Serial.print("Paired peer: "); formatMac(gPeerMac);
    } else {
      Serial.println("Pair failed");
    }
  }
}

static void printHelp()
{
  Serial.println("Head Node Commands:");
  Serial.println("#SMHELP            : Show help");
  Serial.println("#SMESPMAC          : Show this ESP32 WiFi MAC");
  Serial.println("#SMMARCBAUD <baud> : Set Maestro baud and re-init");
  Serial.println("#SMENC <0|1>       : Disable/Enable ESP-NOW encryption");
  Serial.println("#SMKEY <32HEX>     : Set ESP-NOW LMK (16 bytes as 32 hex chars)");
  Serial.println("#SMPEER <mac>      : Set/add peer MAC");
  Serial.println("#SMPEERSTATUS      : Show peer presence and details");
  Serial.println("#SMCONFIG          : Show current configuration");
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("Head/Dome Maestro Node");

  // Init Maestro serial
  MAESTRO_SERIAL_INIT(maestroBaud);

  // Load settings
  prefs.begin("head_node", false);
  gEncEnabled = prefs.getBool("enc", false);
  String key = prefs.getString("lmk", ""); if (key.length()==32) { strncpy(gLmkHex, key.c_str(), 32); gLmkHex[32]='\0'; }
  String peer = prefs.getString("peer", ""); if (peer.length()>=17) parseMac(peer.c_str(), gPeerMac);

  // ESP-NOW init
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
  } else {
    esp_now_register_recv_cb(onEspNowRecv);
    if (gPeerMac[0]|gPeerMac[1]|gPeerMac[2]|gPeerMac[3]|gPeerMac[4]|gPeerMac[5]) {
      esp_now_peer_info_t p={}; memcpy(p.peer_addr,gPeerMac,6); p.channel=0; p.encrypt=gEncEnabled; if (gEncEnabled){uint8_t lmk[16]; if(parseHexKey(gLmkHex,lmk)) memcpy(p.lmk,lmk,16); else p.encrypt=false;} esp_now_add_peer(&p);
    }
  }
}

void loop()
{
  // Simple serial command parser
  static String line;
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\r' || ch == '\n') {
      line.trim();
      if (line.equalsIgnoreCase("#SMHELP")) {
        printHelp();
      } else if (line.equalsIgnoreCase("#SMESPMAC")) {
        uint8_t mac[6]; esp_read_mac(mac, ESP_MAC_WIFI_STA);
        Serial.printf("WiFi MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
      } else if (line.startsWith("#SMMARCBAUD")) {
        long b = line.substring(String("#SMMARCBAUD").length()).toInt();
        if (b > 0) { maestroBaud = (int)b; MAESTRO_SERIAL.end(); MAESTRO_SERIAL_INIT(maestroBaud); Serial.println("Maestro baud updated"); }
        else Serial.println("Usage: #SMMARCBAUD <baud>");
      } else if (line.startsWith("#SMENC")) {
        long v = line.substring(6).toInt(); gEncEnabled = (v!=0); prefs.putBool("enc", gEncEnabled); Serial.printf("Encryption %s\n", gEncEnabled?"Enabled":"Disabled");
      } else if (line.startsWith("#SMKEY")) {
        String k = line.substring(6); k.trim(); if (k.length()==32) { strncpy(gLmkHex,k.c_str(),32); gLmkHex[32]='\0'; prefs.putString("lmk", k); Serial.println("LMK set."); } else Serial.println("Usage: #SMKEY <32HEX>");
      } else if (line.equalsIgnoreCase("#SMPEERSTATUS")) {
        bool exists = (gPeerMac[0]|gPeerMac[1]|gPeerMac[2]|gPeerMac[3]|gPeerMac[4]|gPeerMac[5]) && esp_now_is_peer_exist(gPeerMac);
        Serial.println("Peer Status:");
        Serial.print("  Configured Peer: "); if (gPeerMac[0]|gPeerMac[1]|gPeerMac[2]|gPeerMac[3]|gPeerMac[4]|gPeerMac[5]) formatMac(gPeerMac); else Serial.println("(none)");
        Serial.printf("  Peer Added:      %s\n", exists?"Yes":"No");
        Serial.printf("  Encryption:      %s\n", gEncEnabled?"Enabled":"Disabled");
      } else if (line.equalsIgnoreCase("#SMCONFIG")) {
        uint8_t mac[6]; esp_read_mac(mac, ESP_MAC_WIFI_STA);
        Serial.println("Head Node Config:");
        Serial.print("  WiFi MAC:        "); formatMac(mac);
        Serial.printf("  Maestro Baud:    %d\n", maestroBaud);
        Serial.printf("  Encryption:      %s\n", gEncEnabled?"Enabled":"Disabled");
        Serial.printf("  LMK Set:         %s\n", (strlen(gLmkHex)==32)?"Yes":"No");
        Serial.print("  Peer MAC:        "); if (gPeerMac[0]|gPeerMac[1]|gPeerMac[2]|gPeerMac[3]|gPeerMac[4]|gPeerMac[5]) formatMac(gPeerMac); else Serial.println("(none)");
        bool exists = (gPeerMac[0]|gPeerMac[1]|gPeerMac[2]|gPeerMac[3]|gPeerMac[4]|gPeerMac[5]) && esp_now_is_peer_exist(gPeerMac);
        Serial.printf("  Peer Added:      %s\n", exists?"Yes":"No");
      } else if (line.startsWith("#SMPEER")) {
        String m = line.substring(7); m.trim(); if (parseMac(m.c_str(), gPeerMac)) { prefs.putString("peer", m); esp_now_peer_info_t p={}; memcpy(p.peer_addr,gPeerMac,6); p.channel=0; p.encrypt=gEncEnabled; if(gEncEnabled){uint8_t lmk[16]; if(parseHexKey(gLmkHex,lmk)) memcpy(p.lmk,lmk,16); else p.encrypt=false;} esp_now_del_peer(gPeerMac); esp_now_add_peer(&p); Serial.print("Peer set: "); formatMac(gPeerMac);} else Serial.println("Usage: #SMPEER <AA:BB:CC:DD:EE:FF>");
      } else if (line.length()) {
        Serial.printf("Unknown: %s\n", line.c_str());
        printHelp();
      }
      line = String();
    } else {
      line += ch;
    }
  }
}
