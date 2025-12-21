#include <UGOKU-Pad_Controller.h>
#include <M5StickCPlus2.h>

UGOKUPadController controller;

bool isConnected = false;

static constexpr uint8_t kPadChannelCount = 10;
static constexpr uint8_t kInvalidValue = 0xFF;

static constexpr uint32_t kDisplayIntervalMs = 200;
static constexpr uint32_t kSensorIntervalMs = 200;

static constexpr int kHeaderHeight = 18;
static constexpr int kLineHeight = 18;
static constexpr int kLeftColumnX = 0;
static constexpr int kRightColumnX = 120;

enum SensorIndex : uint8_t {
  kIdxAccelX = 0,
  kIdxAccelY,
  kIdxAccelZ,
  kIdxGyroX,
  kIdxGyroY,
  kIdxGyroZ,
  kIdxImuTemp,
  kIdxBatteryLevel,
  kIdxBatteryVoltage,
  kIdxVbusVoltage,
  kIdxBatteryCurrent,
  kIdxChargeState,
  kIdxMicLevel,
  kIdxRtcHour,
  kIdxRtcMinute,
  kIdxRtcSecond,
  kSensorCount,
};

static constexpr uint8_t kSensorChannels[kSensorCount] = {
  10, // Accel X
  11, // Accel Y
  12, // Accel Z
  13, // Gyro X
  14, // Gyro Y
  15, // Gyro Z
  16, // IMU Temp
  17, // Battery Level
  18, // Battery Voltage
  19, // VBUS Voltage
  20, // Battery Current
  21, // Charging State
  22, // Mic Level
  23, // RTC Hour
  24, // RTC Minute
  25, // RTC Second
};

static constexpr size_t kMicSampleCount = 128;

uint8_t padValues[kPadChannelCount];
uint8_t sensorValues[kSensorCount];
int16_t micSamples[kMicSampleCount];

uint32_t lastDisplayMs = 0;
uint32_t lastSensorMs = 0;

static inline uint8_t clampToByte(int32_t value) {
  if (value < 0) return 0;
  if (value > 255) return 255;
  return static_cast<uint8_t>(value);
}

static uint8_t scaleFloatToByte(float value, float minValue, float maxValue) {
  if (minValue >= maxValue) return 0;
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;
  float ratio = (value - minValue) / (maxValue - minValue);
  int32_t scaled = static_cast<int32_t>(ratio * 255.0f + 0.5f);
  return clampToByte(scaled);
}

static uint8_t scaleIntToByte(int32_t value, int32_t minValue, int32_t maxValue) {
  if (minValue >= maxValue) return 0;
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;
  int32_t scaled = ((value - minValue) * 255L) / (maxValue - minValue);
  return clampToByte(scaled);
}

static inline void updateFromChannel(uint8_t ch, uint8_t& var) {
  uint8_t v = controller.valueForChannel(ch);
  if (v != kInvalidValue && v != var) var = v;
}

void drawPadValues() {
  auto& lcd = StickCP2.Display;
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  lcd.setCursor(0, 0);
  lcd.print(isConnected ? "UGOKU-Pad OK" : "UGOKU-Pad WAIT");

  for (uint8_t i = 0; i < kPadChannelCount; ++i) {
    const bool right = (i >= 5);
    const uint8_t row = i % 5;
    const int x = right ? kRightColumnX : kLeftColumnX;
    const int y = kHeaderHeight + row * kLineHeight;

    char buf[4] = "---";
    if (padValues[i] != kInvalidValue) {
      snprintf(buf, sizeof(buf), "%3u", padValues[i]);
    }

    lcd.setCursor(x, y);
    lcd.printf("ch%u:%s", i, buf);
  }
}

uint8_t readMicLevel() {
  if (!StickCP2.Mic.isEnabled()) return kInvalidValue;
  if (!StickCP2.Mic.record(micSamples, kMicSampleCount)) return kInvalidValue;

  uint32_t sumAbs = 0;
  for (size_t i = 0; i < kMicSampleCount; ++i) {
    int32_t sample = micSamples[i];
    if (sample < 0) sample = -sample;
    sumAbs += static_cast<uint32_t>(sample);
  }

  uint32_t avgAbs = sumAbs / kMicSampleCount;
  return scaleIntToByte(static_cast<int32_t>(avgAbs), 0, 32767);
}

void updateSensorValues() {
  for (uint8_t i = 0; i < kSensorCount; ++i) {
    sensorValues[i] = kInvalidValue;
  }

  if (StickCP2.Imu.isEnabled()) {
    float ax = 0.0f;
    float ay = 0.0f;
    float az = 0.0f;
    float gx = 0.0f;
    float gy = 0.0f;
    float gz = 0.0f;
    float temp = 0.0f;

    StickCP2.Imu.getAccel(&ax, &ay, &az);
    StickCP2.Imu.getGyro(&gx, &gy, &gz);
    StickCP2.Imu.getTemp(&temp);

    sensorValues[kIdxAccelX] = scaleFloatToByte(ax, -4.0f, 4.0f);
    sensorValues[kIdxAccelY] = scaleFloatToByte(ay, -4.0f, 4.0f);
    sensorValues[kIdxAccelZ] = scaleFloatToByte(az, -4.0f, 4.0f);
    sensorValues[kIdxGyroX] = scaleFloatToByte(gx, -500.0f, 500.0f);
    sensorValues[kIdxGyroY] = scaleFloatToByte(gy, -500.0f, 500.0f);
    sensorValues[kIdxGyroZ] = scaleFloatToByte(gz, -500.0f, 500.0f);
    sensorValues[kIdxImuTemp] = scaleFloatToByte(temp, 0.0f, 60.0f);
  }

  int32_t battLevel = StickCP2.Power.getBatteryLevel();
  if (battLevel >= 0) {
    sensorValues[kIdxBatteryLevel] = clampToByte(battLevel);
  }

  int32_t battMv = StickCP2.Power.getBatteryVoltage();
  sensorValues[kIdxBatteryVoltage] = scaleIntToByte(battMv, 3000, 4200);

  int32_t vbusMv = StickCP2.Power.getVBUSVoltage();
  if (vbusMv >= 0) {
    sensorValues[kIdxVbusVoltage] = scaleIntToByte(vbusMv, 4000, 5200);
  }

  int32_t battMa = StickCP2.Power.getBatteryCurrent();
  sensorValues[kIdxBatteryCurrent] = scaleIntToByte(battMa, -500, 500);

  sensorValues[kIdxChargeState] = clampToByte(StickCP2.Power.isCharging());
  sensorValues[kIdxMicLevel] = readMicLevel();

  if (StickCP2.Rtc.isEnabled()) {
    m5::rtc_time_t time = StickCP2.Rtc.getTime();
    if (time.hours >= 0) sensorValues[kIdxRtcHour] = clampToByte(time.hours);
    if (time.minutes >= 0) sensorValues[kIdxRtcMinute] = clampToByte(time.minutes);
    if (time.seconds >= 0) sensorValues[kIdxRtcSecond] = clampToByte(time.seconds);
  }
}

void sendSensorPacket(size_t offset) {
  uint8_t channels[9];
  uint8_t values[9];

  for (uint8_t i = 0; i < 9; ++i) {
    size_t idx = offset + i;
    if (idx < kSensorCount) {
      channels[i] = kSensorChannels[idx];
      values[i] = sensorValues[idx];
    } else {
      channels[i] = kInvalidValue;
      values[i] = 0;
    }
  }

  controller.writePacket(channels, values);
}

void sendAllSensorValues() {
  updateSensorValues();
  sendSensorPacket(0);
  if (kSensorCount > 9) {
    sendSensorPacket(9);
  }
}

void setup() {
  Serial.begin(115200);

  StickCP2.begin();
  StickCP2.Display.setRotation(1);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  StickCP2.Display.fillScreen(TFT_BLACK);

  for (uint8_t i = 0; i < kPadChannelCount; ++i) {
    padValues[i] = kInvalidValue;
  }

  controller.begin("UGOKU-Pad M5StickCPlus2");
  controller.setOnConnectCallback(onDeviceConnect);
  controller.setOnDisconnectCallback(onDeviceDisconnect);

  Serial.println("Waiting for a device to connect...");
  drawPadValues();
}

void onDeviceConnect() {
  Serial.println("Device connected!");
  isConnected = true;
}

void onDeviceDisconnect() {
  Serial.println("Device disconnected!");
  isConnected = false;
}

void loop() {
  StickCP2.update();

  if (isConnected) {
    uint8_t err = controller.readPacket();
    if (err == UGOKU_PAD_NO_ERROR) {
      if (controller.lastPairsCount() > 0) {
        for (uint8_t i = 0; i < kPadChannelCount; ++i) {
          updateFromChannel(i, padValues[i]);
        }
      }
    } else if (err == UGOKU_PAD_CS_ERROR) {
      Serial.println("Checksum error on incoming packet");
    } else if (err == UGOKU_PAD_DATA_ERROR) {
      Serial.println("Incoming packet length != 19");
    }
  }

  uint32_t now = millis();
  if (now - lastDisplayMs >= kDisplayIntervalMs) {
    lastDisplayMs = now;
    drawPadValues();
  }

  if (isConnected && (now - lastSensorMs >= kSensorIntervalMs)) {
    lastSensorMs = now;
    sendAllSensorValues();
  }

  delay(10);
}
