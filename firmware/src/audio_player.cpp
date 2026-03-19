/**
 * 語音播放實作 - ESP32-audioI2S + MAX98357A
 */

#include "audio_player.h"
#include "config.h"

#include "Audio.h"

namespace AudioPlayer {

  static Audio* audio = nullptr;

  void begin() {
    if (audio) return;
    audio = new Audio();
    audio->setPinout(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DOUT_PIN);
    audio->setVolume(21);  // 0-21
  }

  void tick() {
    if (audio) {
      audio->loop();
    }
  }

  void playFromServer(IPAddress serverIP) {
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
  }

  bool isPlaying() {
    if (!audio) return false;
    return audio->isRunning();
  }

}  // namespace AudioPlayer
