# iCloud Tray Restart

为 Microsoft Store 版 iCloud for Windows 的系统托盘菜单增加重启项。按钮文字按 Windows 用户首选语言的顺序，采用首个受支持的语言：

- 英语：`Restart iCloud`
- 日语：`iCloudを再起動`
- 简体中文：`重新启动 iCloud`
- 繁体中文：`重新啟動 iCloud`

若首选语言列表中没有受支持的语言，或读取失败，则回退到英语。重启功能已验证可用于恢复偶发的同步延迟。

## 运行结构

- `ICloudTrayBridge.exe`：监视 `iCloudHome.exe`、加载菜单模块，并处理重启请求。
- `ICloudTrayMenu.dll`：在 iCloud 原生托盘菜单弹出时加入重启项目。

程序不会修改 `WindowsApps` 中的 Apple 文件，也不依赖 iCloud 的版本化安装路径。目标进程必须属于包族 `AppleInc.iCloud_nzyj5cx40ttqa`。

## 兼容范围

- 已在 Windows 11 x64、Microsoft Store 版 iCloud for Windows `15.10.39.0` 上验证。其他 iCloud 版本和 Windows 10 尚未实测。Apple 当前推荐 Windows 11 和 iCloud for Windows 15，详见[官方系统要求](https://support.apple.com/118308)。
- 仅构建 x64 程序，并要求 `iCloudHome.exe` 是当前登录会话中的 x64 进程。ARM64 原生版、32 位版及非 Store 版不在当前支持范围内。
- 托盘菜单识别依赖其中同时出现 `iCloud.com` 和另一项包含 `iCloud` 的文字；它不依赖“帮助”一词的语言。如果 Apple 更改菜单结构，可能需要更新识别逻辑。
- 重启项通过 Windows 的 `GlobalizationPreferences.Languages` 接口读取首选语言顺序，不读取 Windows 显示语言或 iCloud 菜单文字。当前机器已验证该接口返回简体中文、日语的顺序；简体中文、繁体中文、英语和日语的托盘菜单显示均已实测。
- 注入菜单模块可能受到安全软件或组织的应用控制策略阻止。失败原因写入日志；程序不会尝试绕过这些限制。

## 构建

克隆源码后先构建，再运行安装脚本；Git 仓库不包含 `bin` 中的编译产物。

Visual Studio 2022 的 x64 C++ 工具集与 CMake 3.21 或更高版本：

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

也可以使用 Zig 0.16：

```bat
set ZIG_EXE=C:\path\to\zig.exe
scripts\BuildWithZig.cmd
```

生成文件位于 `bin`：

- `ICloudTrayBridge.exe`
- `ICloudTrayMenu.dll`

## 安装与卸载

确认 `bin` 中有上述两个文件后，双击 `scripts\Install.cmd` 注册登录自启动并立即启动 Bridge。安装脚本在当前用户的“启动”文件夹中创建快捷方式，不要求管理员权限。该方式用于避免本机登录时静默忽略自定义 `Run` 项的问题。

双击 `scripts\Uninstall.cmd` 删除启动快捷方式以及旧版本遗留的计划任务和 `Run` 启动项，并停止 Bridge。卸载脚本不会删除源码或构建产物。

更新已有安装时，先停止 Bridge 并退出 iCloud 主界面，再重新构建或替换 `bin` 中的文件；旧 DLL 在 iCloud 主界面退出前可能仍被加载。之后重新运行安装脚本并启动 iCloud。

## 救援入口

`tools\RestartICloud.cmd` 可在托盘菜单不可用时独立重启 iCloud。

## 日志

Bridge 日志写入：

`%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log`

## 已知边界

- Apple 如果更换进程名称、包身份或托盘菜单实现，可能需要更新程序。
- iCloud 必须在当前用户会话中运行，才能显示菜单项。
- 如果菜单项没有出现，请查看 Bridge 日志。不要把本机日志、个人文件或 iCloud 数据上传到公开仓库。

## 授权

本项目自有代码采用 [MIT 许可证](LICENSE)。内置的 MinHook 使用独立的 BSD 风格许可证，见 [`third_party/minhook/LICENSE.txt`](third_party/minhook/LICENSE.txt)。发布二进制包时也应附带该许可证文本。

## 当前验证记录

验证环境：

- iCloud for Windows 包版本 `15.10.39.0`
- `iCloudHome.exe` 文件版本 `610.6.0.24`
- x64 Windows 当前用户会话
- iCloud 显示语言：日语

已验证：

- Bridge 能识别 Store 包身份并加载菜单模块。
- 原有 `iCloud.comへ移動` 与 `iCloudヘルプ` 保持可用。
- 分隔线及 `iCloudを再起動` 正确显示。
- 按不同的首选语言顺序切换后，简体中文、繁体中文、英语和日语的重启项均与 iCloud 菜单语言一致。
- 重启请求会结束当前包的 iCloud 进程并重新启动应用。
- 新的 `iCloudHome.exe` PID 出现后菜单模块会自动恢复。
- Bridge 自身重新启动后保持单实例，并能识别已加载的菜单模块。
- 当前用户“启动”文件夹快捷方式能避开本机对自定义 `Run` 项的静默忽略。
- 实际出现同步延迟时，托盘重启项能使 iCloud 恢复同步。
