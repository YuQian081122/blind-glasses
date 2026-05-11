/**
 * OV3660 MJPEG 串流 - port 81 /stream
 */

#ifndef CAMERA_STREAM_H
#define CAMERA_STREAM_H

#include <Arduino.h>

namespace CameraStream {

  bool begin();
  bool isReady();

  // 拍一張 JPEG 供推幀用；成功時 *outBuf / *outLen 有效，用完須呼叫 releaseFrame()
  bool captureJpeg(const uint8_t** outBuf, size_t* outLen);
  void releaseFrame();

}  // namespace CameraStream

#endif  // CAMERA_STREAM_H
