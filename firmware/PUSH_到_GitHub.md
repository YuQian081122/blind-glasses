# 只推送韌體到 GitHub（不包含 server）

目標：https://github.com/YuQian081122/blind-glasses

## 方式一：用 Git（建議）

1. 安裝 Git for Windows：https://git-scm.com/download/win（安裝時勾選「Add Git to PATH」）
2. 在此資料夾 `firmware` 內雙擊 **`push-to-github.bat`**
3. 依提示登入 GitHub（瀏覽器或憑證），完成後僅韌體會出現在 repo

## 方式二：不用 Git，手動上傳

1. 在瀏覽器打開 https://github.com/YuQian081122/blind-glasses
2. 若 repo 是空的：點「uploading an existing file」，把本機 `firmware` 裡**所有檔案與資料夾**（`include/`、`src/`、`platformio.ini`、`README.md`、`.gitignore`、`sdkconfig.defaults.template` 等）拖曳進去（不要選到上一層的 server）
3. 填寫 commit 訊息後按 Commit changes

注意：**不要**上傳 `server` 目錄或 `.env`，只上傳本 `firmware` 目錄內容。
