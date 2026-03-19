/**
 * 按鈕處理 - 電源鍵 + 切換鍵
 * 電源鍵：短/雙擊/三擊觸發功能，長按 5 秒開關機
 * 切換鍵：短按切換模式，長按 5 秒呼叫 AI 語音助理
 */

#include "button_handler.h"
#include "config.h"
#include "op_mode.h"

namespace ButtonHandler {

  static bool lastPower = HIGH, lastMode = HIGH;
  static unsigned long powerDownTime = 0, powerLastRelease = 0;
  static unsigned long modeDownTime = 0;
  static int powerClickCount = 0;
  static bool powerLongFired = false;

  void begin() {
    pinMode(BTN_POWER_PIN, INPUT_PULLUP);
    pinMode(BTN_MODE_PIN, INPUT_PULLUP);
  }

  ButtonEvent tick() {
    bool power = digitalRead(BTN_POWER_PIN);
    bool modeBtn = digitalRead(BTN_MODE_PIN);
    unsigned long now = millis();

    // 切換鍵：短按 = 切換模式，長按 = 呼叫 AI 語音助理
    if (modeBtn == LOW && lastMode == HIGH) {
      modeDownTime = now;
    } else if (modeBtn == HIGH && lastMode == LOW) {
      unsigned long held = now - modeDownTime;
      if (held >= BTN_DEBOUNCE_MS) {
        lastMode = modeBtn;
        unsigned long voiceThreshold = BTN_POWER_HOLD_MS;  // 兩種模式皆長按 5 秒 = 語音
        if (held >= voiceThreshold)
          return ButtonEvent::SceneryLong;  // 長按 = 語音助理（持續模式 5 秒）
        return ButtonEvent::ModeSwitch;     // 短按 = 切換模式
      }
    }
    lastMode = modeBtn;

    ButtonEvent evt = ButtonEvent::None;

    // 電源鍵
    if (power == LOW && lastPower == HIGH) {
      powerDownTime = now;
      powerLongFired = false;
    } else if (power == HIGH && lastPower == LOW) {
      unsigned long held = now - powerDownTime;
      if (held >= BTN_DEBOUNCE_MS) {
        if (held >= BTN_POWER_HOLD_MS) {
          powerLongFired = true;
          evt = ButtonEvent::PowerToggle;  // 長按 5 秒 = 開關機
        } else {
          powerClickCount++;
          powerLastRelease = now;
          evt = ButtonEvent::None;
        }
      }
    } else if (power == LOW && !powerLongFired && (now - powerDownTime >= BTN_POWER_HOLD_MS)) {
      powerLongFired = true;
      evt = ButtonEvent::PowerToggle;
    }
    lastPower = power;

    if (evt != ButtonEvent::None) {
      powerClickCount = 0;
      return evt;
    }

    if (OpMode::isAlwaysOn()) {
      // 持續模式：語音由切換鍵長按 5 秒觸發，電源鍵不做語音
      powerClickCount = 0;
      return ButtonEvent::None;
    }

    if (powerClickCount > 0 && (now - powerLastRelease) >= BTN_DOUBLE_CLICK_MS) {
      if (powerClickCount >= 3) evt = ButtonEvent::TrafficShort;
      else if (powerClickCount == 2) evt = ButtonEvent::ItemSearchShort;
      else evt = ButtonEvent::SceneryShort;
      powerClickCount = 0;
      return evt;
    }

    return ButtonEvent::None;
  }

}  // namespace ButtonHandler
