#pragma once

#include <stdint.h>

// UUIDs for the BLE service and characteristic
// You can generate new UUIDs using: https://www.uuidgenerator.net/
static constexpr char UGOKU_PAD_SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
static constexpr char UGOKU_PAD_CHARACTERISTIC_UUID[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

// Error codes for packet handling
enum : uint8_t {
    UGOKU_PAD_NO_ERROR   = 0xE0,  // No error
    UGOKU_PAD_CS_ERROR   = 0xE1,  // Checksum error
    UGOKU_PAD_DATA_ERROR = 0xE2   // Packet length was not exactly 19 bytes
};

// Maximum number of channels supported
static constexpr uint8_t UGOKU_PAD_MAX_CHANNELS = 255;
