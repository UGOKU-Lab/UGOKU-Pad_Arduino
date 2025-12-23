#include "UGOKU-Pad_Controller.h"
#include "UGOKU-Pad_ServerCallbacks.h"

UGOKUPadController::UGOKUPadController() {
  for (int i = 0; i < UGOKU_PAD_MAX_CHANNELS; i++) {
    dataArray[i] = 0xFF;
    cachedArray[i] = 0xFF;
    hasCached[i] = false;
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

void UGOKUPadController::setConnectionHandlers(void (*onConnect)(), void (*onDisconnect)()) {
  setOnConnectCallback(onConnect);
  setOnDisconnectCallback(onDisconnect);
}

void UGOKUPadController::resetState() {
  for (int i = 0; i < UGOKU_PAD_MAX_CHANNELS; i++) {
    dataArray[i] = 0xFF;
    cachedArray[i] = 0xFF;
    hasCached[i] = false;
  }
  lastError = UGOKU_PAD_NO_ERROR;
  lastPairs = 0;
  if (pCharacteristic) {
    pCharacteristic->setValue("");
  }
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
      if (val != 0xFF) {
        cachedArray[ch] = val;
        hasCached[ch] = true;
      }
      lastPairs++;
    }
  }

  lastError = UGOKU_PAD_NO_ERROR;
  return lastError;
}

bool UGOKUPadController::readPacketCached() {
  uint8_t dataBackup[UGOKU_PAD_MAX_CHANNELS];
  uint8_t cachedBackup[UGOKU_PAD_MAX_CHANNELS];
  bool hasBackup[UGOKU_PAD_MAX_CHANNELS];

  for (int i = 0; i < UGOKU_PAD_MAX_CHANNELS; i++) {
    dataBackup[i] = dataArray[i];
    cachedBackup[i] = cachedArray[i];
    hasBackup[i] = hasCached[i];
  }

  uint8_t err = readPacket();
  if (err == UGOKU_PAD_NO_ERROR) {
    return true;
  }

  for (int i = 0; i < UGOKU_PAD_MAX_CHANNELS; i++) {
    dataArray[i] = dataBackup[i];
    cachedArray[i] = cachedBackup[i];
    hasCached[i] = hasBackup[i];
  }
  return false;
}

bool UGOKUPadController::update() {
  return readPacketCached();
}

uint8_t UGOKUPadController::read(uint8_t channel) const {
  return valueForChannelCached(channel, 0xFF);
}

uint8_t UGOKUPadController::read(uint8_t channel, uint8_t fallback) const {
  return valueForChannelCached(channel, fallback);
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

void UGOKUPadController::write(uint8_t channel, uint8_t value) {
  writeChannel(channel, value);
}

uint8_t UGOKUPadController::valueForChannel(uint8_t channel) const {
  if (channel < UGOKU_PAD_MAX_CHANNELS) {
    return dataArray[channel];
  }
  return 0xFF;
}

uint8_t UGOKUPadController::valueOr(uint8_t channel, uint8_t fallback) const {
  uint8_t v = valueForChannel(channel);
  return (v == 0xFF) ? fallback : v;
}

uint8_t UGOKUPadController::valueForChannelCached(uint8_t channel, uint8_t fallback) const {
  if (channel < UGOKU_PAD_MAX_CHANNELS && hasCached[channel]) {
    return cachedArray[channel];
  }
  return fallback;
}

void UGOKUPadController::setDefaultValue(uint8_t channel, uint8_t value) {
  if (channel < UGOKU_PAD_MAX_CHANNELS) {
    cachedArray[channel] = value;
    hasCached[channel] = true;
  }
}

uint8_t UGOKUPadController::lastPairsCount() const {
  return lastPairs;
}
