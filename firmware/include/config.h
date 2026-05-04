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

// ============ 相機 OV3660 SCCB（I2C，與板載腳位表一致）============
#define CAMERA_SIOD_PIN     40   // Camera I2C SDA
#define CAMERA_SIOC_PIN     39   // Camera I2C SCL

// ============ 操作模式 ============
// 0 = 單一按鈕模式 (短/長/雙擊/三擊區分功能)
// 1 = 持續監測模式 (按鈕僅語音觸發，避障由伺服器持續處理)
#define OP_MODE_SINGLE_BTN  0
#define OP_MODE_ALWAYS_ON   1
// NVS 尚無紀錄時的開機預設（曾用切換鍵切過則以 NVS 為準；若要強制改回持續監測可清除快閃或短按切換鍵切一次）
#define OP_MODE_DEFAULT     OP_MODE_ALWAYS_ON

// ============ 按鈕 - 電源鍵與切換鍵（D1/D2 改供 I2S 後，按鈕改接 D8/D9）============
#define BTN_POWER_PIN       7    // 電源鍵 → 板載 D8（GPIO7）
#define BTN_MODE_PIN        8    // 切換鍵 → 板載 D9（GPIO8）
#define BTN_SINGLE_PIN      BTN_POWER_PIN
#define BTN_DEBOUNCE_MS     50
#define BTN_POWER_HOLD_MS   5000  // 電源鍵長按 5 秒 = 開/關機；切換鍵長按 5 秒 = 語音助理

// ============ I2S (MAX98357A) ============
// Pin-B 測試版：先不改實體接線，僅在韌體內交換 BCLK/LRC 做相容性排查
#define I2S_DOUT_PIN    1    // D0（GPIO1）→ MAX98357A DIN
#define I2S_BCLK_PIN    3    // D2（GPIO3）→ MAX98357A BCLK (test-B swap)
#define I2S_LRC_PIN     2    // D1（GPIO2）→ MAX98357A LRC/WS (test-B swap)

// ============ GPIO LED 測試 ============
// 1=在 loop 中每 2 秒切換測試腳位高低電位，方便接 LED 觀察輸出
#define GPIO1_TOGGLE_TEST_ENABLE      0
#define GPIO1_TOGGLE_TEST_PIN         9    // D10 (GPIO9)
#define GPIO1_TOGGLE_INTERVAL_MS      2000

// ============ IMU (ICM-20948) ============
// 與 Seeed XIAO ESP32-S3 腳位圖一致：D4=SDA→GPIO5、D5=SCL→GPIO6（見 Wiki Getting Started）
#define IMU_SDA_PIN        5    // I2C SDA（板載 D4）
#define IMU_SCL_PIN        6    // I2C SCL（板載 D5）
#define IMU_I2C_ADDR       0x68 // AD0=LOW；若模組 AD0 接 VCC 則為 0x69，begin 時 ad0val=true
#define IMU_SEND_INTERVAL_MS    100   // 一般
#define IMU_SEND_INTERVAL_PS_MS 200   // 省電時拉長（POWER_SAVE_ENABLE）

// ============ IMU 單獨測試（診斷用） ============
// 1=開啟「只測 IMU」模式：不啟用 WiFi/伺服器 API，只做 I2C 掃描與連續 gyro/acc 讀值輸出
// 0=維持原本功能
#define IMU_STANDALONE_TEST    0
#define IMU_TEST_OUTPUT_INTERVAL_MS        100   // standalone 每次輸出間隔
#define IMU_TEST_FAIL_THROTTLE_MS          2000  // 讀取失敗/初始化失敗的節流輸出

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
// 與板載 D6/D7 一致：TX=GPIO43、RX=GPIO44（HardwareSerial: RX, TX 順序見 gps_stream.cpp）
#define GPS_TX_PIN          43   // D6 → ESP UART TX → GPS RX
#define GPS_RX_PIN          44   // D7 → ESP UART RX ← GPS TX
#define GPS_BAUD            9600
#define GPS_SEND_INTERVAL_MS    5000   // 一般 5 秒
#define GPS_SEND_INTERVAL_PS_MS 15000  // 省電時 15 秒上傳一次

// 僅使用相機+陀螺儀時：可關閉 GPS UART；音訊測試時請開啟 I2S 喇叭
#define GPS_ENABLE          0   // 1=啟用 NEO-M8N 串流；0=不初始化 GPS UART
#define AUDIO_I2S_ENABLE    1   // 1=MAX98357A 播放；0=不佔用 I2S1（PDM 麥克風仍為 I2S0）

// ============ 音訊自動測試（不需按鈕） ============
// 1=每隔固定秒數自動抓一次 /audio/latest 測 MAX98357 播放鏈路
#define AUDIO_AUTO_TEST_ENABLE       1
#define AUDIO_AUTO_TEST_INTERVAL_MS  10000

#define API_AUDIO_PATH      "/audio/latest"
#define API_TIMEOUT_MS      15000
#define AUDIO_POLL_INTERVAL_MS  500

// ============ BLE 快速連接（S3 可用） ============
#define BLE_QUICK_LINK_ENABLE         1
#define BLE_DEVICE_NAME               "BlindGlasses-S3"
#define BLE_SERVICE_UUID              "6f2f6d30-4d57-4c76-a5dd-86f4d2a06340"
#define BLE_WIFI_SSID_UUID            "6f2f6d31-4d57-4c76-a5dd-86f4d2a06340"
#define BLE_WIFI_PASS_UUID            "6f2f6d32-4d57-4c76-a5dd-86f4d2a06340"
#define BLE_WIFI_APPLY_UUID           "6f2f6d33-4d57-4c76-a5dd-86f4d2a06340"
#define BLE_FIND_ME_UUID              "6f2f6d34-4d57-4c76-a5dd-86f4d2a06340"
#define BLE_STATUS_UUID               "6f2f6d35-4d57-4c76-a5dd-86f4d2a06340"
#define BLE_MODE_UUID                 "6f2f6d36-4d57-4c76-a5dd-86f4d2a06340"
#define BLE_STATUS_NOTIFY_MS          1000

// ============ 相機啟動 ============
// 1=開機且 WiFi 連上後立即啟動串流（即使省電 + 單一按鈕模式）；0=維持省電延後開相機（按鍵或切換持續監測後才開）
#define CAMERA_START_ON_BOOT  1

// ============ 省電 ============
#define POWER_SAVE_ENABLE   1     // 1=啟用省電（單一按鈕時不開相機、WiFi modem sleep、閒置延遲）
// 若監控頁陀螺儀永遠為「-」，常因 modem sleep 導致 POST /api/imu 失敗；先設 0 再試
#define WIFI_MODEM_SLEEP    0     // 1=WiFi 閒置 modem sleep（易影響 IMU HTTP 上傳）
#define LOOP_IDLE_MS        50    // 無工作時 loop 延遲 ms（省電時拉長，預設 50）
#define LOOP_IDLE_MS_NORMAL 10    // 未開省電時延遲

#endif // CONFIG_H
