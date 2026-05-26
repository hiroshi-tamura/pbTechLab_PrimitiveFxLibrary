# pbTechLab Primitive Fx Library

[English README](README.md)

![pbTechLab Primitive Fx Library GUI collection](public_gui_screenshots/pbTechLab_PrimitiveFxLibrary_GUI_Collection_Public.png)

pbTechLab Primitive Fx Library は、C++ と JUCE 8 で作成した軽量オーディオエフェクトプラグイン集です。現在は 17 種類のエフェクトを収録し、Windows / macOS 向けに VST3 と AAX のインストーラーをリリース資産として提供します。

各プラグインは、共通DSPコアと WebView ベースのエディターフレームワークを共有します。製品名、plug-in code、パラメーター、HTML/CSS/JavaScript UI、DAWオートメーション、状態保存、バイパス、入出力メーターは各プラグインごとに用意されています。

## リリースパッケージ

インストーラーは Git 管理には含めず、GitHub Release の添付ファイルとして配布します。

| Platform | Installer | Included formats |
|---|---|---|
| Windows 64-bit | `pbTechLab_PrimitiveFxLibrary_1.0.0_Windows_Setup.exe` | VST3, AAX |
| macOS universal | `pbTechLab_PrimitiveFxLibrary-1.0.0-macOS.pkg` | VST3, AAX |

macOS PKG は署名、公証、staple 済みです。Windows AAX プラグインは PACE 署名済みです。Windows インストーラー本体は、公開信頼された Authenticode 証明書をこのリポジトリに含めないため未署名です。

## 収録プラグイン

| Plug-in | Main controls | Purpose |
|---|---|---|
| pbPFL Reverb | Pre Delay, Size, Damping, Diffusion, Mod, Width, Dry/Wet, Output | アンビエンス、ルーム感、広がりのあるリバーブテールを作るステレオリバーブ。 |
| pbPFL Delay | Time, Feedback, Tone, Spread, Mod Rate, Mod Depth, Dry/Wet, Output | フィードバック、トーン、ステレオ広がり、モジュレーションを備えたディレイ。 |
| pbPFL Distortion | Drive, Bias, Tone, Low Cut, Shape, Presence, Dry/Wet, Output | 倍音、歪み、質感、存在感を調整するドライブプロセッサー。 |
| pbPFL Compressor | Threshold, Ratio, Attack, Release, Knee, Makeup, Dry/Wet, Output | パラレルコンプレッションにも対応するスタジオ系コンプレッサー。 |
| pbPFL Limiter | Input, Ceiling, Release, Soft Clip, Stereo Link, Density, Dry/Wet, Output | ピーク制御と音圧調整用のリミッター。 |
| pbPFL 3BandEQ | Low, Low Freq, Mid, Mid Freq, Mid Q, High, Dry/Wet, Output | 大きな音色補正に使いやすい3バンドEQ。 |
| pbPFL 4BandEQ | Low, Low Freq, Low Mid, Low Mid Freq, High Mid, High Mid Freq, Dry/Wet, Output | より細かな音色補正向けの4バンドEQ。 |
| pbPFL PitchShifter | Semitone, Cents, Grain, Crossfade, Tone, Stability, Dry/Wet, Output | ハーモニーやサウンドデザインに使えるタイムドメイン系ピッチシフター。 |
| pbPFL Chorus | Rate, Depth, Delay, Feedback, Spread, Tone, Dry/Wet, Output | 広がり、揺れ、モジュレーション感を加えるステレオコーラス。 |
| pbPFL Phaser | Rate, Depth, Center, Feedback, Stages, Spread, Dry/Wet, Output | フィードバックとステレオ広がりを持つオールパス系フェイザー。 |
| pbPFL Flanger | Rate, Depth, Delay, Feedback, Spread, Tone, Dry/Wet, Output | 短い変調ディレイとフィードバックによるステレオフランジャー。 |
| pbPFL StereoEnhancer | Width, Enhance, Focus, Bass Mono, Air, Center, Dry/Wet, Output | ステレオ強調と低域モノ化を行うエンハンサー。 |
| pbPFL StereoWidth | Width, Mono, Balance, Rotation, Low Mono, Focus, Dry/Wet, Output | ステレオ幅、モノブレンド、バランス、回転を調整するユーティリティ。 |
| pbPFL MidSideProcessor | Mid Gain, Side Gain, Width, Balance, Tilt, Mono Bass, Dry/Wet, Output | Mid/Side の音量、幅、バランスを調整するプロセッサー。 |
| pbPFL AutoPan | Rate, Depth, Phase, Shape, Offset, Smooth, Dry/Wet, Output | ステレオ位相制御付きのオートパン。 |
| pbPFL Tremolo | Rate, Depth, Shape, Phase, Smooth, Bias, Dry/Wet, Output | 音量変調によるパルス、揺れ、リズム表現用トレモロ。 |
| pbPFL Vibrato | Rate, Depth, Delay, Shape, Spread, Smooth, Dry/Wet, Output | ピッチ変調によるビブラート系の揺れを作るエフェクト。 |

## エディター

Windows 版のリリースプラグインは、`Common/PrimitiveFxImGuiEditor.*` の共通ネイティブ ImGui エディターを使います。横一列のコンパクトなノブ配置、A/B、プリセット操作、Undo、Redo、Bypass の埋め込み PNG ヘッダーアイコンを全プラグインで統一しています。従来の `mock` フォルダは、ブラウザで開ける見た目確認用およびネイティブ ImGui 経路を使わないプラットフォーム向け WebView アセットとして残しています。

エディター機能:

- ノブ数に合わせた固定サイズGUI;
- 横一列のノブ配置、統一された `Dry/Wet` / `Output` 配置、左右の無駄な空白を避ける自動間隔調整;
- ドラッグ、微調整ドラッグ、ホイール、ダブルクリックリセット;
- A/B スロットボタンとプリセットセレクター;
- Save と Help コントロール;
- 全17プラグイン共通の PNG Undo / Redo / Bypass ヘッダーアイコン;
- `pbTechLab`、ライブラリバージョン、クリック可能な `https://pbtechlab.com/` を中央表示するヘルプ;
- 入出力メーター;
- JUCE APVTS 経由のホスト向けパラメーター更新。

## DSP と状態保存

`Common/PrimitiveFxProcessor.*` が APVTS パラメーター作成、状態保存、メーター、チャンネルレイアウト、エフェクト処理の分岐を担当します。各プラグインは個別の `CMakeLists.txt` から compile-time 定義を渡し、同じ共通実装を別製品としてビルドします。

Dry/Wet は未処理バッファと処理後バッファの真のミックスです。

```text
output = dry * (1 - wet) + processed * wet
```

100% Wet では明示的な dry バッファは混ざりません。ただし EQ、ダイナミクス、ステレオ処理、パン、トレモロなどは、処理後信号自体に原音として認識できる成分が残ります。

## ソースからのビルド

必要環境:

- CMake 3.22 以上
- C++17 compiler
- JUCE 8 対応ビルド環境
- Windows は Visual Studio 2022
- macOS は Xcode
- AAX をビルドする場合のみ AAX SDK
- 配布可能な AAX を作る場合のみ PACE signing tools

Windows の基本的な VST3/Standalone 開発ビルド:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

VST3/AAX リリース形式ビルド例:

```powershell
cmake -S . -B build-release -G "Visual Studio 17 2022" -A x64 `
  -DPB_ENABLE_AAX=ON `
  -DPB_BUILD_STANDALONE=OFF `
  -DPB_AAX_SDK_PATH="path/to/aax-sdk"

cmake --build build-release --config Release
```

ローカルJUCEを使う場合:

```powershell
-DPB_JUCE_LOCAL="path/to/JUCE"
```

ローカルJUCEを指定しない場合、CMake が JUCE 8.0.0 を取得します。

## プライバシー

このリポジトリには、ローカルビルドログ、インストーラー実体、サーバー固有スクリプト、API資格情報、署名資格情報、秘密鍵、マシン名、ローカル引き継ぎファイルを含めません。インストーラーは GitHub Releases でのみ配布します。
