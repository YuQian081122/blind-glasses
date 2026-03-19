# AI 智慧導盲眼鏡 - ESP32 韌體

適用開發板：Seeed XIAO ESP32-S3 Sense (OV3660)

## 功能

- **UDP 探索**：開機後廣播 `WHO_IS_SERVER`，取得伺服器 IP
- **MJPEG 串流**：port 81 `/stream`，供伺服器即時擷取影像
- **雙按鈕**：電源鍵 + 切換鍵，可切換「單一按鈕」與「持續監測」模式
- **語音播放**：從伺服器 `GET /audio/latest` 取得 TTS 並以 MAX98357A I2S 播放
- **麥克風**：PDM 錄音上傳 POST `/api/asr`（語音指令）
- **IMU**：ICM-20948 資料定期 POST 至 `/api/imu`
- **GPS**：NEO-M8N 經緯度定期 POST 至 `/api/gps`

## 按鈕操作

| 按鈕 | 動作 | 功能 |
|------|------|------|
| 電源鍵 | 短按 | 風景（general） |
| 電源鍵 | 雙擊 | 物品查找（item_search） |
| 電源鍵 | 三擊 | 紅綠燈（light） |
| 電源鍵 | 長按 5 秒 | 開/關機（進入深眠，再按喚醒） |
| 切換鍵 | 短按 | 切換 單一按鈕 ↔ 持續監測 模式 |
| 切換鍵 | 長按 5 秒 | 呼叫 AI 語音助理 |

**單一按鈕模式**：切換鍵長按 5 秒呼叫語音助理，TTS 播完後會自動切換為持續監測。  
**持續模式**：電源鍵不觸發語音，僅切換鍵長按 5 秒觸發語音助理。

## 硬體接線

| 模組 | 腳位 (config.h) |
|------|-----------------|
| 電源鍵 | D1 (GPIO 35) |
| 切換鍵 | D2 (GPIO 37) |
| MAX98357A BCLK | D3 (GPIO 36) |
| MAX98357A LRC | D5 (GPIO 34) |
| MAX98357A DIN | D7 (GPIO 43) |
| IMU SDA/SCL | GPIO 7 / 8 (I2C) |
| GPS TX/RX | GPIO 44 / 45 (UART) |

請依實際接線修改 `include/config.h`。

## 省電

`config.h` 內建省電選項（預設啟用，適合 500mAh 等小電池）：

| 選項 | 說明 |
|------|------|
| `POWER_SAVE_ENABLE` | 1=啟用：相機延後開、WiFi modem sleep、UDP/IMU/GPS 放慢 |
| `WIFI_MODEM_SLEEP` | 1=WiFi 閒置時 modem sleep |
| `LOOP_IDLE_MS` | 無播放時 loop 延遲 ms（預設 50） |
| `UDP_DISCOVERY_INTERVAL_SLOW_MS` | 尚未找到伺服器時廣播間隔（預設 8 秒） |
| `IMU_SEND_INTERVAL_PS_MS` | 省電時 IMU 上傳間隔（預設 200ms） |
| `GPS_SEND_INTERVAL_PS_MS` | 省電時 GPS 上傳間隔（預設 15 秒） |

**行為**：單一按鈕開機不啟動相機；第一次按風景／物品／紅綠燈或切換到持續監測時才啟動。關機為深眠（電源鍵長按 5 秒），再按電源鍵喚醒。

## 材料清單與省電建議

| 品項 | 備註 |
|------|------|
| 薄型內磁喇叭 0288-23-1（1W 8Ω） | 與 MAX98357 搭配；不播時韌體不送 I2S，耗電低 |
| 電池 502248（500mAh） | 省電全開時可延長續航；關機用深眠，再按喚醒 |
| TP4056 鋰電池充電板 1A | 充電用；放電截止電壓勿設過高以保護電池 |
| NEO-M8N (GPS) | 省電時 15 秒上傳一次；有源陶瓷天線接天線腳可改善收訊 |
| ESP32-S3 Sense 開發板 | 相機／WiFi 為主要耗電；單一按鈕不預開相機、WiFi modem sleep |
| MAX98357 I2S | 僅播放時耗電；無硬體 shutdown 時不送訊號即省電 |
| ICM-20948（陀螺儀） | 韌體已支援；I2C 位址 0x68（AD0 接 GND）或 0x69（AD0 接 VCC） |
| 2.4G 天線、有源陶瓷天線 | 天線收訊好可減少重試，間接省電 |

## 建置

1. 複製 `sdkconfig.defaults.template` 為 `sdkconfig.defaults`
2. 修改 `include/config.h` 中的 `WIFI_SSID`、`WIFI_PASSWORD`
3. 在 VS Code 開啟專案，使用 PlatformIO 編譯並燒錄
4. 或命令列：`pio run`、`pio run -t upload`

## 與伺服器 API

| 動作 | ESP32 | 伺服器 |
|------|-------|--------|
| 探索 | 廣播 WHO_IS_SERVER | 回覆 SERVER_IP: x.x.x.x |
| 串流 | HTTP 81 /stream MJPEG | cv2.VideoCapture 拉取 |
| 風景/紅綠燈/物品查找 | POST /api/gemini?mode=general\|light\|item_search | 執行 Gemini，產生 TTS |
| 語音指令 | POST /api/asr (WAV) | Whisper ASR，回寫 TTS |
| IMU | POST /api/imu (JSON) | 姿態/導航輔助 |
| GPS | POST /api/gps (JSON) | 經緯度、海拔、衛星數 |
| 語音檔 | GET /audio/latest | 回傳最新 TTS 檔 |
