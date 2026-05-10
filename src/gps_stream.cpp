/**
 * GPS 串流 - NEO-M8N (UART)
 */

#include "gps_stream.h"
#include "config.h"

#if GPS_ENABLE

#include <HardwareSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#if SERVER_USE_HTTPS
#include <WiFiClientSecure.h>
#endif

#include <TinyGPSPlus.h>

static HardwareSerial GPSerial(1);
static TinyGPSPlus gps;
static IPAddress serverIP(0, 0, 0, 0);
static unsigned long lastSendTime = 0;
static bool ready = false;

namespace GpsStream {

  void setServerIP(IPAddress ip) { serverIP = ip; }
  bool isReady() { return ready; }

  void begin() {
    GPSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    ready = true;
    Serial.println("[GPS] Ready");
  }

  void tick() {
    const int kMaxBytesPerLoop = 256;
    int drained = 0;
    while (GPSerial.available() > 0 && drained < kMaxBytesPerLoop) {
      if (gps.encode(GPSerial.read())) {
        if (gps.location.isValid()) break;
      }
      drained++;
    }
    if (drained >= kMaxBytesPerLoop) {
      yield();
    }

#if !CLOUD_MODE
    if (serverIP == IPAddress(0, 0, 0, 0) || !gps.location.isValid()) return;
#else
    if (!gps.location.isValid()) return;
#endif

    unsigned long now = millis();
    unsigned long interval = GPS_SEND_INTERVAL_MS;
#if POWER_SAVE_ENABLE
    interval = GPS_SEND_INTERVAL_PS_MS;
#endif
    if (now - lastSendTime < interval) return;
    lastSendTime = now;

    char url[128];
#if CLOUD_MODE
    snprintf(url, sizeof(url), "https://%s%s", SERVER_HOST, API_GPS_PATH);
#elif SERVER_HTTP_PORT == 80
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], API_GPS_PATH);
#else
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d:%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], SERVER_HTTP_PORT, API_GPS_PATH);
#endif

    StaticJsonDocument<128> doc;
    doc["lat"] = gps.location.lat();
    doc["lng"] = gps.location.lng();
    doc["alt"] = gps.altitude.meters();
    doc["sat"] = gps.satellites.value();

    char body[140];
    serializeJson(doc, body);

    HTTPClient http;
#if SERVER_USE_HTTPS
    WiFiClientSecure client;
    client.setInsecure();
    http.begin(client, url);
#else
    http.begin(url);
#endif
    http.addHeader("Content-Type", "application/json");
#if CLOUD_MODE
    const char* devToken = DEVICE_API_TOKEN;
    if (devToken && devToken[0]) http.addHeader("X-Device-Token", devToken);
#endif
    http.setTimeout(3000);
    http.POST(body);
    http.end();
  }

}  // namespace GpsStream

#else  // !GPS_ENABLE

namespace GpsStream {

  void setServerIP(IPAddress) {}
  bool isReady() { return false; }

  void begin() { Serial.println("[GPS] Disabled (no antenna / not used)"); }

  void tick() {}

}  // namespace GpsStream

#endif  // GPS_ENABLE
