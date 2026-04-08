/**
 * 語音播放實作 - ESP32-audioI2S + MAX98357A
 */

#include "audio_player.h"
#include "config.h"

#if AUDIO_I2S_ENABLE
#include "Audio.h"
#endif

namespace AudioPlayer {

#if AUDIO_I2S_ENABLE
  static Audio* audio = nullptr;
#endif

  void begin() {
#if !AUDIO_I2S_ENABLE
    Serial.println("[AUDIO] I2S speaker disabled (no amp/speaker)");
#else
    if (audio) return;
    // I2S1：PDM 麥克風固定佔用 I2S0（ESP32-S3 硬體限制），見 mic_upload.cpp
    audio = new Audio(false, 3, I2S_NUM_1);
    audio->setPinout(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DOUT_PIN);
    audio->setVolume(21);  // 0-21
#endif
  }

  void tick() {
#if AUDIO_I2S_ENABLE
    if (audio) {
      audio->loop();
    }
#endif
  }

  void playFromServer(IPAddress serverIP) {
#if !AUDIO_I2S_ENABLE
    (void)serverIP;
#else
    if (!audio) return;
    char url[80];
#if SERVER_HTTP_PORT == 80
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], API_AUDIO_PATH);
#else
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d:%d%s",
             serverIP[0], serverIP[1], serverIP[2], serverIP[3], SERVER_HTTP_PORT, API_AUDIO_PATH);
#endif
    Serial.print("[AUDIO] Playing: ");
    Serial.println(url);
    audio->connecttohost(url);
#endif
  }

  bool isPlaying() {
#if !AUDIO_I2S_ENABLE
    return false;
#else
    if (!audio) return false;
    return audio->isRunning();
#endif
  }

}  // namespace AudioPlayer