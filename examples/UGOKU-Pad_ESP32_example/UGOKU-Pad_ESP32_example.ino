#include <UGOKU-Pad_Controller.h>
#include <ESP32Servo.h>

UGOKUPadController UGOKUPad;
bool isConnected = false;

Servo servo2;
Servo servo3;

void onConnect() {
  isConnected = true;
  servo2.attach(12);
  servo3.attach(14);
  servo2.write(90);
  servo3.write(90);
}

void onDisconnect() {
  isConnected = false;
  digitalWrite(27, LOW);
  servo2.detach();
  servo3.detach();
}

void setup() {
  UGOKUPad.begin("My ESP32");
  UGOKUPad.setConnectionHandlers(onConnect, onDisconnect);
  pinMode(26, INPUT);
  pinMode(27, OUTPUT);
}

void loop() {
  //Limit loop rate
  static unsigned long last = 0;
  if (millis() - last < 50) return;
  last = millis();

  //Disconnected: stop outputs
  if (!isConnected) return;

  //Update UGOKU Pad data
  if (!UGOKUPad.update()) return;
  
  //Digital output from UGOKU Pad
  digitalWrite(27, UGOKUPad.read(1)); 
  
  //Servo control from UGOKU Pad
  servo2.write(UGOKUPad.read(2)); 
  servo3.write(UGOKUPad.read(3));
  
  //Analog read and send to UGOKU Pad
  uint8_t percent = (analogRead(26) * 100U) / 4095U; // 0-100 from ADC.
  UGOKUPad.write(5, percent); // Send 0-100 value.
}
