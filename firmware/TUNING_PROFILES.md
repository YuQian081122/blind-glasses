# 韌體調參配置說明

## 兩組預設配置

### 低延遲模式（預設，適合 WiFi 穩定環境）

```c
#define MIC_RECORD_SEC         2
#define API_TIMEOUT_MS         5000
#define AUDIO_POLL_INTERVAL_MS 500
```

- 端到端延遲目標：Gemini 路徑 P50 < 1.5s
- 適用場景：室內、WiFi 訊號良好、短指令

### 高穩定模式（適合嘈雜/弱網環境）

```c
#define MIC_RECORD_SEC         3
#define API_TIMEOUT_MS         8000
#define AUDIO_POLL_INTERVAL_MS 500
```

- 容許稍高延遲換取更好的 ASR 辨識率
- 適用場景：路口、馬路邊、公車站

## 切換方式

修改與 `platformio.ini` 同層之 `include/config.h` 內對應的 `#define`，重新編譯上傳。

## 驗證方法

1. 啟動 server，執行 `python benchmark_latency.py --runs 30`
2. 觀察 Serial Monitor 的 `[LAT]` 輸出
3. 對比 P50/P95 是否符合目標
4. ASR 測試：在目標環境中說 10 次短指令，確認辨識成功率 >= 80%

## 參數影響速查

| 參數 | 降低 → | 升高 → |
|------|--------|--------|
| MIC_RECORD_SEC | 延遲降低、短語可辨識 | 長句辨識率提升、噪音容忍 |
| API_TIMEOUT_MS | 快速失敗重試 | 弱網不易超時 |
| TTS_GRACE_MS (main.cpp) | 更快拿到音訊 | 確保 TTS 生成完畢 |
