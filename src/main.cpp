#include <Arduino.h>

#include <cstring>

#include <cstdio>

#include "bluetooth_link.h"
#include "can_bus.h"
#include "config.h"
#include "realdash_protocol.h"

namespace {
BluetoothLink bluetoothLink;
CanBus canBus;
uint8_t transportBuffer[realdash::kTransportFrameSize];

bool shouldForwardFrame(const CanFrame& frame) {
  if (frame.isExtended) {
    return false;
  }

  return frame.id >= config::kForwardedFrameMinId && frame.id <= config::kForwardedFrameMaxId;
}

void writeUInt16(uint8_t* data, size_t offset, uint16_t value) {
  data[offset] = static_cast<uint8_t>(value & 0xFF);
  data[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void writeInt16(uint8_t* data, size_t offset, int16_t value) {
  writeUInt16(data, offset, static_cast<uint16_t>(value));
}

void sendVirtualFrame(uint32_t frameId, const uint8_t* payload) {
  realdash::VirtualFrame frame{};
  frame.id = frameId;
  std::memcpy(frame.data, payload, sizeof(frame.data));

  const size_t encodedLength = realdash::encodeVirtual44Frame(frame, transportBuffer, sizeof(transportBuffer));
  if (encodedLength > 0 && bluetoothLink.isConnected()) {
    bluetoothLink.write(transportBuffer, encodedLength);
  }
}

void sendTestFrames() {
  static uint32_t lastSendAt = 0;
  const uint32_t now = millis();
  if (now - lastSendAt < config::kRealDashTestIntervalMs) {
    return;
  }

  lastSendAt = now;

  const float phase = static_cast<float>(now % 10000UL) / 1000.0F;
  const uint16_t rpm = 900 + static_cast<uint16_t>(400.0F + 2500.0F * (sin(phase) + 1.0F) * 0.5F);
  const uint16_t speedScaled = static_cast<uint16_t>((40.0F + 40.0F * (sin(phase * 0.7F) + 1.0F) * 0.5F) * 100.0F);
  const int16_t coolantScaled = static_cast<int16_t>((82.0F + 6.0F * sin(phase * 0.3F)) * 10.0F);
  const uint16_t batteryScaled = static_cast<uint16_t>((13.8F + 0.2F * sin(phase * 1.1F)) * 100.0F);
  const uint16_t throttleScaled = static_cast<uint16_t>((12.0F + 65.0F * (sin(phase * 1.4F) + 1.0F) * 0.5F) * 10.0F);
  const int16_t intakeScaled = static_cast<int16_t>((28.0F + 4.0F * sin(phase * 0.5F)) * 10.0F);
  const uint16_t mapScaled = static_cast<uint16_t>((42.0F + 25.0F * (sin(phase * 1.2F) + 1.0F) * 0.5F) * 10.0F);
  const uint16_t fuelScaled = static_cast<uint16_t>((58.0F + 2.0F * sin(phase * 0.15F)) * 10.0F);
  const int16_t oilTempScaled = static_cast<int16_t>((92.0F + 5.0F * sin(phase * 0.4F)) * 10.0F);
  const uint16_t oilPressureScaled = static_cast<uint16_t>((2.4F + 1.3F * (sin(phase * 1.0F) + 1.0F) * 0.5F) * 100.0F);
  const int16_t boostScaled = static_cast<int16_t>((-0.15F + 0.95F * sin(phase * 1.3F)) * 100.0F);

  uint8_t frame500[8] = {0};
  writeUInt16(frame500, 0, rpm);
  writeUInt16(frame500, 2, speedScaled);
  writeInt16(frame500, 4, coolantScaled);
  writeUInt16(frame500, 6, batteryScaled);
  sendVirtualFrame(0x500, frame500);

  uint8_t frame501[8] = {0};
  writeUInt16(frame501, 0, throttleScaled);
  writeInt16(frame501, 2, intakeScaled);
  writeUInt16(frame501, 4, mapScaled);
  writeUInt16(frame501, 6, fuelScaled);
  sendVirtualFrame(0x501, frame501);

  uint8_t frame502[8] = {0};
  writeInt16(frame502, 0, oilTempScaled);
  writeUInt16(frame502, 2, oilPressureScaled);
  writeInt16(frame502, 4, boostScaled);
  frame502[6] = 1;
  sendVirtualFrame(0x502, frame502);

  uint8_t frame503[8] = {0};
  if (static_cast<uint32_t>(phase) % 2 == 0) {
    frame503[0] |= 1 << 1;
  }
  if (static_cast<uint32_t>(phase * 2.0F) % 2 == 0) {
    frame503[0] |= 1 << 2;
  } else {
    frame503[0] |= 1 << 3;
  }
  if (rpm > 3200) {
    frame503[0] |= 1 << 0;
  }
  sendVirtualFrame(0x503, frame503);
}

void logCanFrame(const CanFrame& frame) {
  static uint32_t lastPrintAt = 0;
  const uint32_t now = millis();
  if (now - lastPrintAt < config::kSnifferPrintIntervalMs) {
    return;
  }

  lastPrintAt = now;

  char payload[3 * 8 + 1] = {0};
  size_t cursor = 0;
  for (uint8_t index = 0; index < frame.dlc && index < 8; ++index) {
    const int written = std::snprintf(payload + cursor, sizeof(payload) - cursor, "%02X%s", frame.data[index], index + 1 < frame.dlc ? " " : "");
    if (written <= 0) {
      break;
    }
    cursor += static_cast<size_t>(written);
    if (cursor >= sizeof(payload)) {
      break;
    }
  }

  Serial.printf("[CAN] id=0x%03lX dlc=%u data=%s\n", static_cast<unsigned long>(frame.id), frame.dlc, payload);
}

void runCanSniffer() {
  CanFrame frame;
  if (!canBus.readFrame(frame)) {
    return;
  }

  logCanFrame(frame);

  if (!shouldForwardFrame(frame)) {
    return;
  }

  const size_t encodedLength = realdash::encode44Frame(frame, transportBuffer, sizeof(transportBuffer));
  if (encodedLength > 0 && bluetoothLink.isConnected()) {
    bluetoothLink.write(transportBuffer, encodedLength);
  }
}
}

void setup() {
  Serial.begin(config::kSerialBaudRate);
  delay(250);

  const bool bluetoothStarted = bluetoothLink.begin(config::kBluetoothDeviceName);
  Serial.printf("[BT] start: %s\n", bluetoothStarted ? "ok" : "failed");

  const bool canStarted = canBus.begin();
  Serial.printf("[CAN] start: %s\n", canStarted ? "ok" : "failed");
  Serial.printf("[MODE] %s\n", config::kRunMode == config::RunMode::kCanSniffer ? "can-sniffer" : "realdash-testframes");
}

void loop() {
  if (config::kRunMode == config::RunMode::kCanSniffer) {
    runCanSniffer();
  } else {
    sendTestFrames();
  }
  delay(config::kLoopDelayMs);
}
