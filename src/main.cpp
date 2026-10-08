#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "lwip/etharp.h"
#include "lwip/netif.h"
#include "secrets.h"

#ifndef UPSTREAM_IP
#define UPSTREAM_IP 9, 9, 9, 9
#endif
#ifndef UPSTREAM_PORT
#define UPSTREAM_PORT 53
#endif
static const IPAddress UPSTREAM(UPSTREAM_IP);
static const uint16_t DNS_PORT = 53;
static const char* BLOCKLIST_PATH = "/blocklist.bin";
static const int HASH_BYTES = 5;
static const uint64_t HASH_MASK = (1ULL << (HASH_BYTES * 8)) - 1;
static const int INDEX_ENTRIES = 4096;
static const int CACHE_SIZE = 256;
static const int MAX_RANGE = 256;

WiFiUDP dnsServer, upstreamCli;
WebServer web(80);
File blocklist;
uint32_t numHashes = 0, totalBlocked = 0, totalAllowed = 0;
uint8_t buf[1536];

static uint8_t blIndex[INDEX_ENTRIES][HASH_BYTES];
static uint8_t cacheKey[CACHE_SIZE][HASH_BYTES];
static uint8_t cacheRes[CACHE_SIZE];
static uint8_t cacheValid[CACHE_SIZE];
static uint8_t rangeBuf[MAX_RANGE * HASH_BYTES];

struct Dev { uint32_t ip; uint8_t mac[6]; uint32_t blocked, allowed, lastSeen; bool banned; String label; };
static const int MAX_CLIENTS = 96;
Dev clients[MAX_CLIENTS]; int numClients = 0;

static const int MAX_CUSTOM = 200;
String customDom[MAX_CUSTOM]; uint64_t customHash[MAX_CUSTOM]; int numCustom = 0;

static const int MAX_BAN = 32;
uint32_t bannedIP[MAX_BAN]; int numBanned = 0;

Preferences prefs;
DNSServer   dnsPortal;
String      portalOpts;

bool     blockingOn = true;
uint32_t resumeAt   = 0;

static uint64_t fnv40(const char* s, size_t n) {
  uint64_t h = 0xcbf29ce484222325ULL;
  for (size_t i = 0; i < n; i++) { h ^= (uint8_t)s[i]; h *= 0x100000001b3ULL; }
  return h & HASH_MASK;
}
static inline uint64_t unpackHash(const uint8_t* b) {
  uint64_t v = 0;
  for (int k = 0; k < HASH_BYTES; k++) v |= (uint64_t)b[k] << (8 * k);
  return v;
}
static inline void packHash(uint64_t h, uint8_t* b) {
  for (int k = 0; k < HASH_BYTES; k++) { b[k] = (uint8_t)h; h >>= 8; }
}

static void buildFlashIndex() {
  if (!blocklist || numHashes == 0) return;
  for (int i = 0; i < INDEX_ENTRIES; i++) {
    uint32_t pos = (uint32_t)((uint64_t)i * (numHashes - 1) / (INDEX_ENTRIES - 1));
    blocklist.seek((uint32_t)pos * HASH_BYTES);
    blocklist.read(blIndex[i], HASH_BYTES);
  }
  for (int i = 0; i < CACHE_SIZE; i++) cacheValid[i] = 0;
}

static bool inFlash(uint64_t h) {
  if (numHashes == 0) return false;
  uint64_t first = unpackHash(blIndex[0]);
  uint64_t last  = unpackHash(blIndex[INDEX_ENTRIES - 1]);
  if (h < first || h > last) return false;

  int lo = 0, hi = INDEX_ENTRIES - 2, seg = 0;
  while (lo <= hi) {
    int mid = (lo + hi) >> 1;
    uint64_t midv = unpackHash(blIndex[mid]);
    if (midv < h) {
      seg = mid;
      lo = mid + 1;
    } else if (midv > h) {
      hi = mid - 1;
    } else {
      return true;
    }
  }

  uint32_t startPos = (uint32_t)((uint64_t)seg * (numHashes - 1) / (INDEX_ENTRIES - 1));
  uint32_t endPos   = (uint32_t)((uint64_t)(seg + 1) * (numHashes - 1) / (INDEX_ENTRIES - 1));
  if (endPos >= numHashes) endPos = numHashes - 1;
  uint32_t rangeCount = endPos - startPos + 1;
  if (rangeCount > (uint32_t)MAX_RANGE) rangeCount = MAX_RANGE;

  blocklist.seek((uint32_t)startPos * HASH_BYTES);
  blocklist.read(rangeBuf, (uint32_t)rangeCount * HASH_BYTES);

  for (uint32_t i = 0; i < rangeCount; i++) {
    uint64_t v = unpackHash(rangeBuf + i * HASH_BYTES);
    if (v == h) return true;
    if (v > h) break;
  }
  return false;
}

static bool inCustom(uint64_t h) { for (int i = 0; i < numCustom; i++) if (customHash[i] == h) return true; return false; }

static bool isBlockedHash(uint64_t h) {
  if (inCustom(h)) return true;
  uint32_t slot = h & (CACHE_SIZE - 1);
  if (cacheValid[slot]) {
    uint8_t want[HASH_BYTES]; packHash(h, want);
    bool same = true;
    for (int k = 0; k < HASH_BYTES; k++) if (cacheKey[slot][k] != want[k]) { same = false; break; }
    if (same) return cacheRes[slot] != 0;
  }
  bool res = inFlash(h);
  cacheValid[slot] = 1;
  cacheRes[slot] = res ? 1 : 0;
  packHash(h, cacheKey[slot]);
  return res;
}

static bool isBlocked(const char* domain) {
  const char* p = domain;
  while (p && *p) {
    uint64_t h = fnv40(p, strlen(p));
    if (isBlockedHash(h)) return true;
    const char* dot = strchr(p, '.'); if (!dot) break;
    const char* next = dot + 1; if (!strchr(next, '.')) break; p = next;
  }
  return false;
}

static void loadCustom() {
  numCustom = 0; File f = LittleFS.open("/custom.txt", "r"); if (!f) return;
  while (f.available() && numCustom < MAX_CUSTOM) {
    String l = f.readStringUntil('\n'); l.trim(); l.toLowerCase();
    if (l.length() && l.indexOf('.') > 0) { customDom[numCustom] = l; customHash[numCustom] = fnv40(l.c_str(), l.length()); numCustom++; }
  }
  f.close();
}
static void saveCustom() { File f = LittleFS.open("/custom.txt", "w"); if (!f) return; for (int i = 0; i < numCustom; i++) f.println(customDom[i]); f.close(); }
static bool addCustom(String d) {
  d.trim(); d.toLowerCase(); if (d.startsWith("www.")) d = d.substring(4);
  if (!d.length() || d.indexOf('.') < 0 || numCustom >= MAX_CUSTOM) return false;
  for (int i = 0; i < numCustom; i++) if (customDom[i] == d) return false;
  customDom[numCustom] = d; customHash[numCustom] = fnv40(d.c_str(), d.length()); numCustom++; saveCustom(); return true;
}
static void removeCustom(String d) {
  d.toLowerCase();
  for (int i = 0; i < numCustom; i++) if (customDom[i] == d) {
    for (int j = i; j < numCustom - 1; j++) { customDom[j] = customDom[j+1]; customHash[j] = customHash[j+1]; }
    numCustom--; saveCustom(); return;
  }
}
static bool isBannedIP(uint32_t ip) { for (int i = 0; i < numBanned; i++) if (bannedIP[i] == ip) return true; return false; }
static void loadBanned() {
  numBanned = 0; File f = LittleFS.open("/banned.txt", "r"); if (!f) return;
  while (f.available() && numBanned < MAX_BAN) { String l = f.readStringUntil('\n'); l.trim(); IPAddress ip; if (l.length() && ip.fromString(l)) bannedIP[numBanned++] = (uint32_t)ip; }
  f.close();
}
static void saveBanned() {
  numBanned = 0;
  for (int i = 0; i < numClients && numBanned < MAX_BAN; i++) if (clients[i].banned) bannedIP[numBanned++] = clients[i].ip;
  File f = LittleFS.open("/banned.txt", "w"); if (!f) return;
  for (int i = 0; i < numBanned; i++) { IPAddress ip(bannedIP[i]); f.println(ip.toString()); }
  f.close();
}

static void getMac(uint32_t ip, uint8_t* mac) {
  memset(mac, 0, 6); ip4_addr_t ipa; ipa.addr = ip;
  struct eth_addr* eth = nullptr; const ip4_addr_t* ipret = nullptr;
  for (struct netif* nif = netif_list; nif; nif = nif->next)
    if (etharp_find_addr(nif, &ipa, &eth, &ipret) >= 0 && eth) { memcpy(mac, eth->addr, 6); return; }
}
static Dev* getClient(uint32_t ip) {
  for (int i = 0; i < numClients; i++) if (clients[i].ip == ip) { clients[i].lastSeen = millis(); return &clients[i]; }
  if (numClients < MAX_CLIENTS) {
    Dev* c = &clients[numClients++];
    c->ip = ip; c->blocked = c->allowed = 0; c->lastSeen = millis(); c->banned = isBannedIP(ip); c->label = "";
    getMac(ip, c->mac); return c;
  }
  return nullptr;
}

static size_t parseQuery(const uint8_t* pkt, int len, char* out, uint16_t* qtype, int* qend) {
  if (len < 13) return 0; int i = 12; size_t o = 0;
  while (i < len) { uint8_t l = pkt[i++]; if (l == 0) break; if (l & 0xC0) return 0;
    if (o + l + 1 >= 250 || i + l > len) return 0; if (o) out[o++] = '.';
    for (uint8_t k = 0; k < l; k++) out[o++] = tolower(pkt[i++]); }
  out[o] = 0; if (i + 4 > len) return 0; *qtype = (pkt[i] << 8) | pkt[i + 1]; *qend = i + 4;
  if (o > 4 && strncmp(out, "www.", 4) == 0) { memmove(out, out + 4, o - 3); o -= 4; }
  return o;
}
static int buildBlocked(int qend, uint16_t qtype) {
  buf[2] = 0x81; buf[3] = 0x80; buf[6] = 0; buf[7] = (qtype == 1) ? 1 : 0; buf[8] = 0; buf[9] = 0; buf[10] = 0; buf[11] = 0;
  if (qtype != 1) return qend;
  const uint8_t ans[] = {0xC0,0x0C, 0,1, 0,1, 0,0,1,0x2C, 0,4, 0,0,0,0};
  memcpy(buf + qend, ans, sizeof(ans)); return qend + sizeof(ans);
}
static int forwardUpstream(int qlen, int qend) {
  upstreamCli.flush();
  while (upstreamCli.parsePacket() > 0) upstreamCli.flush();
  const uint8_t cid0 = buf[0], cid1 = buf[1];
  const uint16_t wid = (uint16_t)esp_random();
  uint8_t q[260]; int ql = qend - 12;
  const bool haveQ = ql > 0 && ql <= (int)sizeof(q) && qend <= qlen;
  if (haveQ) memcpy(q, buf + 12, ql);
  buf[0] = wid >> 8; buf[1] = wid & 0xFF;
  upstreamCli.beginPacket(UPSTREAM, UPSTREAM_PORT); upstreamCli.write(buf, qlen); upstreamCli.endPacket();
  const uint32_t t0 = millis();
  while (millis() - t0 < 1000) {
    int sz = upstreamCli.parsePacket();
    if (sz <= 0) { delay(1); continue; }
    const bool fromUp = upstreamCli.remoteIP() == UPSTREAM && upstreamCli.remotePort() == UPSTREAM_PORT;
    int n = upstreamCli.read(buf, sizeof(buf));
    upstreamCli.flush();
    if (!fromUp || n < 12 || sz > (int)sizeof(buf)) continue;
    if (buf[0] != (wid >> 8) || buf[1] != (wid & 0xFF)) continue;
    if (haveQ && (n < 12 + ql || memcmp(buf + 12, q, ql) != 0)) continue;
    buf[0] = cid0; buf[1] = cid1;
    return n;
  }
  return 0;
}
static bool handleDns() {
  bool did = false;
  for (int budget = 0; budget < 16; budget++) {
    int sz = dnsServer.parsePacket(); if (sz <= 0) break;
    did = true;
    IPAddress cip = dnsServer.remoteIP(); uint16_t cport = dnsServer.remotePort();
    int qlen = dnsServer.read(buf, sizeof(buf)); if (qlen < 13) continue;
    char domain[256]; uint16_t qtype = 0; int qend = qlen;
    size_t dl = parseQuery(buf, qlen, domain, &qtype, &qend);
    Dev* c = getClient((uint32_t)cip);
    bool ban = c && c->banned;
    bool blocked = ban || (blockingOn && dl && numHashes && isBlocked(domain));
    int rlen;
    if (blocked) { rlen = buildBlocked(qend, qtype); totalBlocked++; if (c) c->blocked++; }
    else         { rlen = forwardUpstream(qlen, qend);     totalAllowed++; if (c) c->allowed++; }
    if (rlen > 0) { dnsServer.beginPacket(cip, cport); dnsServer.write(buf, rlen); dnsServer.endPacket(); }
  }
  return did;
}

static String macStr(const uint8_t* m) { char s[18]; snprintf(s, sizeof(s), "%02x:%02x:%02x:%02x:%02x:%02x", m[0],m[1],m[2],m[3],m[4],m[5]); return String(s); }
static String jesc(const String& s) { String o; for (char ch : s) { if (ch == '"' || ch == '\\') o += '\\'; o += ch; } return o; }

#include "page.h"

static void handleStats() {
  uint32_t up = millis() / 1000;
  char ut[24]; snprintf(ut, sizeof(ut), "%lud %luh %lum", up/86400, (up%86400)/3600, (up%3600)/60);
  String j = "{\"ip\":\"" + WiFi.localIP().toString() + "\",\"blocked\":" + totalBlocked + ",\"allowed\":" + totalAllowed +
             ",\"domains\":" + numHashes + ",\"rssi\":" + WiFi.RSSI() + ",\"temp\":" + String(temperatureRead(), 1) +
             ",\"heap\":" + ESP.getFreeHeap() + ",\"heapTotal\":" + ESP.getHeapSize() +
             ",\"uptime\":\"" + ut + "\"" +
             ",\"blocking\":" + (blockingOn ? "true" : "false") +
             ",\"resumeIn\":" + (uint32_t)(!blockingOn && resumeAt ? (resumeAt - millis()) / 1000 : 0) +
             ",\"defcreds\":" + ((strcmp(WEB_PASS, "CHANGE_ME_WEB_PASSWORD") == 0) ? "true" : "false") +
             ",\"clients\":[";
  for (int i = 0; i < numClients; i++) { Dev& c = clients[i]; IPAddress ip(c.ip);
    j += (i ? "," : ""); j += "{\"ip\":\"" + ip.toString() + "\",\"mac\":\"" + macStr(c.mac) + "\",\"blocked\":" + c.blocked + ",\"allowed\":" + c.allowed + ",\"banned\":" + (c.banned?"true":"false") + "}"; }
  j += "],\"custom\":[";
  for (int i = 0; i < numCustom; i++) { j += (i ? "," : ""); j += "\"" + jesc(customDom[i]) + "\""; }
  j += "]}";
  web.send(200, "application/json", j);
}
static const char* CSRF_HEADER = "X-Requested-With";
static const char* CSRF_VALUE  = "octo-hole";
static bool requireAuth() {
  if (web.header(CSRF_HEADER) != CSRF_VALUE) { web.send(403, "text/plain", "missing CSRF header"); return false; }
  if (web.authenticate(WEB_USER, WEB_PASS)) return true;
  web.requestAuthentication();
  return false;
}
static void handleBan() {
  if (!requireAuth()) return;
  IPAddress ip; if (ip.fromString(web.arg("ip"))) { Dev* c = getClient((uint32_t)ip); if (c) { c->banned = !c->banned; saveBanned(); } }
  web.send(200, "text/plain", "ok");
}

static void reopenBlocklist() {
  blocklist = LittleFS.open(BLOCKLIST_PATH, "r");
  numHashes = blocklist ? blocklist.size() / HASH_BYTES : 0;
  buildFlashIndex();
}
static void beginBlocklistSwap() {
  if (blocklist) blocklist.close();
  numHashes = 0;
  LittleFS.remove(BLOCKLIST_PATH);
  LittleFS.remove("/blocklist.new");
}
static bool commitNewBlocklist() {
  File f = LittleFS.open("/blocklist.new", "r");
  size_t sz = f ? f.size() : 0; if (f) f.close();
  bool ok = sz > 0 && (sz % HASH_BYTES) == 0;
  if (ok) LittleFS.rename("/blocklist.new", BLOCKLIST_PATH);
  else    LittleFS.remove("/blocklist.new");
  reopenBlocklist();
  return ok;
}

static bool upOk = false;
static bool upAuthOk = false;
static File upFile;
static void handleUploadDone() {
  if (!upAuthOk) { web.requestAuthentication(); return; }
  web.send(upOk ? 200 : 500, "text/plain",
           upOk ? "ok" : "rejected: empty or size not a multiple of 5 (not a blocklist.bin?)");
}
static void handleUpload() {
  HTTPUpload& u = web.upload();
  switch (u.status) {
    case UPLOAD_FILE_START:
      upAuthOk = web.header(CSRF_HEADER) == CSRF_VALUE && web.authenticate(WEB_USER, WEB_PASS);
      if (!upAuthOk) { Serial.println("[ota] blocklist upload: auth/CSRF check failed"); break; }
      upOk = false; beginBlocklistSwap();
      upFile = LittleFS.open("/blocklist.new", "w");
      Serial.printf("[ota] receiving %s\n", u.filename.c_str());
      break;
    case UPLOAD_FILE_WRITE:
      if (upAuthOk && upFile) upFile.write(u.buf, u.currentSize);
      break;
    case UPLOAD_FILE_END:
      if (!upAuthOk) break;
      if (upFile) upFile.close();
      upOk = commitNewBlocklist();
      Serial.printf("[ota] %s -> %u domains\n", upOk ? "OK" : "REJECTED", numHashes);
      break;
    case UPLOAD_FILE_ABORTED:
      if (!upAuthOk) break;
      if (upFile) upFile.close();
      LittleFS.remove("/blocklist.new"); reopenBlocklist();
      Serial.println("[ota] aborted");
      break;
  }
}

static bool hasCreds() {
  prefs.begin("wifi", true); bool nvs = prefs.getString("ssid", "").length() > 0; prefs.end();
  return nvs || (WIFI_SSID && *WIFI_SSID && strcmp(WIFI_SSID, "YOUR_WIFI_SSID") != 0);
}
static bool connectWiFi() {
  prefs.begin("wifi", true);
  String ss = prefs.getString("ssid", "");
  String pw = prefs.getString("pass", "");
  prefs.end();
  const char* ssid = ss.length() ? ss.c_str() : WIFI_SSID;
  const char* pass = ss.length() ? pw.c_str() : WIFI_PASS;
  if (!ssid || !*ssid || strcmp(ssid, "YOUR_WIFI_SSID") == 0) return false;
  Serial.printf("WiFi: connecting to \"%s\"%s\n", ssid, ss.length() ? " (provisioned)" : " (secrets.h)");
  WiFi.mode(WIFI_STA); WiFi.setSleep(false); WiFi.begin(ssid, pass);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 30000) { delay(250); Serial.print("."); }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

static void handlePortalRoot() {
  String html =
    "<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
    "<title>Octo-Hole setup</title>"
    "<body style='font:16px system-ui,sans-serif;max-width:420px;margin:36px auto;padding:0 16px;background:#0d1117;color:#c9d1d9'>"
    "<h2>&#128737; Octo-Hole &mdash; WiFi setup</h2>"
    "<p style='color:#8b949e'>Pick your network and enter its password. The device restarts and joins it.</p>"
    "<form method=POST action=/wifisave>"
    "<input list=nets name=s placeholder='WiFi name' required style='width:100%;box-sizing:border-box;padding:11px;margin:6px 0;border-radius:6px;border:1px solid #30363d;background:#161b22;color:#c9d1d9'>"
    "<datalist id=nets>" + portalOpts + "</datalist>"
    "<input name=p type=password placeholder='Password' style='width:100%;box-sizing:border-box;padding:11px;margin:6px 0;border-radius:6px;border:1px solid #30363d;background:#161b22;color:#c9d1d9'>"
    "<button style='width:100%;padding:12px;margin-top:8px;border-radius:6px;border:0;background:#3fb950;color:#000;font-weight:600;cursor:pointer'>Connect</button>"
    "</form></body>";
  web.send(200, "text/html", html);
}
static void handleWifiSave() {
  String ss = web.arg("s"), pw = web.arg("p");
  if (!ss.length()) { web.send(400, "text/plain", "missing WiFi name"); return; }
  prefs.begin("wifi", false); prefs.putString("ssid", ss); prefs.putString("pass", pw); prefs.end();
  web.send(200, "text/html", "<!doctype html><meta charset=utf-8><body style='font:16px system-ui;text-align:center;margin-top:60px'>"
                             "&#9989; Saved. Restarting and joining <b>" + ss + "</b>&hellip;<br><br>"
                             "Reconnect your phone to your normal WiFi, then find the box at <b>octosrc.local</b>.</body>");
  delay(900); ESP.restart();
}
static void startConfigPortal() {
  int n = WiFi.scanNetworks();
  portalOpts = "";
  for (int i = 0; i < n && i < 15; i++) portalOpts += "<option value='" + jesc(WiFi.SSID(i)) + "'>";
  uint8_t mac[6]; WiFi.macAddress(mac);
  char ap[24]; snprintf(ap, sizeof(ap), "Octo-Hole-%02X%02X", mac[4], mac[5]);
  WiFi.mode(WIFI_AP); WiFi.softAP(ap);
  IPAddress apIP = WiFi.softAPIP();
  dnsPortal.start(53, "*", apIP);
  web.on("/", handlePortalRoot);
  web.on("/wifisave", HTTP_POST, handleWifiSave);
  web.onNotFound(handlePortalRoot);
  web.begin();
  Serial.printf("\n[setup] No WiFi. Join open network \"%s\" and a setup page pops up (or http://%s)\n",
                ap, apIP.toString().c_str());
  const bool configured = hasCreds();
  uint32_t t0 = millis();
  while (true) {
    dnsPortal.processNextRequest(); web.handleClient(); delay(2);
    if (WiFi.softAPgetStationNum() > 0) t0 = millis();
    if (configured && millis() - t0 > 180000UL) { Serial.println("[setup] retrying WiFi"); ESP.restart(); }
  }
}

void setup() {
  Serial.begin(115200); delay(300);
  Serial.println("\n[octo-hole] booting");
  if (!LittleFS.begin(true)) Serial.println("LittleFS FAILED");
  blocklist = LittleFS.open(BLOCKLIST_PATH, "r");
  if (blocklist) {
    numHashes = blocklist.size() / HASH_BYTES;
    Serial.printf("blocklist: %u domains\n", numHashes);
    buildFlashIndex();
  }
  loadCustom(); loadBanned();
  Serial.printf("custom: %d, banned: %d\n", numCustom, numBanned);

#if CONFIG_IDF_TARGET_ESP32C3
  const int BOOT_PIN = 9;
#else
  const int BOOT_PIN = 0;
#endif
  pinMode(BOOT_PIN, INPUT_PULLUP);
  if (digitalRead(BOOT_PIN) == LOW) { delay(60);
    if (digitalRead(BOOT_PIN) == LOW) { prefs.begin("wifi", false); prefs.clear(); prefs.end();
      Serial.println("[setup] BOOT held -> cleared saved WiFi"); } }

  if (!connectWiFi()) startConfigPortal();
  Serial.printf("WiFi up: %s\n", WiFi.localIP().toString().c_str());
  if (MDNS.begin("octosrc")) { MDNS.addService("http", "tcp", 80); Serial.println("dashboard: http://octosrc.local"); }

  if (strcmp(WEB_PASS, "CHANGE_ME_WEB_PASSWORD") == 0)
    Serial.println("[WARN] secrets.h still has placeholder WEB_PASS — it's public "
                    "(it's in the repo's example file). Set a real value before trusting this "
                    "device on a network you don't fully control.");

  dnsServer.begin(DNS_PORT); upstreamCli.begin(0);
  { const char* hdrs[] = { CSRF_HEADER }; web.collectHeaders(hdrs, 1); }
  web.on("/", []() { web.send_P(200, "text/html", PAGE); });
  web.on("/stats.json", handleStats);
  web.on("/ban", handleBan);
  web.on("/addblock", []() { if (!requireAuth()) return; addCustom(web.arg("d")); web.send(200, "text/plain", "ok"); });
  web.on("/unblock", []() { if (!requireAuth()) return; removeCustom(web.arg("d")); web.send(200, "text/plain", "ok"); });
  web.on("/pause", []() {
    if (!requireAuth()) return;
    long s = web.hasArg("s") ? web.arg("s").toInt() : 0;
    blockingOn = false; resumeAt = (s > 0) ? millis() + (uint32_t)s * 1000UL : 0;
    web.send(200, "text/plain", "paused");
  });
  web.on("/resume", []() { if (!requireAuth()) return; blockingOn = true; resumeAt = 0; web.send(200, "text/plain", "resumed"); });
  web.on("/forgetwifi", []() { if (!requireAuth()) return; web.send(200, "text/plain", "cleared — rebooting into setup portal");
    prefs.begin("wifi", false); prefs.clear(); prefs.end(); delay(500); ESP.restart(); });
  web.on("/upload", HTTP_POST, handleUploadDone, handleUpload);
  web.begin();
  Serial.println("DNS :53 + dashboard :80 up");
}

void loop() {
  web.handleClient();
  bool busy = handleDns();
  if (!blockingOn && resumeAt && (int32_t)(millis() - resumeAt) >= 0) { blockingOn = true; resumeAt = 0; }
  if (!busy) delay(1);
}
