# AI 智慧導盲眼鏡 - ESP32 韌體

適用開發板：Seeed XIAO ESP32-S3 Sense (OV3660)  
GitHub（firmware-only）：<https://github.com/YuQian081122/blind-glasses-firmware>

## 專案根目錄

本專案的 PlatformIO 根目錄是 `firmware/`。  
請先進入 `firmware` 再執行所有 `pio` 指令。

## 5 分鐘快速開始（Windows）

```powershell
cd firmware
pio run
```

若你的環境尚未有 `pio` 指令：

```powershell
cd firmware
python -m platformio run
```

## 常用指令

```powershell
cd firmware
pio run
pio run -t upload
pio device monitor -b 115200
```

## 第一次建置前

1. `cd firmware`
2. 複製 `sdkconfig.defaults.template` 為 `sdkconfig.defaults`
3. 修改 `include/config.h` 的 WiFi 等參數
4. 執行 `pio run`

