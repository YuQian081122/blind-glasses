# 倉庫目錄結構說明（請勿隨意改動韌體路徑）

最後更新：依本機整包專案與 GitHub **blind-glasses-firmware** 對齊方式整理。

## 1. Git／GitHub 上的韌體（標準版面）

**韌體檔案一律在倉庫根目錄**（與 `platformio.ini` 同層），**不要**再包一層 `firmware/include` 進版本庫。

```
（blind-glasses-firmware 倉庫根）
├── .github/
├── include/           ← 標頭檔
├── src/               ← 原始碼
├── platformio.ini
├── sdkconfig.defaults.template
├── TUNING_PROFILES.md
└── README.md
```

- 編譯：在**上述根目錄**執行 `pio run`。
- 其他電腦：`git clone …/blind-glasses-firmware.git` 後，**進入 clone 根目錄**即可編譯；或使用專案內 `clone_build_upload_firmware.bat`（會自動找根目錄的 `platformio.ini`）。

## 2. 本機「整包」專案（韌體 + 伺服器同資料夾）

若你把 **同一個 git 倉庫** clone 或放在例如：

```
blind_glasses/
├── .git                 # 若遠端為 blind-glasses-firmware，則此層即韌體根
├── include/
├── src/
├── platformio.ini
├── server/              # 通常未納入韌體遠端；僅本機存在
└── …
```

此時 **韌體仍在根目錄**，與第 1 節一致；**沒有** `firmware/` 子資料夾包一層韌體的設計。

## 3. 若你希望本機「視覺上」永遠叫 `firmware/`（與 server 並列）

Git **無法**在同一個工作區同時「遠端根目錄扁平」又「強制多一層 `firmware/` 目錄名」而不重複檔案。建議作法：

**推薦**：整包專案用**兩個目錄、兩個 git（或一個無 git 的 server 資料夾）**：

```
MyProject/
├── server/                    # FastAPI，可獨立 clone 或複製
└── firmware/                  # 單獨：git clone …/blind-glasses-firmware.git
    ├── .git
    ├── include/
    ├── src/
    └── platformio.ini         # 此層即 PlatformIO 根
```

這樣本機路徑永遠是 `…/firmware/include/...`，但 **git 根目錄就是 `firmware/` 這一層**，與 GitHub 扁平結構一致，不會跑掉。

## 4. 不要做的事（容易之後「跑掉」）

| 不要 | 原因 |
|------|------|
| 把 `include/`、`src/` 移進 `firmware/` 再推上 **blind-glasses-firmware** | 遠端會變成多一層子目錄，與 CI、`clone_build_upload_firmware.bat` 預設不符。 |
| 在父目錄用 `firmware/` 當子目錄卻又讓 **同一個** remote 只追蹤子路徑 | 需 subtree／雙遠端，易忘步驟。 |
| 改 `platformio.ini` 的 `src_dir`／`include_dir` 指到奇怪相對路徑 | 他人 clone 後易編譯失敗。 |

## 5. 與本文件一起維護

若之後調整目錄約定，請同步更新：

- 本檔 `docs/REPOSITORY_LAYOUT.md`
- 根目錄 `README.md`（韌體說明）
- `.github/workflows/firmware-build.yml`（路徑觸發與工作目錄）
