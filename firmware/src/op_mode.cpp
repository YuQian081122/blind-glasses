/**
 * 操作模式 - 儲存於 NVS，開機長按切換
 */

#include "op_mode.h"
#include "config.h"

#include <Preferences.h>

namespace OpMode {

  static Preferences prefs;
  static uint8_t currentMode = OP_MODE_SINGLE_BTN;

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
    prefs.begin("blind_glasses", true);
    currentMode = prefs.getUChar("op_mode", OP_MODE_SINGLE_BTN);
    prefs.end();
  }

  void toggle() {
    currentMode = (currentMode == OP_MODE_SINGLE_BTN) ? OP_MODE_ALWAYS_ON : OP_MODE_SINGLE_BTN;
    set(currentMode);
    Serial.printf("[OP] Mode: %s\n", currentMode ? "ALWAYS_ON" : "SINGLE_BTN");
  }

}  // namespace OpMode
