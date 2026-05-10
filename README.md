# AI 智慧導盲眼鏡 - ESP32 韌體

適用開發板：Seeed XIAO ESP32-S3 Sense (OV3660)  
目前 GitHub 倉庫（firmware-only）：<https://github.com/YuQian081122/blind-glasses-firmware>

## 專案範圍（2026 更新）

- 本倉庫目前以 **韌體** 為主，PlatformIO 專案集中在 **`firmware/`**（`include/`、`src/`、`platformio.ini`）
- 伺服器（FastAPI）不保證與本倉庫同步發佈
- 建議以本 README 與 `firmware/include/`、`firmware/src/`、`firmware/platformio.ini` 為主要維護基準

## 快速開始（Windows）

```powershell
cd firmware
pio run
```

如果 `pio` 指令不可用，改用：

```powershell
cd firmware
python -m platformio run
```

燒錄與監看：

```powershell
cd firmware
pio run -t upload
pio device monitor -b 115200
```

## 功能

- **UDP 探索**：開機後廣播 `WHO_IS_SERVER`，取得伺服器 IP
- **MJPEG 串流**：port 81 `/stream`，供伺服器即時擷取影像
- **按鈕控制**：開發中（文件暫不提供按鈕操作對照）
- **語音播放**：從伺服器 `GET /audio/latest` 取得 TTS 並以 MAX98357A I2S 播放（本版預設關閉）
- **麥克風**：PDM 錄音上傳 POST `/api/asr`（語音指令）
- **IMU**：ICM-20948 資料定期 POST 至 `/api/imu`
- **GPS**：NEO-M8N 經緯度定期 POST 至 `/api/gps`（本版預設關閉）

## 按鈕狀態

目前按鈕流程仍在開發與驗證中，暫不提供按鈕行為對照表。  
建議先以 API 與序列監看驗證整體流程（WiFi、UDP、IMU、ASR、TTS）。

## 硬體接線

以下 **D0–D12 ↔ GPIO** 與 Seeed [XIAO ESP32-S3 Getting Started](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) 主表一致；本專案實際使用欄標示 `config.h`。

| XIAO 絲印 | 晶片 GPIO | 本專案用途 |
|-----------|-----------|------------|
| D0 | 1 | I2S 資料 → MAX98357A DIN `I2S_DOUT_PIN` |
| D1 | 2 | I2S BCLK `I2S_BCLK_PIN` |
| D2 | 3 | I2S LRC/WS `I2S_LRC_PIN` |
| D4 / SDA | 5 | IMU SDA `IMU_SDA_PIN` |
| D5 / SCL | 6 | IMU SCL `IMU_SCL_PIN` |
| D8 | 7 | 電源鍵 `BTN_POWER_PIN` |
| D9 | 8 | 切換鍵 `BTN_MODE_PIN` |
| D6 / TX | 43 | GPS：ESP UART TX → 模組 RX `GPS_TX_PIN` |
| D7 / RX | 44 | GPS：ESP UART RX ← 模組 TX `GPS_RX_PIN` |
| MTDO 等 / Camera SCCB | 40 / 39 | 相機 I2C SDA/SCL（OV3660，見 Wiki Camera 表） |
| Camera DVP | 10–18, 38, 47, 48 等 | 內建鏡頭排線，見 `camera_stream.cpp` |
| 內建 PDM 麥克風 | 41 / 42 | DATA / CLK（`mic_upload.cpp`，與表列 Digital microphone 一致） |

若你外接線與上表不同，請只改 `firmware/include/config.h`（與必要時 `firmware/src/camera_stream.cpp`）。

### 目前韌體預設開關（對應現在腳位/接線）

- `IMU_I2C_ADDR = 0x68`（AD0 接 GND）
- `IMU_STANDALONE_TEST = 0`（一般連線模式）
- `GPIO1_TOGGLE_TEST_ENABLE = 1`（啟用 LED 方波測試）
- `GPIO1_TOGGLE_TEST_PIN = 10`（D10，每 2 秒切換高低電位）
- `GPS_ENABLE = 0`（GPS UART 不啟用）
- `AUDIO_I2S_ENABLE = 0`（MAX98357A 不啟用）
- `WIFI_MODEM_SLEEP = 0`（避免 IMU POST 因 sleep 逾時）

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

**行為**：省電模式下會降低背景輪詢與上傳頻率。按鈕相關邏輯尚在開發中，請以目前 `firmware/include/config.h` 與 `firmware/src/main.cpp` 實作為準。

## 材料清單與省電建議

| 品項 | 備註 |
|------|------|
| 薄型內磁喇叭 0288-23-1（1W 8Ω） | 與 MAX98357 搭配；不播時韌體不送 I2S，耗電低 |
| 電池 502248（500mAh） | 省電全開時可延長續航；關機用深眠，再按喚醒 |
| TP4056 鋰電池充電板 1A | 充電用；放電截止電壓勿設過高以保護電池 |
| NEO-M8N (GPS) | 省電時 15 秒上傳一次；有源陶瓷天線接天線腳可改善收訊 |
| ESP32-S3 Sense 開發板 | 相機／WiFi 為主要耗電；單一按鈕不預開相機、WiFi modem sleep |
| MAX98357 I2S | 僅播放時耗電；無硬體 shutdown 時不送訊號即省電 |
| ICM-20948（九軸 IMU；韌體目前僅上傳加速度+陀螺儀共 6 量） | 韌體已支援；I2C 位址 0x68（AD0 接 GND）或 0x69（AD0 接 VCC） |
| 2.4G 天線、有源陶瓷天線 | 天線收訊好可減少重試，間接省電 |

## IMU 單獨測試（連接診斷）

當你要確認「陀螺儀到底壞在哪一層」時，可啟用單獨測試模式，避免 WiFi/伺服器影響判斷。

### 啟用方式

1. 編輯 `firmware/include/config.h`
2. 設定：
   - `IMU_STANDALONE_TEST = 1`
   - （可選）`IMU_TEST_OUTPUT_INTERVAL_MS` 調整輸出頻率
3. 重新燒錄，開啟 Serial Monitor（115200）

啟用後，韌體只會啟動 IMU 診斷流程，略過 WiFi/Camera/GPS/Mic/Audio/UDP。

### 診斷輸出 CASE 對照

| 訊息 | 判讀 |
|------|------|
| `CASE_A` | I2C scan 無任何 ACK：先查 VCC/GND、SDA/SCL 是否接反、是否共地、是否有上拉 |
| `CASE_B` | 有 ACK 但非 0x68/0x69：可能接錯裝置、線路衝突或匯流排干擾 |
| `CASE_C` | 0x68/0x69 有 ACK 但 init 失敗：優先查 AD0 狀態、焊接品質、供電穩定性 |
| `CASE_D` | 初始化成功但 `getAGMT` 常失敗：可能是接觸不良、線太長或訊號品質差 |
| `CASE_E` | 連續 gyro 幾乎不變：請手動旋轉模組，若仍不變可能是感測器本體/讀值路徑異常 |

### 快速檢查流程

1. 看是否印出 `[IMU-TEST] Standalone mode begin`
2. 看 I2C scan 是否有 `0x68` 或 `0x69`
3. 看是否印出 `[IMU] Ready`
4. 手動旋轉 IMU，觀察 `gx/gy/gz` 是否連續變化

## 建置

1. 進入 `firmware` 目錄
2. 複製 `sdkconfig.defaults.template` 為 `sdkconfig.defaults`（與 `platformio.ini` 同層）
3. 修改 `include/config.h` 中的 `WIFI_SSID`、`WIFI_PASSWORD`
4. 在 VS Code 開啟專案，使用 PlatformIO 以 **`firmware` 為專案根目錄** 編譯並燒錄
5. 或命令列（在 `firmware` 內）：`pio run`、`pio run -t upload`

若你先前在 repo 根目錄建置過，根目錄可能留有舊的 `.pio`；可刪除以釋出空間，目前建置產物會在 `firmware/.pio`。

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
