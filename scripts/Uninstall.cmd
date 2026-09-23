@echo off
setlocal

set "TASK_NAME=iCloudTrayRestart"
set "STARTUP_LINK=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\iCloudTrayRestart.lnk"

schtasks /Delete /TN "%TASK_NAME%" /F >nul 2>&1
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "%TASK_NAME%" /f >nul 2>&1
del /q "%STARTUP_LINK%" >nul 2>&1
taskkill /IM ICloudTrayBridge.exe /F >nul 2>&1

echo iCloud tray restart integration uninstalled.
echo Project files were kept in place.
exit /b 0
