// 測試程式：D8->D2 loopback + 內建 RGB LED + 每 5 秒向伺服器回報

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"
#include "udp_discovery.h"

// XIAO ESP32S3 Sense 上的 RGB LED 接在 GPIO21
static const int LED_PIN = 21;
static const int LED_COUNT = 1;

// D8 當輸出、D2 當輸入（你已經用杜邦線把兩腳對接）
static const int TX_PIN = D8;
static const int RX_PIN = D2;

// 每 5 秒向伺服器回報一次
static const unsigned long REPORT_INTERVAL_MS = 5000;
static unsigned long lastReportTime = 0;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

void connectWifi() {
  Serial.print("Connecting to WiFi ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < WIFI_MAX_RETRY) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connect failed!");
  }
}

void sendReportToServer(bool ok) {
  if (!UdpDiscovery::hasServer()) return;
  IPAddress ip = UdpDiscovery::getServerIP();

  String url = String("http://") + ip.toString() + ":" + String(SERVER_HTTP_PORT) + "/api/gpio_test";
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String body = String("{\"ok\":") + (ok ? "true" : "false") + "}";
  int code = http.POST(body);
  Serial.printf("[TEST] POST %s -> %d, body=%s\n", url.c_str(), code, body.c_str());
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== D8-D2 Loopback + RGB LED + Server Test ===");

  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT_PULLUP);

  strip.begin();
  strip.clear();
  strip.show();

  // 開機時先亮白色 1 秒，表示程式有跑起來
  setLedColor(50, 50, 50);
  delay(1000);
  setLedColor(0, 0, 0);

  connectWifi();
  if (WiFi.status() == WL_CONNECTED) {
    UdpDiscovery::begin();
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    UdpDiscovery::tick();
  }

  // 發送 HIGH
  digitalWrite(TX_PIN, HIGH);
  delay(10);
  int valHigh = digitalRead(RX_PIN);

  // 發送 LOW
  digitalWrite(TX_PIN, LOW);
  delay(10);
  int valLow = digitalRead(RX_PIN);

  Serial.print("TX=D8 HIGH-> RX(D2)=");
  Serial.print(valHigh);
  Serial.print(" , LOW->");
  Serial.println(valLow);

  bool ok = (valHigh == HIGH && valLow == LOW);

  // 根據 loopback 結果改變 LED 顏色
  if (ok) {
    // RX 跟著變化 => 連線正常，亮綠燈
    setLedColor(0, 50, 0);
  } else {
    // 讀值怪怪的 => 亮紅燈
    setLedColor(50, 0, 0);
  }

  // 每 5 秒（且 loopback ok）向伺服器回報一次
  unsigned long now = millis();
  if (ok && now - lastReportTime >= REPORT_INTERVAL_MS) {
    lastReportTime = now;
    sendReportToServer(ok);
  }

  delay(500);
}
