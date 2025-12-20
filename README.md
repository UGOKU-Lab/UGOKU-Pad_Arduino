# UGOKU-Pad Arduino Library
ESP32 を UGOKU-Pad アプリで操作するための Arduino ライブラリです。サーボ制御やアナログ入力のサンプルを含み、BLE で 9 組の (channel, value) ペアをやり取りします。

## 機能
- BLE 経由で最大 9 ペアの (channel, value) を受信・送信（パケット長 19 バイト、末尾 XOR チェックサム）
- チャンネルごとの最新値取得 API
- サーボ制御とアナログ計測のデモ（例: 対向2輪を1スティックで操作、距離値送信）

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
	controller.begin("UGOKU-Pad ESP32");
	controller.setOnConnectCallback([](){ Serial.println("connected"); });
	controller.setOnDisconnectCallback([](){ Serial.println("disconnected"); });
}

void loop() {
	uint8_t err = controller.readPacket();
	if (err == UGOKU_PAD_NO_ERROR) {
		uint8_t v = controller.valueForChannel(1);
		// ここで受信値を使う
	}
	controller.writeChannel(5, 123); // ch5 へ 123 を返信
	delay(50);
}
```

## サンプルスケッチ
- examples/UGOKU-Pad_Arduino/UGOKU-Pad_Arduino.ino
	- ch1: LED トグル
	- ch2, ch3: 対向2輪サーボ操作（ミキシング）
	- ch5: アナログ距離値を送信

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

## ライセンス
See LICENSE.
uint8_t btn_1 = 0xFF;    // ch1

