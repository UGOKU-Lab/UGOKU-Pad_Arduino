#include <UGOKU-Pad_Controller.h>
#include <ESP32Servo.h>

UGOKUPadController controller;

// Pin definitions
#define PIN_SERVO_1 12
#define PIN_SERVO_2 14
#define PIN_ANALOG_READ 26
#define PIN_LED 27

Servo servo1;
Servo servo2;

bool isConnected = false;

// Default values (0xFF means "not received yet")
uint8_t stick_2 = 90;
uint8_t stick_3 = 90;
uint8_t btn_1 = 0xFF;
uint8_t prev_btn_1 = 0xFF;

static inline void updateFromChannel(uint8_t ch, uint8_t& var) {
  uint8_t v = controller.valueForChannel(ch);
  if (v != 0xFF && v != var) var = v;
}

void setup() {
  Serial.begin(115200);
  controller.begin("UGOKU-Pad ESP32");
  controller.setOnConnectCallback(onDeviceConnect);
  controller.setOnDisconnectCallback(onDeviceDisconnect);

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  Serial.println("Waiting for a device to connect...");
}

void onDeviceConnect() {
  Serial.println("Device connected!");
  isConnected = true;
  servo1.attach(PIN_SERVO_1, 500, 2500);
  servo2.attach(PIN_SERVO_2, 500, 2500);
  digitalWrite(PIN_LED, HIGH);
}

void onDeviceDisconnect() {
  Serial.println("Device disconnected!");
  isConnected = false;
  servo1.detach();
  servo2.detach();
  digitalWrite(PIN_LED, LOW);
}

void loop() {
  if (!isConnected) {
    delay(50);
    return;
  }

  uint8_t err = controller.readPacket();
  if (err == UGOKU_PAD_NO_ERROR) {
    if (controller.lastPairsCount() > 0) {
      const uint8_t channels[] = {1, 2, 3};
      uint8_t* targets[] = {&btn_1, &stick_2, &stick_3};
      for (uint8_t i = 0; i < sizeof(channels) / sizeof(channels[0]); ++i) {
        updateFromChannel(channels[i], *targets[i]);
      }

      if (btn_1 != 0xFF && btn_1 != prev_btn_1) {
        prev_btn_1 = btn_1;
        digitalWrite(PIN_LED, (btn_1 == 1) ? LOW : HIGH);
      }
    }
  } else if (err == UGOKU_PAD_CS_ERROR) {
    Serial.println("Checksum error on incoming packet");
  } else if (err == UGOKU_PAD_DATA_ERROR) {
    Serial.println("Incoming packet length != 19");
  }

#if 0
  servo1.write(stick_2);
  servo2.write(stick_3);
#endif

#if 1
  servo1.write(stick_2 + stick_3 - 90);
  servo2.write(stick_2 - stick_3 + 90);
#endif

  int psd = analogRead(PIN_ANALOG_READ);
  float dist = 1 / (float)psd * 30000;
  int dist_int = (int)dist;
  controller.writeChannel(5, dist_int);

  delay(50);
}
