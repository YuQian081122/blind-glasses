/**
 * GPS 串流 - NEO-M8N (UART)
 */

#include "gps_stream.h"
#include "config.h"

#include <HardwareSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

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
    while (GPSerial.available() > 0) {
      if (gps.encode(GPSerial.read())) {
        if (gps.location.isValid()) break;
      }
    }
    if (serverIP == IPAddress(0, 0, 0, 0) || !gps.location.isValid()) return;

    unsigned long now = millis();
    unsigned long interval = GPS_SEND_INTERVAL_MS;
#if POWER_SAVE_ENABLE
    interval = GPS_SEND_INTERVAL_PS_MS;
#endif
    if (now - lastSendTime < interval) return;
    lastSendTime = now;

    char url[80];
#if SERVER_HTTP_PORT == 80
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
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(3000);
    int code = http.POST(body);
    http.end();
  }

}  // namespace GpsStream
