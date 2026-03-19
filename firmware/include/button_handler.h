/**
 * 按鈕處理 - 風景、紅綠燈，支援 debounce 與長按
 */

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// 按鈕事件
enum class ButtonEvent : uint8_t {
  None,
  SceneryShort,    // 風景 (電源鍵短按)
  SceneryLong,     // 語音 (電源鍵長按)
  TrafficShort,    // 紅綠燈 (電源鍵三擊)
  TrafficLong,     // 同 TrafficShort
  ItemSearchShort, // 物品查找 (電源鍵雙擊)
  ItemSearchLong,  // 同 ItemSearchShort
  ModeSwitch,      // 模式切換 (切換鍵短按)
  PowerToggle      // 開/關機 (電源鍵長按 5 秒)
};

namespace ButtonHandler {

  void begin();
  ButtonEvent tick();  // 回傳本次事件，每 frame 呼叫

}  // namespace ButtonHandler

#endif  // BUTTON_HANDLER_H
