# UGOKU-Pad Arduino Library 概要
README in English is here: [README.en.md](README.en.md)

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
  UGOKUPad.begin("My ESP32");
}

void loop() {
  if (!UGOKUPad.update()) return; // 受信値を更新
  uint8_t value = UGOKUPad.read(1); // ch1 の値を取得
  UGOKUPad.write(2, 123); // ch2 に 123 の値を送信
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

### 機能
| 機能 | ESP32側のピン | 設定ch |
| ------------- | ------------- | -- |
| トグルスイッチによるデジタル出力の操作 | 27 | ch1 |
| アジャスタによるサーボモーターの操作 | 12 | ch2 |
| スティックによるサーボモーターの操作 | 14 | ch3 |
| アナログ入力値のモニタリング | 26 | ch5 |

### サンプルスケッチ解説 
<details>
<summary>サンプルスケッチの解説はこちら</summary>

### ヘッダーとライブラリ読み込み
UGOKU Pad と通信するためのライブラリと、ESP32でサーボを使うためのライブラリを読み込みます。
```cpp
#include <UGOKU-Pad_Controller.h>
#include <ESP32Servo.h>
```

### グローバル変数（プログラム全体で使うもの）
- `UGOKUPad` はUGOKU Padとやり取りするための箱（オブジェクト）です。
- `isConnected` はスマホがつながっているかどうかを覚えておくフラグ（true/false）です。
- `servo2` と `servo3` はサーボモーターを動かすための変数です。

```cpp
UGOKUPadController UGOKUPad;
bool isConnected = false;

Servo servo2;
Servo servo3;
```

### 接続されたときに呼ばれる関数
UGOKU PadがESP32に接続したときに実行されます。サーボを指定ピンに接続して（attach）、中央の90度に動かします。ローテーションサーボの場合は停止させます。
```cpp
void onConnect() {
  isConnected = true;
  servo2.attach(12);
  servo3.attach(14);
  servo2.write(90);
  servo3.write(90);
}
```

### 切断されたときに呼ばれる関数
UGOKU Padが切断されたら呼ばれます。フラグを下ろして（false）、LEDなどの出力を消し、サーボの制御を止めて安全にします。
```cpp
void onDisconnect() {
  isConnected = false;
  digitalWrite(27, LOW);
  servo2.detach();
  servo3.detach();
}
```
### 初期化：`setup()`
起動時に一度だけ動く部分です。
- `UGOKUPad.begin("My ESP32")` でデバイス名を決めて通信を始めます。
- 接続/切断のときに先ほどの関数を呼ぶよう登録します。
- ピンの役割を決めます。ピン26はセンサーなどの入力、ピン27はLEDなどの出力です。
```cpp
void setup() {
  UGOKUPad.begin("My ESP32");
  UGOKUPad.setConnectionHandlers(onConnect, onDisconnect);
  pinMode(26, INPUT);
  pinMode(27, OUTPUT);
}
```

### 繰り返し処理：`loop()`
```cpp
void loop() {
  //接続していないときにループに入らないようにしています。
  if (!isConnected) return;

  //アプリから来た最新データを受け取ります。データに問題があれば中断します。
  if (!UGOKUPad.update()) return;
  
  //アプリの ch1（トグルスイッチ）を読み取りピン27をオン/オフします。
  digitalWrite(27, UGOKUPad.read(1)); 
  
  //アプリの ch2 と ch3 の値（0〜180）をサーボ角度(ローテーションサーボの場合は速度)にして動かします。
  servo2.write(UGOKUPad.read(2)); 
  servo3.write(UGOKUPad.read(3));
  
  //アナログ入力ピンの値を読み、0〜4095の値を0〜100のパーセントに変換してからでアプリに送ります。
  uint8_t percent = (analogRead(26) * 100U) / 4095U; // 0-100 from ADC.
  UGOKUPad.write(5, percent); // Send 0-100 value.

  //Small delay to avoid flooding
  delay(50);
}
```
</details>
<br>

# 詳細仕様
### 送受信の仕組み
- 1回(1パケット)の通信は **19バイト固定** です
- 中身は **(ch, value) の9組 + チェックサム1バイト**
- **1回で送れるのは最大9チャンネル** です  
  10チャンネル以上を使うときは、複数回に分けて送ります
- チャンネルIDは **0-254** を使えます  
  `0xFF` は「未使用」の印として予約されています
- 値 `0xFF` は「未受信/未設定」を表すため、意味のある値として使わないでください

### 主な関数
- `update()` 受信値を更新します。パケットが不正なら前回値を維持します
- `read(channel)` 受信値を取得します。未受信なら `0xFF` です
- `read(channel, fallback)` 未受信のときに fallback を返します
- `write(channel, value)` アプリへ (channel, value) を送信します
- `setDefaultValue(channel, value)` 最初の受信までの初期値を入れます
- `setConnectionHandlers(onConnect, onDisconnect)` 接続/切断時に関数を実行します
