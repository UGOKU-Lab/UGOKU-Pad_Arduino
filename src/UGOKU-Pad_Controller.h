#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "UGOKU-Pad_Definitions.h"

class UGOKUPadController;
class UGOKUPadServerCallbacks;

class UGOKUPadController {
  public:
    UGOKUPadController();

    // Initialize BLE server, service, and characteristic
    void begin(const char* deviceName);

    // Register callbacks
    void setOnConnectCallback(void (*callback)());
    void setOnDisconnectCallback(void (*callback)());

    // Read exactly 19 bytes from the characteristic and parse pairs
    // Returns UGOKU_PAD_NO_ERROR / UGOKU_PAD_CS_ERROR / UGOKU_PAD_DATA_ERROR
    uint8_t readPacket();

    // Send 9 pairs + checksum
    void writePacket(const uint8_t channels[9], const uint8_t values[9]);

    // Convenience: send a single pair
    void writeChannel(uint8_t channel, uint8_t value);

    // Accessors
    uint8_t valueForChannel(uint8_t channel) const;
    uint8_t lastPairsCount() const;

    void (*onConnectCallback)();
    void (*onDisconnectCallback)();

  private:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pCharacteristic;

    uint8_t dataArray[UGOKU_PAD_MAX_CHANNELS];
    uint8_t lastError;
    uint8_t lastPairs;
};
