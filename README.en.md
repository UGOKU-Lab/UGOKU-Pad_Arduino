# UGOKU-Pad Arduino Library

An Arduino library for controlling an ESP32 from the UGOKU Pad app.
It includes samples for servo motor control and analog input reading.

## About UGOKU Pad

[UGOKU Pad](https://ugoku-lab.github.io/ugokupad.html) is an app that connects a microcontroller such as an ESP32 to a smartphone over Bluetooth and lets you control it easily.
You can combine joysticks, sliders, and buttons to build your own control panel.
It can be used to drive motors or monitor sensor data in many projects.

<img src="https://github.com/user-attachments/assets/b2da444f-e0e3-46c4-aa92-2031e2f38083" width="600">

[<img src="https://github.com/user-attachments/assets/73952bbe-7f89-46e9-9a6e-cdc7eea8e7c8" alt="Get it on Google Play" height="60">](https://play.google.com/store/apps/details?id=com.ugoku_lab.ugoku_console) [<img src="https://github.com/user-attachments/assets/e27e5d09-63d0-4a2e-9e14-0bb05dabd487" alt="Download on the App Store" height="60">](https://apps.apple.com/jp/app/ugoku-pad/id6739496098)

## Supported MCUs
BLE-capable ESP32 and boards with BLE-capable ESP32 (tested with ESP32-WROOM-32E and M5StickCPlus2).

## Minimal code example
```cpp
#include <UGOKU-Pad_Controller.h>

UGOKUPadController UGOKUPad;

void setup() {
  UGOKUPad.begin("UGOKU Pad ESP32");
}

void loop() {
  if (!UGOKUPad.update()) return; // Refresh received values
  uint8_t value = UGOKUPad.read(1); // Get ch1 value
  UGOKUPad.write(2, 123); // Send 123 on ch2
  delay(50);
}
```

## Sample sketch UGOKU-Pad_ESP32_example
What this sample sketch can do:
- Control a digital output using a toggle switch in UGOKU Pad
- Control a servo motor with an adjuster widget
- Control a servo motor with a stick widget
- Monitor analog input values in UGOKU Pad

### Environment
Library: ESP32Servo
MCU: tested with ESP32-WROOM-32E, ESP32-WROVER-32E

### Functions
| Function | ESP32 pin | Channel |
| ------------- | ------------- | -- |
| Toggle switch digital output | 27 | ch1 |
| Adjuster servo control | 12 | ch2 |
| Stick servo control | 14 | ch3 |
| Analog input monitoring | 26 | ch5 |

### Sample sketch walkthrough
<details>
<summary>Click here for the walkthrough</summary>

### Headers and library includes
This sketch uses the UGOKU Pad library for communication and ESP32Servo for servo control.
```cpp
#include <UGOKU-Pad_Controller.h>
#include <ESP32Servo.h>
```

### Global variables
- `UGOKUPad` handles communication with the app.
- `isConnected` tracks whether the phone is connected.
- `servo2` and `servo3` control the servo motors.

```cpp
UGOKUPadController UGOKUPad;
bool isConnected = false;

Servo servo2;
Servo servo3;
```

### Function called on connect
When UGOKU Pad connects, attach the servos to the pins and move them to 90 degrees (center). For a rotation servo, 90 stops the motor.
```cpp
void onConnect() {
  isConnected = true;
  servo2.attach(12);
  servo3.attach(14);
  servo2.write(90);
  servo3.write(90);
}
```

### Function called on disconnect
When UGOKU Pad disconnects, clear the flag, turn off the digital output, and detach the servos.
```cpp
void onDisconnect() {
  isConnected = false;
  digitalWrite(27, LOW);
  servo2.detach();
  servo3.detach();
}
```

### Initialization: `setup()`
- `UGOKUPad.begin("My ESP32")` sets the device name and starts communication.
- Register the connect/disconnect handlers.
- Set pin modes. Pin 26 is input, pin 27 is output.
```cpp
void setup() {
  UGOKUPad.begin("My ESP32");
  UGOKUPad.setConnectionHandlers(onConnect, onDisconnect);
  pinMode(26, INPUT);
  pinMode(27, OUTPUT);
}
```

### Main loop: `loop()`
```cpp
void loop() {
  // Stop outputs when not connected.
  if (!isConnected) return;

  // Update UGOKU Pad data.
  if (!UGOKUPad.update()) return;

  // Digital output from UGOKU Pad.
  digitalWrite(27, UGOKUPad.read(1));

  // Servo control from UGOKU Pad.
  servo2.write(UGOKUPad.read(2));
  servo3.write(UGOKUPad.read(3));

  // Analog read and send to UGOKU Pad.
  uint8_t percent = (analogRead(26) * 100U) / 4095U; // 0-100 from ADC.
  UGOKUPad.write(5, percent); // Send 0-100 value.

  // Small delay to avoid flooding.
  delay(50);
}
```
</details>
<br>

## Detailed specifications
### Packet format and behavior
- Each packet is fixed at 19 bytes.
- Payload is 9 pairs of (channel, value) plus 1 checksum byte.
- Each packet can carry up to 9 channels. To use more than 9, send multiple packets.
- Channel IDs can be 0-254. `0xFF` is reserved to mark "unused".
- Value `0xFF` means "not received / unset", so do not use it as a meaningful value.

### Main functions
- `update()` refreshes received values; keeps the last values if a packet is invalid.
- `read(channel)` gets the latest value; returns `0xFF` if not received.
- `read(channel, fallback)` returns `fallback` if not received.
- `write(channel, value)` sends a (channel, value) pair to the app.
- `setDefaultValue(channel, value)` sets a default until the first reception.
- `setConnectionHandlers(onConnect, onDisconnect)` runs your functions on connect/disconnect.
