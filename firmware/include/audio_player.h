/**
 * 語音播放 - HTTP 下載 + I2S (MAX98357A)
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <Arduino.h>
#include <IPAddress.h>

namespace AudioPlayer {

  void begin();
  void tick();

  // 從 serverIP 取得 /audio/latest 並播放
  void playFromServer(IPAddress serverIP);
  void setVolume(uint8_t vol);

  // 是否正在播放
  bool isPlaying();

}  // namespace AudioPlayer

#endif  // AUDIO_PLAYER_H
