# iCloud Tray Restart

[简体中文](README.md) · 繁體中文 · [English](README.en.md) · [日本語](README.ja.md)

在 Microsoft Store 版 iCloud for Windows 的系統匣選單加入「重新啟動 iCloud」，方便處理偶發的同步延遲。按鈕文字依 Windows 偏好語言順序顯示，支援英語、簡體中文、繁體中文和日語。

## 安裝

1. 從[最新版本](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest)下載 Windows x64 ZIP，解壓縮到固定位置。
2. 執行 `scripts\Install.cmd`。程式會在目前使用者登入時自動啟動，無須系統管理員權限。
3. 從 iCloud 系統匣選單選擇重新啟動。解除安裝時執行 `scripts\Uninstall.cmd`。

更新前請先結束 iCloud 主視窗並執行解除安裝腳本；替換檔案後重新安裝並啟動 iCloud。

## 相容性與建置

已在 Windows 11 x64、Microsoft Store 版 iCloud for Windows `15.10.39.0` 上驗證。Windows 10 與其他 iCloud 版本尚未實測；僅支援 x64 Store 版。

從原始碼建置需要 Visual Studio 2022 的 x64 C++ 工具組與 CMake：

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

也可使用 Zig 0.16 執行 `scripts\BuildWithZig.cmd`。產出檔案位於 `bin`。

## 其他

程式會將選單 DLL 載入 `iCloudHome.exe`，不修改 Apple 的安裝檔案。選單無法使用時可執行 `tools\RestartICloud.cmd`；日誌位於 `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log`。

專案程式碼採用 [MIT](LICENSE)；內含的 MinHook 使用其[獨立授權條款](third_party/minhook/LICENSE.txt)。
