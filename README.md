# UGOKU-Pad Arduino Library
日本語はこちら: [README.ja.md](README.ja.md)

An Arduino library for controlling an ESP32 from the UGOKU-Pad app. It includes examples for servo control and analog input, and exchanges up to 9 (channel, value) pairs over BLE.

## Features
- Exchange up to 9 (channel, value) pairs over BLE (fixed packet length 19 bytes, trailing XOR checksum).
- API to read the latest value per channel.
- Demo for servo control and analog measurements (e.g., differential two-wheel drive with one stick, distance value transmission).

## Requirements
- Arduino IDE 2.x
- Board: ESP32 Dev Module (tested with ESP32-WROOM/WROVER)
- Dependency: ESP32Servo

## Installation
1. Add this repository to the Arduino IDE via "Sketch > Include Library > Add .ZIP Library" or place it under `libraries/UGOKU-Pad`.
2. Install **ESP32Servo** using the Library Manager.
3. Install **esp32 by Espressif Systems** from the Boards Manager and select "ESP32 Dev Module".

## Usage (minimal example)
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
		// use received value here
	}
	controller.writeChannel(5, 123); // reply 123 to ch5
	delay(50);
}
```

## Example sketch
- examples/UGOKU-Pad_Arduino/UGOKU-Pad_Arduino.ino
	- ch1: toggle LED
	- ch2, ch3: differential two-wheel servo control (mixing)
	- ch5: send analog distance value

## Sample pinout
| Function | Pin |
| ------------- | ------------- |
| Digital output (LED) | 27 |
| Distance sensor (analog) | 26 |
| RC servo | 14 |
| Rotation servo | 12 |

## Packet format
- Fixed 19 bytes: (ch0,val0)...(ch8,val8) total 18 bytes + 1 checksum byte
- Checksum is XOR of the first 18 bytes
- 0xFF is used as an indicator for "not received / unset"
