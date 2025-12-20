#include "UGOKU-Pad_ServerCallbacks.h"
#include "UGOKU-Pad_Controller.h"
#include <BLEDevice.h>

UGOKUPadServerCallbacks::UGOKUPadServerCallbacks(UGOKUPadController* ctrl) {
  controller = ctrl;
}

void UGOKUPadServerCallbacks::onConnect(BLEServer* pServer) {
  if (controller->onConnectCallback) {
    controller->onConnectCallback();
  }
}

void UGOKUPadServerCallbacks::onDisconnect(BLEServer* pServer) {
  if (controller->onDisconnectCallback) {
    controller->onDisconnectCallback();
  }
  BLEDevice::startAdvertising();
}
