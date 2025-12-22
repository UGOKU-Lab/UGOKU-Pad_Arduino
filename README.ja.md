# UGOKU-Pad Arduino Library
ESP32 を UGOKU Pad アプリで操作するための Arduino ライブラリです。サーボ制御やアナログ入力のサンプルを含み、BLE で 9 組の (channel, value) ペアをやり取りします。

## UGOKU Padについて
<img src="https://github.com/user-attachments/assets/b2da444f-e0e3-46c4-aa92-2031e2f38083" width="600">

[UGOKU Pad](https://ugoku-lab.github.io/ugokupad.html)は、ESP32などのマイコンをBluetoothでスマートフォンと接続し、簡単に操作できるアプリです。  
ジョイスティックやスライダー、ボタンなど、色々なウィジェットを組み合わせて、自分だけの操作パネルを作成できます。  
モーターの操作やセンサーデータをモニタリングなど、様々な用途で活用できます。

[<img src="https://github.com/user-attachments/assets/73952bbe-7f89-46e9-9a6e-cdc7eea8e7c8" alt="Get it on Google Play" height="60">](https://play.google.com/store/apps/details?id=com.ugoku_lab.ugoku_console)　[<img src="https://github.com/user-attachments/assets/e27e5d09-63d0-4a2e-9e14-0bb05dabd487" alt="Get it on Google Play" height="60">](https://apps.apple.com/jp/app/ugoku-pad/id6739496098)


## 機能
- BLE 経由で最大 9 ペアの (channel, value) を受信・送信（パケット長 19 バイト、末尾 XOR チェックサム）
- チャンネルごとの最新値取得 API
- 受信エラー時に前回値を維持するキャッシュ読み取り

## 必要環境
- Arduino IDE 2.x
- ボード: ESP32 Dev Module（ESP32-WROOM/WROVER で動作確認）
- 依存ライブラリ: ESP32Servo

## インストール
1. 「スケッチ > ライブラリをインクルード > .ZIP 形式のライブラリをインストール」で本リポジトリを追加（または `libraries/UGOKU-Pad` に配置）。
2. ライブラリマネージャーで **ESP32Servo** をインストール。
3. ボードマネージャーで **esp32 by Espressif Systems** をインストールし、ボードに **ESP32 Dev Module** を選択。

## 使い方（最小例）
```cpp
#include <UGOKU-Pad_Controller.h>

UGOKUPadController controller;

void setup() {
  controller.begin("UGOKU Pad ESP32");
}

void loop() {
  controller.update(); // readPacketCached() と同じ
  uint8_t v = controller.read(1); // valueForChannel() と同じ
  if (v != 0xFF) {
    // ここで受信値を使う
  }
  controller.write(5, 123); // writeChannel() と同じ
  delay(50);
}
```

## ショートカット
- `update()` は `readPacketCached()` のラッパーです。
- `read(channel)` は `valueForChannel(channel)` のラッパーです。
- `write(channel, value)` は `writeChannel(channel, value)` のラッパーです。
- `valueForChannelCached(channel, fallback)` は 0xFF を無視して前回の有効値を返します。

## サンプルスケッチ
- examples/UGOKU-Pad_ESP32_example/UGOKU-Pad_ESP32_example.ino
    - ch1: トグルスイッチによるデジタル出力ピンのオンオフ
    - ch2, ch3: アジャスター、スティックによるサーボモーター操作
    - ch5: ADC から 0-100 の値を送信

## ピン配置（サンプル）
| 機能 | ピン |
| ------------- | ------------- |
| デジタル出力 (LED) | 27 |
| 測距モジュール (アナログ) | 26 |
| RCサーボ | 14 |
| ローテーションサーボ | 12 |

## パケット仕様
- 19 バイト固定: (ch0,val0)...(ch8,val8) 計18バイト + チェックサム1バイト
- チェックサムは先頭18バイトの XOR
- 0xFF は「未受信/未設定」の目印として利用

## 予約値と制限
- チャンネル値 255 (0xFF) は本ライブラリ内で「未使用ペア」の意味として予約されています。そのため、使用できるチャンネルIDは 0-254 です。
- 値 255 (0xFF) は「未受信/未設定」を表すため、`valueForChannel()` や `read()` では実データと区別できません。読み取り用途では 255 を意味のある値として使わないでください。
- `writeChannel()` は残りの 8 ペアを「ch=0xFF, val=0」で埋めて送信します。
