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

static unsigned long lastAudioTriggerTime = 0;
static const unsigned long AUDIO_FETCH_DELAY_MS = 3000;
static bool singleBtnVoiceThenMonitor = false;
static bool voicePlaybackStarted = false;
static unsigned long gpio1ToggleMs = 0;
static bool gpio1LevelHigh = false;

void setupWifi() {
  Serial.print("Connecting to WiFi ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== AI Smart Blind Glasses ===");

#if IMU_STANDALONE_TEST
  Serial.println("[IMU-TEST] IMU_STANDALONE_TEST=1, skip WiFi/Camera/GPS/Mic/Audio/UDP");
#if GPIO1_TOGGLE_TEST_ENABLE
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  gpio1LevelHigh = false;
  gpio1ToggleMs = millis();
  Serial.printf("[GPIO1] Toggle test enabled, interval=%lu ms\n", (unsigned long)GPIO1_TOGGLE_INTERVAL_MS);
#endif
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

#if POWER_SAVE_ENABLE && WIFI_MODEM_SLEEP
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setSleep(true);
    Serial.println("[PWR] WiFi modem sleep on");
  }
#endif
}

void loop() {
#if IMU_STANDALONE_TEST
#if GPIO1_TOGGLE_TEST_ENABLE
  unsigned long now = millis();
  if (now - gpio1ToggleMs >= GPIO1_TOGGLE_INTERVAL_MS) {
    gpio1ToggleMs = now;
    gpio1LevelHigh = !gpio1LevelHigh;
    digitalWrite(1, gpio1LevelHigh ? HIGH : LOW);
    Serial.printf("[GPIO1] level=%s\n", gpio1LevelHigh ? "HIGH" : "LOW");
  }
#endif
  ImuStream::tick();
  delay(5);
  return;
#endif

  UdpDiscovery::tick();
  AudioPlayer::tick();
  MicUpload::tick();
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
  if ((fromButton || fromAsr) && !AudioPlayer::isPlaying() && UdpDiscovery::hasServer()) {
    if (singleBtnVoiceThenMonitor) voicePlaybackStarted = true;
    lastAudioTriggerTime = 0;
    MicUpload::clearPendingAudioFetch();
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
