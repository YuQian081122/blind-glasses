/**
 * AI 智慧導盲眼鏡 - 韌體設定
 * 請依實際接線修改 WiFi 與 GPIO
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============ WiFi ============
#define WIFI_SSID       "奇異鳥吃飛魚"
#define WIFI_PASSWORD   "11091122"
#define WIFI_MAX_RETRY  20

// ============ UDP 探索 ============
#define UDP_DISCOVERY_PORT     9999
#define UDP_DISCOVERY_MSG      "WHO_IS_SERVER"
#define UDP_DISCOVERY_PREFIX   "SERVER_IP: "
#define UDP_DISCOVERY_INTERVAL_MS      3000   // 已找到伺服器時
#define UDP_DISCOVERY_INTERVAL_SLOW_MS 8000   // 省電且尚未找到伺服器時

// ============ HTTP 串流 ============
#define STREAM_PORT     81
#define STREAM_PATH     "/stream"

// ============ 操作模式 ============
// 0 = 單一按鈕模式 (短/長/雙擊/三擊區分功能)
// 1 = 持續監測模式 (按鈕僅語音觸發，避障由伺服器持續處理)
#define OP_MODE_SINGLE_BTN  0
#define OP_MODE_ALWAYS_ON   1

// ============ 按鈕 - 電源鍵與切換鍵 ============
#define BTN_POWER_PIN       35   // 電源鍵：功能觸發 (風景/語音/物品/紅綠燈)
#define BTN_MODE_PIN        37   // 切換鍵：單一按鈕 ↔ 持續監測 模式切換
#define BTN_SINGLE_PIN      BTN_POWER_PIN
#define BTN_DEBOUNCE_MS     50
#define BTN_POWER_HOLD_MS   5000  // 電源鍵長按 5 秒 = 開/關機；切換鍵長按 5 秒 = 語音助理

// ============ I2S (MAX98357A) ============
// BCLK, LRC/WS, DOUT (可依接線修改)
#define I2S_BCLK_PIN    36   // D3
#define I2S_LRC_PIN     34   // D5
#define I2S_DOUT_PIN    43   // D7 (TX)

// ============ IMU (ICM-20948) ============
#define IMU_SDA_PIN        7    // 可依接線修改，避開相機/I2S/按鈕
#define IMU_SCL_PIN        8
#define IMU_I2C_ADDR       0x68 // AD0=LOW；若模組 AD0 接 VCC 則為 0x69，begin 時 ad0val=true
#define IMU_SEND_INTERVAL_MS    100   // 一般
#define IMU_SEND_INTERVAL_PS_MS 200   // 省電時拉長（POWER_SAVE_ENABLE）

// ============ 麥克風 (PDM) ============
#define MIC_RECORD_SEC     4
#define MIC_SAMPLE_RATE    16000

// ============ 按鈕 - 單一按鈕手勢 ============
#define BTN_ITEM_PIN       -1
#define BTN_DOUBLE_CLICK_MS  400
#define BTN_TRIPLE_CLICK_MS  600   // 三擊間隔

// ============ 伺服器 API ============
#define SERVER_HTTP_PORT    5000  // 與 server/config.py HTTP_PORT 一致；正式環境可改 80
#define API_GEMINI_PATH     "/api/gemini"
#define API_ASR_PATH        "/api/asr"
#define API_IMU_PATH        "/api/imu"
#define API_GPS_PATH        "/api/gps"
#define GPS_TX_PIN          44   // ESP32 TX -> GPS RX (NEO-M8N)
#define GPS_RX_PIN          45   // ESP32 RX -> GPS TX
#define GPS_BAUD            9600
#define GPS_SEND_INTERVAL_MS    5000   // 一般 5 秒
#define GPS_SEND_INTERVAL_PS_MS 15000  // 省電時 15 秒上傳一次
#define API_AUDIO_PATH      "/audio/latest"
#define API_TIMEOUT_MS      15000
#define AUDIO_POLL_INTERVAL_MS  500

// ============ 省電 ============
#define POWER_SAVE_ENABLE   1     // 1=啟用省電（單一按鈕時不開相機、WiFi modem sleep、閒置延遲）
#define WIFI_MODEM_SLEEP    1     // 1=WiFi 模組在閒置時進入 modem sleep（需 POWER_SAVE_ENABLE）
#define LOOP_IDLE_MS        50    // 無工作時 loop 延遲 ms（省電時拉長，預設 50）
#define LOOP_IDLE_MS_NORMAL 10    // 未開省電時延遲

#endif // CONFIG_H
