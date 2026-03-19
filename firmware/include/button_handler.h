/**
 * 按鈕處理 - 電源鍵 + 切換鍵
 * 電源鍵：短按=紅綠燈，長按 5 秒=開/關機
 * 切換鍵：短按=模式切換，長按 5 秒=AI 語音助理
 * 其餘功能（場景辨識、物品搜尋、導航）需先喚起 AI 語音助理再下達指令
 */

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// 按鈕事件
enum class ButtonEvent : uint8_t {
  None,
  SceneryLong,     // AI 語音助理 (切換鍵長按 5 秒) -> 喚起後用語音下達指令
  TrafficShort,    // 紅綠燈辨識 (電源鍵短按)
  ModeSwitch,      // 模式切換 (切換鍵短按)
  PowerToggle      // 開/關機 (電源鍵長按 5 秒)
};

namespace ButtonHandler {

  void begin();
  ButtonEvent tick();  // 回傳本次事件，每 frame 呼叫

}  // namespace ButtonHandler

#endif  // BUTTON_HANDLER_H
