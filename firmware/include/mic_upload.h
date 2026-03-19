/**
 * 麥克風錄音上傳 - PDM 麥克風錄製 WAV 並 POST 至 /api/asr
 */

#ifndef MIC_UPLOAD_H
#define MIC_UPLOAD_H

#include <Arduino.h>

namespace MicUpload {

  void begin();
  void tick();  // 主迴圈呼叫

  // 開始錄音 (ItemSearchLong 觸發)，完成後自動上傳
  void startRecording();

  bool isRecording();

  bool hasPendingAudioFetch();
  void clearPendingAudioFetch();
  // 是否該取語音 (已過延遲)
  bool shouldFetchAudio();

}  // namespace MicUpload

#endif  // MIC_UPLOAD_H
