# UGOKU-Pad Arduino Library 概要
ESP32 を UGOKU Pad アプリで操作するための Arduino ライブラリです。
サーボモーターの操作やアナログ入力読み取りのサンプルを含みます。

### UGOKU Padについて

[UGOKU Pad](https://ugoku-lab.github.io/ugokupad.html)は、ESP32などのマイコンをBluetoothでスマートフォンと接続し、簡単に操作できるアプリです。  
ジョイスティックやスライダー、ボタンなど、色々なウィジェットを組み合わせて、自分だけの操作パネルを作成できます。  
モーターの操作やセンサーデータをモニタリングなど、様々な用途で活用できます。

<img src="https://github.com/user-attachments/assets/b2da444f-e0e3-46c4-aa92-2031e2f38083" width="600">

[<img src="https://github.com/user-attachments/assets/73952bbe-7f89-46e9-9a6e-cdc7eea8e7c8" alt="Get it on Google Play" height="60">](https://play.google.com/store/apps/details?id=com.ugoku_lab.ugoku_console)　[<img src="https://github.com/user-attachments/assets/e27e5d09-63d0-4a2e-9e14-0bb05dabd487" alt="Get it on Google Play" height="60">](https://apps.apple.com/jp/app/ugoku-pad/id6739496098)

### 対応マイコン
BLE対応ESP32, BLE対応ESP32搭載ボード(ESP32-WROOM-32E, M5StickCPlus2で動作確認済み)

### 最小コード例
```cpp
#include <UGOKU-Pad_Controller.h>

UGOKUPadController UGOKUPad;

void setup() {
  UGOKUPad.begin("UGOKU Pad ESP32");
}

void loop() {
  if (!UGOKUPad.update()) return; // 受信値を更新
  uint8_t value = UGOKUPad.read(1); // ch1 の値を取得
  UGOKUPad.write(5, 123); // ch2 に 123 の値を送信
  delay(50);
}
```

# サンプルスケッチ UGOKU-Pad_ESP32_example
このサンプルスケッチでできること
- UGOKU Pad上のトグルスイッチによるデジタル出力の操作
- UGOKU Pad上のアジャスタによるサーボモーターの操作
- UGOKU Pad上のスティックによるサーボモーターの操作
- アナログ入力値のUGOKU Pad上でのモニタリング

### 対応環境
使用ライブラリ：ESP32Servo
マイコン：ESP32-WROOM-32E, ESP32-WROVER-32E で動作確認済

### 
| 機能 | ESP32側のピン | 設定ch |
| ------------- | ------------- | -- |
| トグルスイッチによるデジタル出力の操作 | 27 | ch1 |
| アジャスタによるサーボモーターの操作 | 12 | ch2 |
| スティックによるサーボモーターの操作 | 14 | ch3 |
| アナログ入力値のモニタリング | 26 | ch5 |


# 詳細仕様
### パケット仕様
- 最大19バイト: (ch0,val0)...(ch8,val8) 計18バイト + チェックサム1バイト
- チェックサムは先頭18バイトの XOR
- 0xFF は「未受信/未設定」の目印として利用

### 関数の説明
- `update()` は受信値を更新し、パケットが不正なら前回値を維持します
- `read(channel)` は前回の有効値を返し、未受信なら 0xFF になります
- `read(channel, fallback)` は未受信のときに fallback を返します
- `write(channel, value)` は (channel, value) をアプリへ送信します
- `setDefaultValue(channel, value)` は最初の受信までの初期値を入れます
- `setConnectionHandlers(onConnect, onDisconnect)` は接続/切断時に関数を実行します

### 予約値と制限
- チャンネル値 255 (0xFF) は本ライブラリ内で「未使用ペア」の意味として予約されています。そのため、使用できるチャンネルIDは 0-254 です。
- 値 255 (0xFF) は「未受信/未設定」を表すため、`valueForChannel()` や `read()` では実データと区別できません。読み取り用途では 255 を意味のある値として使わないでください。
- `writeChannel()` は残りの 8 ペアを「ch=0xFF, val=0」で埋めて送信します。
