/**
 * UDP 探索實作
 */

#include "udp_discovery.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiUdp.h>

namespace UdpDiscovery {

  static WiFiUDP udp;
  static IPAddress serverIP(0, 0, 0, 0);
  static unsigned long lastSendTime = 0;

  IPAddress getServerIP() {
    return serverIP;
  }

  bool hasServer() {
    return serverIP != IPAddress(0, 0, 0, 0);
  }

  void begin() {
    udp.begin(UDP_DISCOVERY_PORT);
  }

  void tick() {
    if (WiFi.status() != WL_CONNECTED) return;

    // 發送廣播（省電時未找到伺服器則放慢）
    unsigned long now = millis();
    unsigned long interval = UDP_DISCOVERY_INTERVAL_MS;
#if POWER_SAVE_ENABLE
    if (serverIP == IPAddress(0, 0, 0, 0))
      interval = UDP_DISCOVERY_INTERVAL_SLOW_MS;
#endif
    if (now - lastSendTime >= interval) {
      lastSendTime = now;
      IPAddress broadcast(255, 255, 255, 255);
      udp.beginPacket(broadcast, UDP_DISCOVERY_PORT);
      udp.write((const uint8_t*)UDP_DISCOVERY_MSG, strlen(UDP_DISCOVERY_MSG));
      udp.endPacket();
      Serial.println("[UDP] Sent WHO_IS_SERVER");
    }

    // 檢查回覆
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
      char buf[64];
      int len = udp.read(buf, sizeof(buf) - 1);
      if (len > 0) {
        buf[len] = '\0';
        String msg = String(buf);
        if (msg.startsWith(UDP_DISCOVERY_PREFIX)) {
          String ipStr = msg.substring(strlen(UDP_DISCOVERY_PREFIX));
          ipStr.trim();
          if (serverIP.fromString(ipStr)) {
            Serial.print("[UDP] Server found: ");
            Serial.println(serverIP);
          }
        }
      }
    }
  }

}  // namespace UdpDiscovery
