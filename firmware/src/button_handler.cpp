/**
 * 按鈕處理 - 電源鍵 + 切換鍵
 * 電源鍵：短按=紅綠燈辨識，長按 5 秒=開/關機
 * 切換鍵：短按=模式切換，長按 5 秒=AI 語音助理
 */

#include "button_handler.h"
#include "config.h"

namespace ButtonHandler {

  static bool lastPower = HIGH, lastMode = HIGH;
  static unsigned long powerDownTime = 0;
  static unsigned long modeDownTime = 0;
  static bool powerLongFired = false;

  void begin() {
    pinMode(BTN_POWER_PIN, INPUT_PULLUP);
    pinMode(BTN_MODE_PIN, INPUT_PULLUP);
  }

  ButtonEvent tick() {
    bool power = digitalRead(BTN_POWER_PIN);
    bool modeBtn = digitalRead(BTN_MODE_PIN);
    unsigned long now = millis();

    // 切換鍵：短按 = 模式切換，長按 5 秒 = AI 語音助理
    if (modeBtn == LOW && lastMode == HIGH) {
      modeDownTime = now;
    } else if (modeBtn == HIGH && lastMode == LOW) {
      unsigned long held = now - modeDownTime;
      if (held >= BTN_DEBOUNCE_MS) {
        lastMode = modeBtn;
        if (held >= BTN_POWER_HOLD_MS)
          return ButtonEvent::SceneryLong;  // 長按 = AI 語音助理
        return ButtonEvent::ModeSwitch;     // 短按 = 模式切換
      }
    }
    lastMode = modeBtn;

    // 電源鍵：短按 = 紅綠燈辨識，長按 5 秒 = 開/關機
    if (power == LOW && lastPower == HIGH) {
      powerDownTime = now;
      powerLongFired = false;
    } else if (power == HIGH && lastPower == LOW) {
      unsigned long held = now - powerDownTime;
      if (held >= BTN_DEBOUNCE_MS) {
        if (held >= BTN_POWER_HOLD_MS) {
          powerLongFired = true;
          lastPower = power;
          return ButtonEvent::PowerToggle;  // 長按 5 秒 = 開關機
        }
        lastPower = power;
        return ButtonEvent::TrafficShort;   // 短按 = 紅綠燈辨識
      }
    } else if (power == LOW && !powerLongFired && (now - powerDownTime >= BTN_POWER_HOLD_MS)) {
      powerLongFired = true;
      return ButtonEvent::PowerToggle;      // 按住超過 5 秒
    }
    lastPower = power;

    return ButtonEvent::None;
  }

}  // namespace ButtonHandler
