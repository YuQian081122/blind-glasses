/**
 * OV3660 MJPEG 串流 - port 81 /stream
 */

#ifndef CAMERA_STREAM_H
#define CAMERA_STREAM_H

#include <Arduino.h>

namespace CameraStream {

  // 初始化相機與 HTTP 串流伺服器
  bool begin();

  // 是否已就緒
  bool isReady();

}  // namespace CameraStream

#endif  // CAMERA_STREAM_H
