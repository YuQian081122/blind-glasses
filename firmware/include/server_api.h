/**
 * 伺服器 API 封裝
 * 風景 / 紅綠燈按鈕 -> POST /api/gemini?mode=...
 */

#ifndef SERVER_API_H
#define SERVER_API_H

#include <Arduino.h>
#include <IPAddress.h>
#include <HTTPClient.h>

namespace ServerAPI {

  // 觸發 Gemini 分析 (mode: "general" | "light" | "item_search")
  bool requestGemini(IPAddress serverIP, const char* mode);

  // 上傳 WAV 至 ASR (POST /api/asr)
  bool uploadAudio(IPAddress serverIP, const uint8_t* data, size_t len);

}  // namespace ServerAPI

#endif  // SERVER_API_H
