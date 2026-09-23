<div align="center">

# iCloud Tray Restart

**Restart iCloud. Right from the tray.**

**English** · [简体中文](README.zh-Hans.md) · [繁體中文](README.zh-Hant.md) · [日本語](README.ja.md)

[**↓ Download for Windows x64**](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest) · [Release notes](https://github.com/fuhkKai/iCloudTrayRestart/releases/tag/v0.1.0) · [Report an issue](https://github.com/fuhkKai/iCloudTrayRestart/issues)

</div>

---

Adds **Restart iCloud** to the existing tray menu of **iCloud for Windows (Microsoft Store)**. A quick way to restart iCloud when syncing stalls.

## Get started

1. Download the **Windows x64 ZIP** from the latest release and extract it to a folder you plan to keep. Choose the app ZIP, not “Source code”.
2. Double-click `scripts\Install.cmd`. No administrator rights are needed; the helper starts now and whenever you sign in.
3. Open iCloud, right-click its system tray icon, and choose **Restart iCloud**.

The menu follows your Windows preferred language order: English, 简体中文, 繁體中文, 日本語.

> Tested with Windows 11 x64 and Microsoft Store iCloud **15.10.39.0**. Windows 10 and other iCloud versions are untested; only the x64 Store app is supported.

## More information

<details>
<summary>Update or uninstall</summary>

**Update:** Exit iCloud, run `scripts\Uninstall.cmd`, replace the extracted files, then run `scripts\Install.cmd` and open iCloud again.

**Uninstall:** Run `scripts\Uninstall.cmd`, then exit and reopen iCloud to remove the loaded menu extension. The script keeps the project folder; you can delete it afterward.

</details>

<details>
<summary>Missing menu or troubleshooting</summary>

Make sure iCloud is running and that you kept the extracted folder in place. If the menu is unavailable, use `tools\RestartICloud.cmd`.

For an issue report, include your Windows and iCloud versions and relevant lines from `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log`.

</details>

<details>
<summary>Build from source · implementation</summary>

Use Visual Studio 2022 with the x64 C++ tools and CMake 3.21 or newer:

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

Alternatively, run `scripts\BuildWithZig.cmd` with Zig 0.16. Both methods write the executable and DLL to `bin`; then run `scripts\Install.cmd`.

The helper loads a menu DLL into `iCloudHome.exe` without changing Apple's installed files. Restarting stops the iCloud package's processes and launches iCloud again, briefly interrupting sync.

</details>

---

Project: [MIT](LICENSE) · Bundled MinHook: [license](third_party/minhook/LICENSE.txt)
