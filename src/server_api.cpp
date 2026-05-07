/**
 * 伺服器 API 實作
 */

#include "server_api.h"
#include "config.h"

namespace ServerAPI {

  bool requestGemini(IPAddress serverIP, const char* mode) {
    if (serverIP == IPAddress(0, 0, 0, 0)) return false;

    char url[96];
#if SERVER_HTTP_PORT == 80
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d%s?mode=%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3],
             API_GEMINI_PATH, mode);
#else
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d:%d%s?mode=%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3],
             SERVER_HTTP_PORT, API_GEMINI_PATH, mode);
#endif

    HTTPClient http;
    http.begin(url);
    http.setTimeout(API_TIMEOUT_MS);
    int code = http.POST("");
    http.end();

    if (code > 0 && code < 400) {
      Serial.printf("[API] Gemini %s OK: %d\n", mode, code);
      return true;
    }
    Serial.printf("[API] Gemini %s failed: %d\n", mode, code);
    return false;
  }

  bool uploadAudio(IPAddress serverIP, const uint8_t* data, size_t len) {
    if (serverIP == IPAddress(0, 0, 0, 0) || !data || len == 0) return false;

    char url[80];
#if SERVER_HTTP_PORT == 80
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], API_ASR_PATH);
#else
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d:%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], SERVER_HTTP_PORT, API_ASR_PATH);
#endif

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "audio/wav");
    http.setTimeout(API_TIMEOUT_MS);
    int code = http.POST(const_cast<uint8_t*>(data), len);
    http.end();

    if (code > 0 && code < 400) {
      Serial.printf("[API] ASR upload OK: %d\n", code);
      return true;
    }
    Serial.printf("[API] ASR upload failed: %d\n", code);
    return false;
  }

}  // namespace ServerAPI
