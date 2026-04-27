#ifndef BLE_QUICK_LINK_H
#define BLE_QUICK_LINK_H

#include <Arduino.h>
#include <IPAddress.h>

namespace BleQuickLink {

  void begin();
  void tick();

  bool hasWifiApplyRequest();
  bool consumeWifiApplyRequest(String& ssid, String& password);
  bool consumeFindMeRequest();

  void setRuntimeStatus(bool wifiConnected, const IPAddress& ip);

}  // namespace BleQuickLink

#endif  // BLE_QUICK_LINK_H
