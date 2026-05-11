# 智慧導盲眼鏡（本機整包）

本目錄可同時放 **韌體**（`firmware/`）與 **伺服器**（`server/`）等子專案。

## 韌體（PlatformIO）

- 原始碼與 `platformio.ini` 在 **`firmware/`** 內。
- 編譯／燒錄：在 `firmware` 目錄執行 `pio run`、 `pio run -t upload`（或 `python -m platformio …`）。
- 詳見 [`firmware/README.md`](firmware/README.md)。

## 推送到 GitHub 之「獨立韌體倉庫」（根目錄即 PIO，不要多一層 `firmware/`）

遠端 [blind-glasses-firmware](https://github.com/YuQian081122/blind-glasses-firmware) 應為 **clone 根目錄即 `include/`、`src/`**，請勿直接把本機的 `firmware/` 資料夾當成一層推上去。

請在本目錄執行：**`push_firmware_flat_to_github.bat`**（內含 `git subtree split --prefix=firmware`），會把本機 `firmware/` 底下的內容推成遠端分支上的**根目錄**結構。

（若 split 與舊歷史不相容，腳本註解內有 `--force-with-lease` 的說明；請自行確認無誤後再使用。）
