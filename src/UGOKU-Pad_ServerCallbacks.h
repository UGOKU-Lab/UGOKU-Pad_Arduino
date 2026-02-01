#pragma once

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>


class UGOKUPadController;

class UGOKUPadServerCallbacks : public BLEServerCallbacks {
public:
  explicit UGOKUPadServerCallbacks(UGOKUPadController *ctrl);
  void onConnect(BLEServer *pServer) override;
  void onDisconnect(BLEServer *pServer) override;

private:
  UGOKUPadController *controller;
};

class UGOKUPadCharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
  explicit UGOKUPadCharacteristicCallbacks(UGOKUPadController *ctrl);
  void onWrite(BLECharacteristic *pCharacteristic) override;

private:
  UGOKUPadController *controller;
};
