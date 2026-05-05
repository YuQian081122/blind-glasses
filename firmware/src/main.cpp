/**
 * AI 智慧導盲眼鏡 - 主程式
 * 按鍵：電源鍵短按=紅綠燈、長按=開關機；切換鍵短按=模式、長按=AI 語音助理
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <esp_task_wdt.h>
#include "driver/rtc_io.h"

#include "config.h"
#include "op_mode.h"
#include "udp_discovery.h"
#include "camera_stream.h"
#include "button_handler.h"
#include "audio_player.h"
#include "server_api.h"
#include "imu_stream.h"
#include "mic_upload.h"
#include "gps_stream.h"
#include "ble_quick_link.h"
#include <Preferences.h>

static unsigned long lastAudioTriggerTime = 0;
static const unsigned long AUDIO_FETCH_DELAY_MS = 3000;
static unsigned long lastAutoAudioTestTime = 0;
static bool singleBtnVoiceThenMonitor = false;
static bool voicePlaybackStarted = false;
static unsigned long gpio1ToggleMs = 0;
static bool gpio1LevelHigh = false;
static Preferences wifiPrefs;

void setupWifi() {
  String ssid = WIFI_SSID;
  String pass = WIFI_PASSWORD;
  wifiPrefs.begin("wifi_cfg", false);
  ssid = wifiPrefs.getString("ssid", ssid);
  pass = wifiPrefs.getString("pass", pass);
  wifiPrefs.end();

  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < WIFI_MAX_RETRY) {
    delay(500);
    esp_task_wdt_reset();
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // modem sleep 延後到 setup() 尾端，避免與 UDP/I2S/相機初始化衝突觸發 TWDT
    Serial.print("Stream URL: http://");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.print(STREAM_PORT);
    Serial.println(STREAM_PATH);
  } else {
    Serial.println();
    Serial.println("WiFi connect failed!");
  }
}

void applyWifiFromBle(const String& ssid, const String& pass) {
  if (ssid.isEmpty()) {
    Serial.println("[BLE] WiFi apply ignored: empty ssid");
    return;
  }
  Serial.printf("[BLE] Applying WiFi SSID=%s\n", ssid.c_str());
  WiFi.disconnect(true, true);
  delay(50);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
}

void triggerFindAlert(unsigned long durationMs) {
  (void)durationMs;
  if (UdpDiscovery::hasServer()) {
    Serial.println("[BLE] Find-me alert: play latest server audio");
    AudioPlayer::playFromServer(UdpDiscovery::getServerIP());
    return;
  }
  Serial.println("[BLE] Find-me alert requested, but server not ready");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== AI Smart Blind Glasses ===");

#if GPIO1_TOGGLE_TEST_ENABLE
  pinMode(GPIO1_TOGGLE_TEST_PIN, OUTPUT);
  digitalWrite(GPIO1_TOGGLE_TEST_PIN, LOW);
  gpio1LevelHigh = false;
  gpio1ToggleMs = millis();
  Serial.printf("[GPIO-TEST] pin=%d enabled, interval=%lu ms\n",
                GPIO1_TOGGLE_TEST_PIN, (unsigned long)GPIO1_TOGGLE_INTERVAL_MS);
#endif

#if IMU_STANDALONE_TEST
  Serial.println("[IMU-TEST] IMU_STANDALONE_TEST=1, skip WiFi/Camera/GPS/Mic/Audio/UDP");
  ImuStream::beginStandalone();
  return;
#endif

  OpMode::begin();
  Serial.printf("[OP] Mode: %s\n", OpMode::isSingleButton() ? "SINGLE_BTN" : "ALWAYS_ON");

  setupWifi();
  esp_task_wdt_reset();
  UdpDiscovery::begin();
  esp_task_wdt_reset();
  ButtonHandler::begin();
  esp_task_wdt_reset();
  AudioPlayer::begin();
  esp_task_wdt_reset();
  MicUpload::begin();
  esp_task_wdt_reset();

  if (WiFi.status() == WL_CONNECTED) {
#if POWER_SAVE_ENABLE
    const bool startCam = OpMode::isAlwaysOn() || (CAMERA_START_ON_BOOT);
    if (!startCam) {
      Serial.println("[PWR] Camera off in SINGLE_BTN (power save)");
    } else
#endif
    {
      if (CameraStream::begin()) {
        Serial.println("[CAM] Ready");
      } else {
        Serial.println("[CAM] Init failed");
      }
      esp_task_wdt_reset();
    }
  }
  ImuStream::begin();
  esp_task_wdt_reset();
  GpsStream::begin();
  esp_task_wdt_reset();

  BleQuickLink::begin();
  esp_task_wdt_reset();

#if AUDIO_AUTO_TEST_ENABLE
  lastAutoAudioTestTime = millis();
  Serial.printf("[AUDIO-TEST] enabled, interval=%lu ms\n",
                (unsigned long)AUDIO_AUTO_TEST_INTERVAL_MS);
#endif

#if POWER_SAVE_ENABLE && WIFI_MODEM_SLEEP
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setSleep(true);
    Serial.println("[PWR] WiFi modem sleep on");
  }
#endif
}

void loop() {
#if GPIO1_TOGGLE_TEST_ENABLE
  unsigned long now = millis();
  if (now - gpio1ToggleMs >= GPIO1_TOGGLE_INTERVAL_MS) {
    gpio1ToggleMs = now;
    gpio1LevelHigh = !gpio1LevelHigh;
    digitalWrite(GPIO1_TOGGLE_TEST_PIN, gpio1LevelHigh ? HIGH : LOW);
    Serial.printf("[GPIO-TEST] pin=%d level=%s\n",
                  GPIO1_TOGGLE_TEST_PIN, gpio1LevelHigh ? "HIGH" : "LOW");
  }
#endif

#if IMU_STANDALONE_TEST
  ImuStream::tick();
  delay(5);
  return;
#endif

  UdpDiscovery::tick();
  BleQuickLink::tick();
  BleQuickLink::setRuntimeStatus(WiFi.status() == WL_CONNECTED, WiFi.localIP());
  AudioPlayer::tick();
  MicUpload::tick();

  String bleSsid;
  String blePass;
  if (BleQuickLink::consumeWifiApplyRequest(bleSsid, blePass)) {
    applyWifiFromBle(bleSsid, blePass);
  }
  if (BleQuickLink::consumeFindMeRequest()) {
    triggerFindAlert(10000);
  }
  uint8_t bleOpMode = OP_MODE_DEFAULT;
  String bleTaskMode;
  bool hasBleTaskMode = false;
  if (BleQuickLink::consumeModeRequest(bleOpMode, bleTaskMode, hasBleTaskMode)) {
    OpMode::set(bleOpMode);
    Serial.printf("[BLE] Apply op_mode: %s\n", OpMode::isSingleButton() ? "SINGLE_BTN" : "ALWAYS_ON");
#if POWER_SAVE_ENABLE
    if (OpMode::isAlwaysOn() && !CameraStream::isReady() && WiFi.status() == WL_CONNECTED) {
      if (CameraStream::begin()) Serial.println("[CAM] Ready (ble mode)");
    }
#endif
    if (hasBleTaskMode) {
      if (UdpDiscovery::hasServer()) {
        ServerAPI::requestGemini(UdpDiscovery::getServerIP(), bleTaskMode.c_str());
        lastAudioTriggerTime = millis();
        Serial.printf("[BLE] Trigger task_mode: %s\n", bleTaskMode.c_str());
      } else {
        Serial.printf("[BLE] task_mode=%s ignored: server not ready\n", bleTaskMode.c_str());
      }
    }
  }
  uint8_t newVolume = 0;
  if (BleQuickLink::consumeVolumeRequest(newVolume)) {
    AudioPlayer::setVolume(newVolume);
  }
  if (UdpDiscovery::hasServer()) {
    IPAddress ip = UdpDiscovery::getServerIP();
    ImuStream::setServerIP(ip);
    GpsStream::setServerIP(ip);
    ImuStream::tick();
    GpsStream::tick();
  }

  ButtonEvent evt = ButtonHandler::tick();
  if (evt == ButtonEvent::PowerToggle) {
    Serial.println("[PWR] Power off - deep sleep. Press power button to wake.");
    rtc_gpio_pulldown_dis((gpio_num_t)BTN_POWER_PIN);
    rtc_gpio_pullup_en((gpio_num_t)BTN_POWER_PIN);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_POWER_PIN, 0);
    esp_deep_sleep_start();
  } else if (evt == ButtonEvent::ModeSwitch) {
    OpMode::toggle();
#if POWER_SAVE_ENABLE
    if (OpMode::isAlwaysOn() && !CameraStream::isReady() && WiFi.status() == WL_CONNECTED) {
      if (CameraStream::begin()) Serial.println("[CAM] Ready (mode switch)");
    }
#endif
  } else if (evt != ButtonEvent::None && UdpDiscovery::hasServer()) {
    IPAddress ip = UdpDiscovery::getServerIP();
#if POWER_SAVE_ENABLE
    if (OpMode::isSingleButton() && !CameraStream::isReady() && WiFi.status() == WL_CONNECTED) {
      if (evt == ButtonEvent::TrafficShort || evt == ButtonEvent::SceneryLong) {
        if (CameraStream::begin()) Serial.println("[CAM] Ready (first use)");
      }
    }
#endif
    if (evt == ButtonEvent::TrafficShort) {
      ServerAPI::requestGemini(ip, "light");
      lastAudioTriggerTime = millis();
    } else if (evt == ButtonEvent::SceneryLong) {
      if (OpMode::isSingleButton()) singleBtnVoiceThenMonitor = true;
      MicUpload::startRecording();
    }
  }

  bool fromButton = lastAudioTriggerTime > 0 && (millis() - lastAudioTriggerTime >= AUDIO_FETCH_DELAY_MS);
  bool fromAsr = MicUpload::hasPendingAudioFetch() && MicUpload::shouldFetchAudio();
  bool fromAutoTest = false;
#if AUDIO_AUTO_TEST_ENABLE
  if (UdpDiscovery::hasServer() &&
      (millis() - lastAutoAudioTestTime >= AUDIO_AUTO_TEST_INTERVAL_MS)) {
    fromAutoTest = true;
    lastAutoAudioTestTime = millis();
    Serial.println("[AUDIO-TEST] trigger playFromServer");
  }
#endif
  bool canPlayNormal = (fromButton || fromAsr) && !AudioPlayer::isPlaying() && UdpDiscovery::hasServer();
  bool canPlayAutoTest = fromAutoTest && UdpDiscovery::hasServer();
  if (canPlayNormal || canPlayAutoTest) {
    if (singleBtnVoiceThenMonitor) voicePlaybackStarted = true;
    lastAudioTriggerTime = 0;
    MicUpload::clearPendingAudioFetch();
    if (canPlayAutoTest) {
      Serial.printf("[AUDIO-TEST] forcing playback (isPlaying=%d)\n", AudioPlayer::isPlaying() ? 1 : 0);
    }
    AudioPlayer::playFromServer(UdpDiscovery::getServerIP());
  }

  if (singleBtnVoiceThenMonitor && voicePlaybackStarted && !AudioPlayer::isPlaying()) {
    singleBtnVoiceThenMonitor = false;
    voicePlaybackStarted = false;
    OpMode::set(OP_MODE_ALWAYS_ON);
#if POWER_SAVE_ENABLE
    if (!CameraStream::isReady() && CameraStream::begin()) {
      Serial.println("[CAM] Ready (power save -> ALWAYS_ON)");
    }
#endif
    Serial.println("[OP] Voice done -> ALWAYS_ON (monitoring)");
  }

  unsigned long idleMs = LOOP_IDLE_MS_NORMAL;
#if POWER_SAVE_ENABLE
  if (!AudioPlayer::isPlaying()) idleMs = LOOP_IDLE_MS;
#endif
  delay(idleMs);
}
