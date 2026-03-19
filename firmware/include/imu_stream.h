/**
 * IMU 資料串流 - ICM-20948
 * 定時讀取並上傳加速度、陀螺儀至伺服器
 */

#ifndef IMU_STREAM_H
#define IMU_STREAM_H

#include <Arduino.h>
#include <IPAddress.h>

namespace ImuStream {

  void begin();
  void tick();  // 在主迴圈呼叫，處理讀取與上傳

  bool isReady();
  void setServerIP(IPAddress ip);

}  // namespace ImuStream

#endif  // IMU_STREAM_H
