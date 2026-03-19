/**
 * AI 智慧導盲眼鏡 - 主程式
 * 按鍵：電源鍵短按=紅綠燈、長按=開關機；切換鍵短按=模式、長按=AI 語音助理
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>
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

void setupWifi() {
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

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#if POWER_SAVE_ENABLE && WIFI_MODEM_SLEEP
    WiFi.setSleep(true);
    Serial.println("[PWR] WiFi modem sleep on");
#endif
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

  OpMode::begin();
  Serial.printf("[OP] Mode: %s\n", OpMode::isSingleButton() ? "SINGLE_BTN" : "ALWAYS_ON");

  setupWifi();
  UdpDiscovery::begin();
  ButtonHandler::begin();
  AudioPlayer::begin();
  MicUpload::begin();

  if (WiFi.status() == WL_CONNECTED) {
#if POWER_SAVE_ENABLE
    if (OpMode::isAlwaysOn()) {
#endif
      if (CameraStream::begin()) {
        Serial.println("[CAM] Ready");
      } else {
        Serial.println("[CAM] Init failed");
      }
#if POWER_SAVE_ENABLE
    } else {
      Serial.println("[PWR] Camera off in SINGLE_BTN (power save)");
    }
#endif
  }
  ImuStream::begin();
  GpsStream::begin();
}

void loop() {
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
