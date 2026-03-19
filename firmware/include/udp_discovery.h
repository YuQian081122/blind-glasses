/**
 * UDP 探索 - 自動尋找伺服器
 * 廣播 WHO_IS_SERVER，接收 SERVER_IP: x.x.x.x
 */

#ifndef UDP_DISCOVERY_H
#define UDP_DISCOVERY_H

#include <Arduino.h>
#include <IPAddress.h>

namespace UdpDiscovery {

  // 取得已探索到的伺服器 IP (未探索到則為 0.0.0.0)
  IPAddress getServerIP();

  // 是否已取得伺服器 IP
  bool hasServer();

  // 執行一次探索 (發送 + 等待回覆)
  void tick();

  // 初始化 (在 WiFi 連線後呼叫)
  void begin();

}  // namespace UdpDiscovery

#endif  // UDP_DISCOVERY_H
