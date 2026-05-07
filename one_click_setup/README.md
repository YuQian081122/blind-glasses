# One-Click Environment Setup

This folder is portable. You can copy it to another Windows PC and run one file:

- `setup_env.bat`

## What the script does

1. Checks Git (and tries to install via `winget` if missing)
2. Checks Python 3 (and tries to install via `winget` if missing)
3. Clones or updates the repository
4. Creates a virtual environment: `.venv-setup`
5. Installs server dependencies from `server/requirements.txt`
6. Installs PlatformIO (inside the virtual environment)
7. Validates key Python packages
8. Runs a test firmware build: `platformio run` in `firmware`

## Default paths

- Script folder: current folder where `setup_env.bat` is located
- Workspace folder: `.\workspace`
- Project folder: `.\workspace\blind-glasses-firmware`
- Log file: `.\logs\setup.log`

## Optional configuration

Open `setup_env.bat` and edit:

- `REPO_URL`
- `REPO_BRANCH`
- `PROJECT_DIR_NAME`
- `NO_PAUSE` (`1` = no "Press any key")

## Firmware build note

- If `firmware/sdkconfig.defaults` is missing but `sdkconfig.defaults.template` exists,
  the script will auto-copy the template before PlatformIO build.

## Notes

- Keep script messages/logs in English to avoid encoding issues.
- First-time setup can take a while (Python packages + PlatformIO toolchains).
- If your PC has restrictive permissions, run the bat file as administrator.
- On failure, the script now prints the last 60 lines from the setup log.
