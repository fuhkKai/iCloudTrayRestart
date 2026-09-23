# iCloud Tray Restart

English · [简体中文](README.zh-Hans.md) · [繁體中文](README.zh-Hant.md) · [日本語](README.ja.md)

Adds **Restart iCloud** to the system tray menu of the Microsoft Store version of iCloud for Windows. It can help when syncing stalls. The label follows the Windows preferred language order and supports English, Simplified Chinese, Traditional Chinese, and Japanese.

## Install

1. Download the Windows x64 ZIP from the [latest release](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest) and extract it to a permanent folder.
2. Run `scripts\Install.cmd`. The app starts when the current user signs in; administrator rights are not required.
3. Choose the restart item in the iCloud tray menu. To remove it, run `scripts\Uninstall.cmd`.

Before updating, close the iCloud main window and run the uninstall script. Replace the files, then install again and start iCloud.

## Compatibility and build

Tested on Windows 11 x64 with the Microsoft Store version of iCloud for Windows `15.10.39.0`. Windows 10 and other iCloud versions have not been tested. Only the x64 Store version is supported.

To build from source, use Visual Studio 2022 with the x64 C++ tools and CMake:

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

Alternatively, run `scripts\BuildWithZig.cmd` with Zig 0.16. Output goes to `bin`.

## Notes

The app loads a menu DLL into `iCloudHome.exe`; it does not modify Apple's installed files. If the tray menu is unavailable, run `tools\RestartICloud.cmd`. Logs are in `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log`.

Project code is [MIT licensed](LICENSE). Bundled MinHook has a [separate license](third_party/minhook/LICENSE.txt).
