@echo off
setlocal
title Restart iCloud for Windows

echo Restarting iCloud for Windows...

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference = 'Stop'; function Stop-ICloudProcesses { $items = Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -and (($_.ExecutablePath -like '*\WindowsApps\AppleInc.iCloud_*\iCloud\*') -or ($_.ExecutablePath -like '*\Apple Inc\iCloud\*')) }; foreach ($item in $items) { Stop-Process -Id $item.ProcessId -Force -ErrorAction SilentlyContinue } }; Stop-ICloudProcesses; Start-Sleep -Seconds 2; Stop-ICloudProcesses; Start-Sleep -Seconds 2; $package = Get-AppxPackage -Name AppleInc.iCloud; if (-not $package) { throw 'iCloud for Windows is not installed.' }; $appId = $package.PackageFamilyName + '!iCloud'; Start-Process -FilePath explorer.exe -ArgumentList ('shell:AppsFolder\' + $appId)"

if errorlevel 1 (
    echo.
    echo Failed to restart iCloud. Try right-clicking this file and choosing "Run as administrator".
    pause
    exit /b 1
)

echo iCloud has been restarted.
powershell.exe -NoLogo -NoProfile -Command "Start-Sleep -Seconds 2"
exit /b 0
