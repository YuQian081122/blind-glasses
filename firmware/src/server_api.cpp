/**
 * 伺服器 API 實作
 * CLOUD_MODE=1 時使用 HTTPS + hostname；=0 時使用 HTTP + IP
 */

#include "server_api.h"
#include "config.h"

#if SERVER_USE_HTTPS
#include <WiFiClientSecure.h>
#endif

namespace ServerAPI {

  static void buildUrl(char* out, size_t outLen, IPAddress serverIP, const char* path, const char* query) {
#if CLOUD_MODE
    if (query && query[0]) {
      snprintf(out, outLen, "https://%s%s?%s", SERVER_HOST, path, query);
    } else {
      snprintf(out, outLen, "https://%s%s", SERVER_HOST, path);
    }
    (void)serverIP;
#else
  #if SERVER_HTTP_PORT == 80
    if (query && query[0]) {
      snprintf(out, outLen, "http://%d.%d.%d.%d%s?%s",
               serverIP[0], serverIP[1], serverIP[2], serverIP[3], path, query);
    } else {
      snprintf(out, outLen, "http://%d.%d.%d.%d%s",
               serverIP[0], serverIP[1], serverIP[2], serverIP[3], path);
    }
  #else
    if (query && query[0]) {
      snprintf(out, outLen, "http://%d.%d.%d.%d:%d%s?%s",
               serverIP[0], serverIP[1], serverIP[2], serverIP[3], SERVER_HTTP_PORT, path, query);
    } else {
      snprintf(out, outLen, "http://%d.%d.%d.%d:%d%s",
               serverIP[0], serverIP[1], serverIP[2], serverIP[3], SERVER_HTTP_PORT, path);
    }
  #endif
#endif
  }

  static void addDeviceToken(HTTPClient& http) {
#if CLOUD_MODE
    const char* token = DEVICE_API_TOKEN;
    if (token && token[0]) {
      http.addHeader("X-Device-Token", token);
    }
#else
    (void)http;
#endif
  }

  bool requestGemini(IPAddress serverIP, const char* mode) {
#if !CLOUD_MODE
    if (serverIP == IPAddress(0, 0, 0, 0)) return false;
#endif

    unsigned long t0 = millis();
    char url[128];
    char query[32];
    snprintf(query, sizeof(query), "mode=%s", mode);
    buildUrl(url, sizeof(url), serverIP, API_GEMINI_PATH, query);

    HTTPClient http;
#if SERVER_USE_HTTPS
    WiFiClientSecure client;
    client.setInsecure();
    http.begin(client, url);
#else
    http.begin(url);
#endif
    addDeviceToken(http);
    http.setTimeout(API_TIMEOUT_MS);
    int code = http.POST("");
    http.end();

    unsigned long elapsed = millis() - t0;
    if (code > 0 && code < 400) {
      Serial.printf("[LAT] Gemini %s OK: %dms\n", mode, (int)elapsed);
      return true;
    }
    Serial.printf("[LAT] Gemini %s FAIL(%d): %dms\n", mode, code, (int)elapsed);
    return false;
  }

  bool uploadAudio(IPAddress serverIP, const uint8_t* data, size_t len) {
#if !CLOUD_MODE
    if (serverIP == IPAddress(0, 0, 0, 0) || !data || len == 0) return false;
#else
    if (!data || len == 0) return false;
#endif

    unsigned long t0 = millis();
    char url[128];
    buildUrl(url, sizeof(url), serverIP, API_ASR_PATH, nullptr);

    HTTPClient http;
#if SERVER_USE_HTTPS
    WiFiClientSecure client;
    client.setInsecure();
    http.begin(client, url);
#else
    http.begin(url);
#endif
    http.addHeader("Content-Type", "audio/wav");
    addDeviceToken(http);
    http.setTimeout(API_TIMEOUT_MS);
    int code = http.POST(const_cast<uint8_t*>(data), len);
    http.end();

    unsigned long elapsed = millis() - t0;
    if (code > 0 && code < 400) {
      Serial.printf("[LAT] ASR upload OK: %dms (%u bytes)\n", (int)elapsed, (unsigned)len);
      return true;
    }
    Serial.printf("[LAT] ASR upload FAIL(%d): %dms\n", code, (int)elapsed);
    return false;
  }

}  // namespace ServerAPI
