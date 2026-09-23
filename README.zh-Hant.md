<div align="center">

# iCloud Tray Restart

**在系統匣裡，輕鬆重新啟動 iCloud。**

[English](README.md) · [简体中文](README.zh-Hans.md) · **繁體中文** · [日本語](README.ja.md)

[**↓ 下載 Windows x64 版**](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest) · [版本說明](https://github.com/fuhkKai/iCloudTrayRestart/releases/tag/v0.1.0) · [回報問題](https://github.com/fuhkKai/iCloudTrayRestart/issues)

</div>

---

在 **Microsoft Store 版 iCloud for Windows** 的原有系統匣選單加入 **「重新啟動 iCloud」**。同步卡住時，不必再手動尋找並結束處理程序。

## 開始使用

1. 從最新版本下載 **Windows x64 ZIP 安裝包**，解壓縮到準備長期保留的資料夾。請選擇程式壓縮檔，而非「Source code」原始碼壓縮檔。
2. 按兩下 `scripts\Install.cmd`。無須系統管理員權限，輔助程式會立即執行，並在之後登入時自動啟動。
3. 開啟 iCloud，在系統匣的 iCloud 圖示上按滑鼠右鍵，選擇 **「重新啟動 iCloud」**。

選單文字依 Windows 偏好語言順序顯示，支援繁體中文、简体中文、English 和日本語。

> 已驗證：Windows 11 x64 + Microsoft Store 版 iCloud **15.10.39.0**。Windows 10 與其他 iCloud 版本尚未實測；僅支援 x64 Store 版。

## 更多資訊

<details>
<summary>更新與解除安裝</summary>

**更新：** 結束 iCloud，執行 `scripts\Uninstall.cmd`，替換解壓縮後的檔案，再執行 `scripts\Install.cmd` 並重新開啟 iCloud。

**解除安裝：** 執行 `scripts\Uninstall.cmd`，然後結束並重新開啟 iCloud，以移除已載入的選單擴充功能。腳本會保留專案資料夾，之後可自行刪除。

</details>

<details>
<summary>選單未出現或需要疑難排解</summary>

請確認 iCloud 正在執行，且解壓縮後的資料夾未被移動。選單無法使用時，可以執行 `tools\RestartICloud.cmd`。

回報問題時，請附上 Windows 和 iCloud 版本，以及 `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log` 中的相關日誌。

</details>

<details>
<summary>從原始碼建置 · 實作方式</summary>

需要 Visual Studio 2022 的 x64 C++ 工具組及 CMake 3.21 或更新版本：

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

也可以使用 Zig 0.16 執行 `scripts\BuildWithZig.cmd`。兩種方式均將程式與 DLL 輸出至 `bin`，之後執行 `scripts\Install.cmd` 即可。

輔助程式會將選單 DLL 載入 `iCloudHome.exe`，不修改 Apple 的安裝檔案。重新啟動時會結束 iCloud 應用程式套件的處理程序，再啟動 iCloud，同步會短暫中斷。

</details>

---

專案：[MIT](LICENSE) · 內含 MinHook：[獨立授權條款](third_party/minhook/LICENSE.txt)
