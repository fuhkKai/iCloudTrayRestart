# iCloud Tray Restart

[简体中文](README.zh-Hans.md) · [繁體中文](README.zh-Hant.md) · [English](README.md) · 日本語

Microsoft Store 版 iCloud for Windows のトレイメニューに「iCloudを再起動」を追加します。同期が滞ったときに利用できます。項目名は Windows の優先する言語の順序に従い、英語・簡体字中国語・繁体字中国語・日本語に対応します。

## インストール

1. [最新リリース](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest)から Windows x64 ZIP をダウンロードし、移動しないフォルダーに展開します。
2. `scripts\Install.cmd` を実行します。現在のユーザーのサインイン時に自動起動し、管理者権限は不要です。
3. iCloud のトレイメニューから再起動項目を選びます。アンインストールには `scripts\Uninstall.cmd` を実行します。

更新時は、先に iCloud のメイン画面を終了してアンインストール用スクリプトを実行し、ファイルを入れ替えてから再インストールして iCloud を起動してください。

## 対応環境とビルド

Windows 11 x64 と Microsoft Store 版 iCloud for Windows `15.10.39.0` で動作を確認しました。Windows 10 と他の iCloud バージョンは未検証です。x64 の Store 版のみ対応します。

ソースからのビルドには Visual Studio 2022 の x64 C++ ツールと CMake を使用します。

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

Zig 0.16 で `scripts\BuildWithZig.cmd` を実行する方法もあります。生成物は `bin` に保存されます。

## 補足

メニュー用 DLL を `iCloudHome.exe` に読み込みますが、Apple のインストール済みファイルは変更しません。メニューが使えない場合は `tools\RestartICloud.cmd` を実行できます。ログは `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log` に保存されます。

プロジェクトのコードは [MIT ライセンス](LICENSE)です。同梱の MinHook には[別のライセンス](third_party/minhook/LICENSE.txt)が適用されます。
