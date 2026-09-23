<div align="center">

# iCloud Tray Restart

**在托盘里，轻松重启 iCloud。**

[English](README.md) · **简体中文** · [繁體中文](README.zh-Hant.md) · [日本語](README.ja.md)

[**↓ 下载 Windows x64 版**](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest) · [版本说明](https://github.com/fuhkKai/iCloudTrayRestart/releases/tag/v0.1.0) · [反馈问题](https://github.com/fuhkKai/iCloudTrayRestart/issues)

</div>

---

为 **Microsoft Store 版 iCloud for Windows** 的原有托盘菜单添加 **「重新启动 iCloud」**。同步卡住时，不必再手动寻找并结束进程。

## 开始使用

1. 从最新版本下载 **Windows x64 ZIP 安装包**，解压到准备长期保留的文件夹。请选择程序压缩包，而非「Source code」源码包。
2. 双击 `scripts\Install.cmd`。无需管理员权限，辅助程序会立即运行，并在以后登录时自动启动。
3. 打开 iCloud，右键点击系统托盘中的 iCloud 图标，选择 **「重新启动 iCloud」**。

菜单文字跟随 Windows 首选语言顺序，支持简体中文、繁體中文、English 和日本語。

> 已验证：Windows 11 x64 + Microsoft Store 版 iCloud **15.10.39.0**。Windows 10 和其他 iCloud 版本尚未实测；仅支持 x64 Store 版。

## 更多信息

<details>
<summary>更新与卸载</summary>

**更新：** 退出 iCloud，运行 `scripts\Uninstall.cmd`，替换解压后的文件，再运行 `scripts\Install.cmd` 并重新打开 iCloud。

**卸载：** 运行 `scripts\Uninstall.cmd`，然后退出并重新打开 iCloud，以移除已加载的菜单扩展。脚本会保留项目文件夹，之后可自行删除。

</details>

<details>
<summary>菜单未出现或需要排查</summary>

请确认 iCloud 正在运行，且解压后的文件夹未被移动。菜单不可用时，可以运行 `tools\RestartICloud.cmd`。

反馈问题时，请附上 Windows 和 iCloud 版本，以及 `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log` 中的相关日志。

</details>

<details>
<summary>从源码构建 · 实现方式</summary>

需要 Visual Studio 2022 的 x64 C++ 工具集及 CMake 3.21 或更新版本：

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

也可以使用 Zig 0.16 运行 `scripts\BuildWithZig.cmd`。两种方式均将程序与 DLL 输出到 `bin`，之后运行 `scripts\Install.cmd` 即可。

辅助程序向 `iCloudHome.exe` 加载菜单 DLL，不修改 Apple 安装文件。重启时会结束 iCloud 应用包的进程并重新启动 iCloud，同步会短暂中断。

</details>

---

项目：[MIT](LICENSE) · 内置 MinHook：[独立许可证](third_party/minhook/LICENSE.txt)
