/**
 * 操作模式 - 儲存於 NVS，開機長按切換
 */

#include "op_mode.h"
#include "config.h"

#include <Preferences.h>

namespace OpMode {

  static Preferences prefs;
  static uint8_t currentMode = OP_MODE_DEFAULT;

  uint8_t get() {
    return currentMode;
  }

  void set(uint8_t m) {
    if (m > 1) m = 0;
    currentMode = m;
    prefs.begin("blind_glasses", false);
    prefs.putUChar("op_mode", m);
    prefs.end();
  }

  bool isSingleButton() {
    return currentMode == OP_MODE_SINGLE_BTN;
  }

  bool isAlwaysOn() {
    return currentMode == OP_MODE_ALWAYS_ON;
  }

  void begin() {
    // readOnly=true 在命名空間尚未建立時會 nvs_open NOT_FOUND；改用 RW 以便首次開機建立
    if (!prefs.begin("blind_glasses", false)) {
      currentMode = OP_MODE_DEFAULT;
      return;
    }
    currentMode = prefs.getUChar("op_mode", OP_MODE_DEFAULT);
    prefs.end();
  }

  void toggle() {
    currentMode = (currentMode == OP_MODE_SINGLE_BTN) ? OP_MODE_ALWAYS_ON : OP_MODE_SINGLE_BTN;
    set(currentMode);
    Serial.printf("[OP] Mode: %s\n", currentMode ? "ALWAYS_ON" : "SINGLE_BTN");
  }

}  // namespace OpMode
