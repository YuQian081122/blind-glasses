@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo 只推送「韌體」到 GitHub，不會包含 server 或專案其他部分。
echo.

where git >nul 2>&1
if errorlevel 1 (
    echo [錯誤] 找不到 git。請先安裝 Git for Windows 並確認已加入 PATH。
    echo 下載：https://git-scm.com/download/win
    pause
    exit /b 1
)

if not exist .git (
    git init
    git add .
    git commit -m "Initial commit: AI 智慧導盲眼鏡韌體 (XIAO ESP32-S3 Sense)"
)

git remote remove origin 2>nul
git remote add origin https://github.com/YuQian081122/blind-glasses
git branch -M main
echo.
echo 正在推送到 GitHub（請依提示登入或使用憑證）...
git push -u origin main
if errorlevel 1 (
    echo.
    echo 若 repo 名稱或帳號不同，請編輯此檔，把 YuQian081122/blind-glasses 改成你的 帳號/repo名稱
    pause
    exit /b 1
)
echo.
echo 完成：僅韌體已推送到 GitHub。
pause
