#include "UGOKU-Pad_Controller.h"
#include "UGOKU-Pad_ServerCallbacks.h"

UGOKUPadController::UGOKUPadController() {
  for (int i = 0; i < UGOKU_PAD_MAX_CHANNELS; i++) {
    dataArray[i] = 0xFF;
  }
  lastError = UGOKU_PAD_NO_ERROR;
  lastPairs = 0;
  onConnectCallback = nullptr;
  onDisconnectCallback = nullptr;
  pServer = nullptr;
  pService = nullptr;
  pCharacteristic = nullptr;
}

void UGOKUPadController::begin(const char* deviceName) {
  BLEDevice::init(deviceName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new UGOKUPadServerCallbacks(this));

  pService = pServer->createService(UGOKU_PAD_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    UGOKU_PAD_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(UGOKU_PAD_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void UGOKUPadController::setOnConnectCallback(void (*callback)()) {
  onConnectCallback = callback;
}

void UGOKUPadController::setOnDisconnectCallback(void (*callback)()) {
  onDisconnectCallback = callback;
}

uint8_t UGOKUPadController::readPacket() {
  String raw = pCharacteristic->getValue();
  size_t len = raw.length();

  if (len != 19) {
    lastError = UGOKU_PAD_DATA_ERROR;
    lastPairs = 0;
    return lastError;
  }

  uint8_t packet[19];
  for (size_t i = 0; i < 19; i++) {
    packet[i] = static_cast<uint8_t>(raw[i]);
  }

  uint8_t computedCs = 0;
  for (int i = 0; i < 18; i++) {
    computedCs ^= packet[i];
  }

  uint8_t receivedCs = packet[18];
  if (computedCs != receivedCs) {
    lastError = UGOKU_PAD_CS_ERROR;
    lastPairs = 0;
    return lastError;
  }

  lastPairs = 0;
  for (int i = 0; i < 9; i++) {
    uint8_t ch = packet[2 * i];
    uint8_t val = packet[2 * i + 1];
    if (ch < UGOKU_PAD_MAX_CHANNELS) {
      dataArray[ch] = val;
      lastPairs++;
    }
  }

  lastError = UGOKU_PAD_NO_ERROR;
  return lastError;
}

void UGOKUPadController::writePacket(const uint8_t channels[9], const uint8_t values[9]) {
  uint8_t packet[19];

  for (int i = 0; i < 9; i++) {
    packet[2 * i] = channels[i];
    packet[2 * i + 1] = values[i];
  }

  uint8_t cs = 0;
  for (int i = 0; i < 18; i++) {
    cs ^= packet[i];
  }
  packet[18] = cs;

  pCharacteristic->setValue(packet, 19);
  pCharacteristic->notify();
}

void UGOKUPadController::writeChannel(uint8_t channel, uint8_t value) {
  uint8_t chArr[9];
  uint8_t valArr[9];

  for (int i = 0; i < 9; i++) {
    chArr[i] = 0xFF;
    valArr[i] = 0;
  }

  chArr[0] = channel;
  valArr[0] = value;
  writePacket(chArr, valArr);
}

uint8_t UGOKUPadController::valueForChannel(uint8_t channel) const {
  if (channel < UGOKU_PAD_MAX_CHANNELS) {
    return dataArray[channel];
  }
  return 0xFF;
}

uint8_t UGOKUPadController::lastPairsCount() const {
  return lastPairs;
}
