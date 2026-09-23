# iCloud Tray Restart

简体中文 · [繁體中文](README.zh-Hant.md) · [English](README.en.md) · [日本語](README.ja.md)

为 Microsoft Store 版 iCloud for Windows 的托盘菜单添加「重新启动 iCloud」，用于处理偶发的同步延迟。按钮文字按 Windows 首选语言顺序显示，支持英、简中、繁中、日语。

## 安装

1. 从[最新版本](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest)下载 Windows x64 ZIP，解压到固定位置。
2. 双击 `scripts\Install.cmd`。程序会在当前用户登录时自动启动，无需管理员权限。
3. 从 iCloud 托盘菜单选择重启项。卸载时运行 `scripts\Uninstall.cmd`。

更新前请退出 iCloud 主界面并运行卸载脚本；替换文件后重新安装并启动 iCloud。

## 兼容与构建

已在 Windows 11 x64、Microsoft Store 版 iCloud for Windows `15.10.39.0` 上验证。Windows 10 和其他 iCloud 版本尚未实测；仅支持 x64 Store 版。

从源码构建需要 Visual Studio 2022 的 x64 C++ 工具集和 CMake：

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

也可使用 Zig 0.16 运行 `scripts\BuildWithZig.cmd`。产物位于 `bin`。

## 其他

程序向 `iCloudHome.exe` 加载菜单 DLL，不修改 Apple 安装文件。菜单不可用时可运行 `tools\RestartICloud.cmd`；日志位于 `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log`。

项目代码采用 [MIT](LICENSE)；内置 MinHook 遵循其[独立许可证](third_party/minhook/LICENSE.txt)。
