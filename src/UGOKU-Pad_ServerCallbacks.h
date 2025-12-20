#pragma once

#include <BLEServer.h>

class UGOKUPadController;

class UGOKUPadServerCallbacks : public BLEServerCallbacks {
  public:
    explicit UGOKUPadServerCallbacks(UGOKUPadController* ctrl);
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;

  private:
    UGOKUPadController* controller;
};
