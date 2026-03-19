/**
 * 操作模式 - 單一按鈕 / 持續監測，支援開機長按切換
 */

#ifndef OP_MODE_H
#define OP_MODE_H

#include <Arduino.h>

namespace OpMode {

  uint8_t get();       // 0=單一按鈕, 1=持續監測
  void set(uint8_t m);
  bool isSingleButton();
  bool isAlwaysOn();

  void begin();   // 從 NVS 載入
  void toggle();  // 切換模式 (由切換鍵呼叫)

}  // namespace OpMode

#endif  // OP_MODE_H
