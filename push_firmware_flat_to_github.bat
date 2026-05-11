@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

echo.
echo ============================================================
echo 將本機「firmware\」內容以「根目錄」推到遠端 blind-glasses-firmware
echo （使用 git subtree split，遠端不會多出 firmware\ 子目錄）
echo ============================================================
echo.
echo 請先確認：已 git add / commit 本機變更；建議工作區乾淨再執行。
pause

git rev-parse --git-dir >nul 2>&1
if errorlevel 1 (
  echo 錯誤：不在 git 倉庫根目錄。
  exit /b 1
)

if not exist "firmware\platformio.ini" (
  echo 錯誤：找不到 firmware\platformio.ini
  exit /b 1
)

git fetch origin 2>nul

set "TMPB=__fw_flat_export_%RANDOM%"
echo.
echo --- git subtree split --prefix=firmware ---
git subtree split --prefix=firmware -b "%TMPB%"
if errorlevel 1 (
  echo subtree split 失敗。請確認已提交含 firmware\ 的歷史。
  exit /b 1
)

echo.
echo --- git push origin %TMPB%:main ---
echo 若與遠端 main 無法快轉，請改用手動（並自行承擔風險）：
echo   git push origin %TMPB%:main --force-with-lease
echo.
git push origin "%TMPB%":main
set PUSH_RC=%ERRORLEVEL%

git branch -D "%TMPB%" 2>nul

if not "%PUSH_RC%"=="0" (
  echo.
  echo 推送未成功 ^(錯誤碼 %PUSH_RC%^)。請依上列訊息處理後再試。
  exit /b 1
)

echo.
echo 完成：遠端 main 應為扁平結構 ^(include\、src\ 在根目錄^)。
pause
endlocal
exit /b 0
