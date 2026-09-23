@echo off
setlocal

for %%I in ("%~dp0..") do set "PROJECT_DIR=%%~fI"
set "BRIDGE=%PROJECT_DIR%\bin\ICloudTrayBridge.exe"
set "DLL=%PROJECT_DIR%\bin\ICloudTrayMenu.dll"
set "TASK_NAME=iCloudTrayRestart"
set "STARTUP_LINK=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\iCloudTrayRestart.lnk"

if not exist "%BRIDGE%" (
    echo Missing: %BRIDGE%
    exit /b 1
)
if not exist "%DLL%" (
    echo Missing: %DLL%
    exit /b 1
)

rem Remove legacy startup registrations from earlier builds.
schtasks /Delete /TN "%TASK_NAME%" /F >nul 2>&1
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" ^
  /v "%TASK_NAME%" /f >nul 2>&1

powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
  "$shortcut = (New-Object -ComObject WScript.Shell).CreateShortcut($env:STARTUP_LINK); $shortcut.TargetPath = $env:BRIDGE; $shortcut.WorkingDirectory = Join-Path $env:PROJECT_DIR 'bin'; $shortcut.Description = 'iCloud tray restart bridge'; $shortcut.Save()"
if errorlevel 1 (
    echo Unable to create the current-user Startup shortcut.
    exit /b 1
)

echo Installed with the current-user Startup folder.

start "" "%BRIDGE%"
echo iCloud tray restart integration installed.
exit /b 0
