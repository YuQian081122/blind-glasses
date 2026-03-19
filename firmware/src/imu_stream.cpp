/**
 * IMU 串流實作 - ICM-20948 (I2C)
 */

#include "imu_stream.h"
#include "config.h"

#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "ICM_20948.h"

namespace ImuStream {

  static ICM_20948_I2C imu;
  static IPAddress serverIP(0, 0, 0, 0);
  static unsigned long lastSendTime = 0;
  static bool ready = false;

  void setServerIP(IPAddress ip) {
    serverIP = ip;
  }

  bool isReady() {
    return ready;
  }

  void begin() {
    Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN);
    // AD0=LOW -> 0x68；AD0=HIGH -> 0x69
    bool ad0val = (IMU_I2C_ADDR == 0x69);
    if (imu.begin(Wire, ad0val, ICM_20948_ARD_UNUSED_PIN) != ICM_20948_Stat_Ok) {
      Serial.println("[IMU] Init failed");
      return;
    }
    ready = true;
    Serial.println("[IMU] Ready (ICM-20948)");
  }

  void tick() {
    if (!ready || serverIP == IPAddress(0, 0, 0, 0)) return;

    unsigned long now = millis();
    unsigned long interval = IMU_SEND_INTERVAL_MS;
#if POWER_SAVE_ENABLE
    interval = IMU_SEND_INTERVAL_PS_MS;
#endif
    if (now - lastSendTime < interval) return;
    lastSendTime = now;

    imu.getAGMT();
    if (imu.status != ICM_20948_Stat_Ok) return;
    // acc 為 milli-g，換算成 g；gyr 為 dps
    float ax = imu.accX() / 1000.0f;
    float ay = imu.accY() / 1000.0f;
    float az = imu.accZ() / 1000.0f;
    float gx = imu.gyrX();
    float gy = imu.gyrY();
    float gz = imu.gyrZ();

    char url[80];
#if SERVER_HTTP_PORT == 80
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], API_IMU_PATH);
#else
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d:%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], SERVER_HTTP_PORT, API_IMU_PATH);
#endif

    StaticJsonDocument<128> doc;
    doc["ax"] = round(ax * 100) / 100.0f;
    doc["ay"] = round(ay * 100) / 100.0f;
    doc["az"] = round(az * 100) / 100.0f;
    doc["gx"] = round(gx * 100) / 100.0f;
    doc["gy"] = round(gy * 100) / 100.0f;
    doc["gz"] = round(gz * 100) / 100.0f;

    char body[140];
    serializeJson(doc, body);

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(2000);
    int code = http.POST(body);
    http.end();
  }

}  // namespace ImuStream
