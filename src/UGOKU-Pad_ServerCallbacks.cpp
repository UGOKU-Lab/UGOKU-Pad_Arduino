#include "UGOKU-Pad_ServerCallbacks.h"
#include "UGOKU-Pad_Controller.h"
#include <BLEDevice.h>
#include <cstring>
#include <string>


UGOKUPadServerCallbacks::UGOKUPadServerCallbacks(UGOKUPadController *ctrl) {
  controller = ctrl;
}

void UGOKUPadServerCallbacks::onConnect(BLEServer *pServer) {
  if (controller->onConnectCallback) {
    controller->onConnectCallback();
  }
}

void UGOKUPadServerCallbacks::onDisconnect(BLEServer *pServer) {
  controller->resetState();
  if (controller->onDisconnectCallback) {
    controller->onDisconnectCallback();
  }
  BLEDevice::startAdvertising();
}

UGOKUPadCharacteristicCallbacks::UGOKUPadCharacteristicCallbacks(
    UGOKUPadController *ctrl) {
  controller = ctrl;
}

void UGOKUPadCharacteristicCallbacks::onWrite(
    BLECharacteristic *pCharacteristic) {
  // Use Arduino String for BLE characteristic value
  String rxValue = pCharacteristic->getValue();

  if (rxValue.length() == 19) {
    // Direct access via friend declaration
    uint8_t *buff = controller->rxBuffer;
    const char *data = rxValue.c_str();
    for (int i = 0; i < 19; i++) {
      buff[i] = (uint8_t)data[i];
    }
    controller->rxBufferValid = true;
  }
}
