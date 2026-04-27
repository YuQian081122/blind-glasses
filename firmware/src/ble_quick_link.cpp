#include "ble_quick_link.h"
#include "config.h"

#if BLE_QUICK_LINK_ENABLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Preferences.h>
#endif

namespace BleQuickLink {

#if BLE_QUICK_LINK_ENABLE
  static BLEServer* server = nullptr;
  static BLECharacteristic* cWifiSsid = nullptr;
  static BLECharacteristic* cWifiPass = nullptr;
  static BLECharacteristic* cWifiApply = nullptr;
  static BLECharacteristic* cFindMe = nullptr;
  static BLECharacteristic* cStatus = nullptr;
  static Preferences prefs;

  static String pendingSsid;
  static String pendingPass;
  static volatile bool requestWifiApply = false;
  static volatile bool requestFindMe = false;
  static bool wifiConnected = false;
  static IPAddress wifiIp(0, 0, 0, 0);
  static unsigned long lastNotifyMs = 0;

  class WriteCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* c) override {
      std::string v = c->getValue();
      String value(v.c_str());
      if (c == cWifiSsid) {
        pendingSsid = value;
      } else if (c == cWifiPass) {
        pendingPass = value;
      } else if (c == cWifiApply && value == "1") {
        requestWifiApply = true;
      } else if (c == cFindMe && value == "1") {
        requestFindMe = true;
      }
    }
  };

  static WriteCallbacks callbacks;

  static String statusJson() {
    String s = "{";
    s += "\"wifi\":";
    s += (wifiConnected ? "true" : "false");
    s += ",\"ip\":\"";
    s += wifiIp.toString();
    s += "\",\"ssid\":\"";
    s += pendingSsid;
    s += "\"}";
    return s;
  }
#endif

  void begin() {
#if BLE_QUICK_LINK_ENABLE
    prefs.begin("wifi_cfg", false);
    pendingSsid = prefs.getString("ssid", WIFI_SSID);
    pendingPass = prefs.getString("pass", WIFI_PASSWORD);

    BLEDevice::init(BLE_DEVICE_NAME);
    server = BLEDevice::createServer();
    BLEService* service = server->createService(BLE_SERVICE_UUID);

    cWifiSsid = service->createCharacteristic(BLE_WIFI_SSID_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    cWifiPass = service->createCharacteristic(BLE_WIFI_PASS_UUID, BLECharacteristic::PROPERTY_WRITE);
    cWifiApply = service->createCharacteristic(BLE_WIFI_APPLY_UUID, BLECharacteristic::PROPERTY_WRITE);
    cFindMe = service->createCharacteristic(BLE_FIND_ME_UUID, BLECharacteristic::PROPERTY_WRITE);
    cStatus = service->createCharacteristic(BLE_STATUS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

    cWifiSsid->setCallbacks(&callbacks);
    cWifiPass->setCallbacks(&callbacks);
    cWifiApply->setCallbacks(&callbacks);
    cFindMe->setCallbacks(&callbacks);

    cWifiSsid->setValue(pendingSsid.c_str());
    cStatus->setValue("{\"wifi\":false}");

    service->start();
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SERVICE_UUID);
    adv->start();
    Serial.printf("[BLE] Quick link ready: %s\n", BLE_DEVICE_NAME);
#endif
  }

  void tick() {
#if BLE_QUICK_LINK_ENABLE
    const unsigned long now = millis();
    if (now - lastNotifyMs >= BLE_STATUS_NOTIFY_MS) {
      lastNotifyMs = now;
      if (cStatus) {
        String payload = statusJson();
        cStatus->setValue(payload.c_str());
        cStatus->notify();
      }
    }
#endif
  }

  bool hasWifiApplyRequest() {
#if BLE_QUICK_LINK_ENABLE
    return requestWifiApply;
#else
    return false;
#endif
  }

  bool consumeWifiApplyRequest(String& ssid, String& password) {
#if BLE_QUICK_LINK_ENABLE
    if (!requestWifiApply) return false;
    requestWifiApply = false;
    ssid = pendingSsid;
    password = pendingPass;
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    return true;
#else
    (void)ssid;
    (void)password;
    return false;
#endif
  }

  bool consumeFindMeRequest() {
#if BLE_QUICK_LINK_ENABLE
    if (!requestFindMe) return false;
    requestFindMe = false;
    return true;
#else
    return false;
#endif
  }

  void setRuntimeStatus(bool connected, const IPAddress& ip) {
#if BLE_QUICK_LINK_ENABLE
    wifiConnected = connected;
    wifiIp = ip;
#else
    (void)connected;
    (void)ip;
#endif
  }

}  // namespace BleQuickLink
