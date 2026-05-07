@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul
title Blind Glasses One-Click Setup

REM =========================
REM Configuration (edit if needed)
REM =========================
set "REPO_URL=https://github.com/YuQian081122/blind-glasses-firmware.git"
set "REPO_BRANCH=main"
set "PROJECT_DIR_NAME=blind-glasses-firmware"
set "NO_PAUSE=0"

REM =========================
REM Runtime paths
REM =========================
set "SCRIPT_DIR=%~dp0"
set "WORKSPACE_DIR=%SCRIPT_DIR%workspace"
set "PROJECT_DIR=%WORKSPACE_DIR%\%PROJECT_DIR_NAME%"
set "LOG_DIR=%SCRIPT_DIR%logs"
set "LOG_FILE=%LOG_DIR%\setup.log"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>&1
if not exist "%WORKSPACE_DIR%" mkdir "%WORKSPACE_DIR%" >nul 2>&1

call :log "==============================================="
call :log "Blind Glasses one-click setup started"
call :log "Script dir: %SCRIPT_DIR%"
call :log "Workspace : %WORKSPACE_DIR%"
call :log "Project   : %PROJECT_DIR%"
call :log "Repo URL  : %REPO_URL%"
call :log "Branch    : %REPO_BRANCH%"
call :log "==============================================="

REM -------------------------
REM Step 1: Ensure Git
REM -------------------------
call :log "[1/8] Checking Git..."
where git >nul 2>&1
if errorlevel 1 (
    call :log "Git not found. Attempting install via winget..."
    where winget >nul 2>&1
    if errorlevel 1 (
        call :fail "Git is missing and winget is unavailable. Install Git manually: https://git-scm.com/download/win"
    )
    winget install --id Git.Git -e --source winget --accept-source-agreements --accept-package-agreements >> "%LOG_FILE%" 2>&1
    where git >nul 2>&1 || call :fail "Git installation failed. Install Git manually and rerun."
)
for /f "delims=" %%i in ('git --version') do set "GIT_VER=%%i"
call :log "Git OK: %GIT_VER%"

REM -------------------------
REM Step 2: Ensure Python
REM -------------------------
call :log "[2/8] Checking Python..."
set "PY_CMD="
where py >nul 2>&1 && set "PY_CMD=py -3"
if not defined PY_CMD (
    where python >nul 2>&1 && set "PY_CMD=python"
)
if not defined PY_CMD (
    call :log "Python not found. Attempting install via winget..."
    where winget >nul 2>&1
    if errorlevel 1 (
        call :fail "Python is missing and winget is unavailable. Install Python 3.10+ manually."
    )
    winget install --id Python.Python.3.11 -e --source winget --accept-source-agreements --accept-package-agreements >> "%LOG_FILE%" 2>&1
    where py >nul 2>&1 && set "PY_CMD=py -3"
    if not defined PY_CMD (
        where python >nul 2>&1 && set "PY_CMD=python"
    )
    if not defined PY_CMD call :fail "Python installation failed. Install Python manually and rerun."
)
for /f "delims=" %%i in ('%PY_CMD% --version 2^>^&1') do set "PY_VER=%%i"
call :log "Python OK: %PY_VER%"

REM -------------------------
REM Step 3: Clone or update repo
REM -------------------------
call :log "[3/8] Cloning/updating repository..."
if not exist "%PROJECT_DIR%\.git" (
    call :log "Cloning %REPO_URL% ..."
    git clone --branch "%REPO_BRANCH%" --single-branch "%REPO_URL%" "%PROJECT_DIR%" >> "%LOG_FILE%" 2>&1 || call :fail "Git clone failed."
) else (
    call :log "Repository exists. Pulling latest..."
    pushd "%PROJECT_DIR%"
    git fetch --all >> "%LOG_FILE%" 2>&1
    git checkout "%REPO_BRANCH%" >> "%LOG_FILE%" 2>&1
    git pull --ff-only >> "%LOG_FILE%" 2>&1 || call :log "Warning: git pull failed, keeping local copy."
    popd
)

if not exist "%PROJECT_DIR%\firmware\platformio.ini" call :fail "firmware/platformio.ini not found after clone."
set "SERVER_REQ=%PROJECT_DIR%\server\requirements.txt"
set "HAS_SERVER_REQ=0"
if exist "%SERVER_REQ%" set "HAS_SERVER_REQ=1"
if "%HAS_SERVER_REQ%"=="1" (
    call :log "Detected server requirements: %SERVER_REQ%"
) else (
    call :log "No server/requirements.txt found. Server dependency install will be skipped."
    call :log "If you need server setup, point REPO_URL to a repo/branch that contains /server."
)

REM -------------------------
REM Step 4: Create virtual environment
REM -------------------------
call :log "[4/8] Creating Python virtual environment..."
set "VENV_DIR=%PROJECT_DIR%\.venv-setup"
if not exist "%VENV_DIR%\Scripts\python.exe" (
    %PY_CMD% -m venv "%VENV_DIR%" >> "%LOG_FILE%" 2>&1 || call :fail "Failed to create virtual environment."
)
set "VENV_PY=%VENV_DIR%\Scripts\python.exe"
if not exist "%VENV_PY%" call :fail "Virtual environment python not found."

REM -------------------------
REM Step 5: Install Python dependencies
REM -------------------------
call :log "[5/8] Installing server dependencies..."
"%VENV_PY%" -m pip install --upgrade pip setuptools wheel >> "%LOG_FILE%" 2>&1 || call :fail "Failed to upgrade pip tools."
if "%HAS_SERVER_REQ%"=="1" (
    "%VENV_PY%" -m pip install -r "%SERVER_REQ%" >> "%LOG_FILE%" 2>&1 || call :fail "Failed to install server requirements."
) else (
    call :log "Skipped: server dependencies (requirements file not found)."
)

REM -------------------------
REM Step 6: Install PlatformIO
REM -------------------------
call :log "[6/8] Installing/checking PlatformIO..."
"%VENV_PY%" -m pip install --upgrade platformio >> "%LOG_FILE%" 2>&1 || call :fail "Failed to install PlatformIO."
"%VENV_PY%" -m platformio --version >> "%LOG_FILE%" 2>&1 || call :fail "PlatformIO check failed."

REM -------------------------
REM Step 7: Validate environment
REM -------------------------
call :log "[7/8] Running package checks..."
if "%HAS_SERVER_REQ%"=="1" (
    "%VENV_PY%" -c "import fastapi,uvicorn,requests; print('Python package check OK')" >> "%LOG_FILE%" 2>&1 || call :fail "Python package import check failed."
) else (
    "%VENV_PY%" -c "import platformio; print('PlatformIO package check OK')" >> "%LOG_FILE%" 2>&1 || call :fail "PlatformIO package import check failed."
)

REM -------------------------
REM Step 8: Test firmware build
REM -------------------------
call :log "[8/8] Testing PlatformIO firmware build..."
if not exist "%PROJECT_DIR%\firmware\sdkconfig.defaults" (
    if exist "%PROJECT_DIR%\firmware\sdkconfig.defaults.template" (
        call :log "sdkconfig.defaults missing. Copying from sdkconfig.defaults.template..."
        copy /Y "%PROJECT_DIR%\firmware\sdkconfig.defaults.template" "%PROJECT_DIR%\firmware\sdkconfig.defaults" >> "%LOG_FILE%" 2>&1 || call :fail "Failed to create sdkconfig.defaults from template."
    )
)
pushd "%PROJECT_DIR%\firmware"
"%VENV_PY%" -m platformio run >> "%LOG_FILE%" 2>&1 || call :fail "PlatformIO build failed."
popd

call :log "==============================================="
call :log "SETUP COMPLETED SUCCESSFULLY"
call :log "Project folder : %PROJECT_DIR%"
call :log "Venv python    : %VENV_PY%"
call :log "Full log       : %LOG_FILE%"
call :log "==============================================="
echo.
echo Setup completed successfully.
echo Project: %PROJECT_DIR%
echo Log    : %LOG_FILE%
echo.
if not "%NO_PAUSE%"=="1" pause
exit /b 0

:log
echo %~1
echo [%date% %time%] %~1>> "%LOG_FILE%"
exit /b 0

:fail
echo.
echo [ERROR] %~1
echo [ERROR] %~1>> "%LOG_FILE%"
echo See log: %LOG_FILE%
echo Last 60 log lines:
powershell -NoProfile -Command "if (Test-Path '%LOG_FILE%') { Get-Content -Path '%LOG_FILE%' -Tail 60 }"
echo.
if not "%NO_PAUSE%"=="1" pause
exit /b 1
