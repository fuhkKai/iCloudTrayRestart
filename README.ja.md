<div align="center">

# iCloud Tray Restart

**トレイから、手軽に iCloud を再起動。**

[English](README.md) · [简体中文](README.zh-Hans.md) · [繁體中文](README.zh-Hant.md) · **日本語**

[**↓ Windows x64 版をダウンロード**](https://github.com/fuhkKai/iCloudTrayRestart/releases/latest) · [リリースノート](https://github.com/fuhkKai/iCloudTrayRestart/releases/tag/v0.1.0) · [問題を報告](https://github.com/fuhkKai/iCloudTrayRestart/issues)

</div>

---

**Microsoft Store 版 iCloud for Windows** の既存のトレイメニューに **「iCloudを再起動」** を追加します。同期が止まったとき、プロセスを探して手動で終了する手間を省けます。

## 使い始める

1. 最新リリースから **Windows x64 ZIP** をダウンロードし、移動せずに使い続けるフォルダーに展開します。「Source code」ではなく、アプリの ZIP を選んでください。
2. `scripts\Install.cmd` をダブルクリックします。管理者権限は不要です。ヘルパーが起動し、次回以降はサインイン時に自動起動します。
3. iCloud を開き、システムトレイの iCloud アイコンを右クリックして **「iCloudを再起動」** を選びます。

メニューの表示言語は Windows の優先する言語の順序に従います。日本語・English・简体中文・繁體中文に対応しています。

> 動作確認済み：Windows 11 x64 + Microsoft Store 版 iCloud **15.10.39.0**。Windows 10 と他の iCloud バージョンは未検証です。x64 の Store 版のみ対応しています。

## 詳しい情報

<details>
<summary>更新・アンインストール</summary>

**更新：** iCloud を終了し、`scripts\Uninstall.cmd` を実行します。展開したファイルを置き換え、`scripts\Install.cmd` を実行して iCloud を開き直してください。

**アンインストール：** `scripts\Uninstall.cmd` を実行した後、iCloud を終了して開き直すと、読み込み済みのメニュー拡張も解除されます。プロジェクトのフォルダーは残るため、その後に削除できます。

</details>

<details>
<summary>メニューが表示されない・問題を調べる</summary>

iCloud が起動していることと、展開先のフォルダーを移動していないことを確認してください。メニューを使えない場合は `tools\RestartICloud.cmd` を実行できます。

問題の報告には Windows と iCloud のバージョン、および `%LOCALAPPDATA%\iCloudTrayRestart\Logs\bridge.log` の関連箇所を添えてください。

</details>

<details>
<summary>ソースからビルド・仕組み</summary>

Visual Studio 2022 の x64 C++ ツールと CMake 3.21 以降を使用します。

```bat
cmake -S . -B build\cmake -A x64
cmake --build build\cmake --config Release
```

Zig 0.16 で `scripts\BuildWithZig.cmd` を実行する方法もあります。どちらも実行ファイルと DLL を `bin` に出力します。その後、`scripts\Install.cmd` を実行してください。

ヘルパーは `iCloudHome.exe` にメニュー DLL を読み込みます。Apple のインストール済みファイルは変更しません。再起動時は iCloud パッケージのプロセスを終了して iCloud を起動し直すため、同期が一時的に中断します。

</details>

---

プロジェクト：[MIT](LICENSE) · 同梱 MinHook：[ライセンス](third_party/minhook/LICENSE.txt)
