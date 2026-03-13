#pragma once

#include <Arduino.h>

namespace config {
enum class RunMode {
  kNormalOperation,
  kRealDashTestFrames,
  kCanSniffer,
};

constexpr char kBluetoothDeviceName[] = "ESP32-RealDash";
constexpr uint32_t kSerialBaudRate = 115200;

constexpr int kSpiSckPin = 18;
constexpr int kSpiMisoPin = 19;
constexpr int kSpiMosiPin = 23;
constexpr int kCanChipSelectPin = 5;
constexpr int kCanInterruptPin = 4;

constexpr uint32_t kCanSpeed = 500000;
constexpr uint8_t kCanClockMHz = 8;

constexpr RunMode kRunMode = RunMode::kNormalOperation;
constexpr uint32_t kForwardedFrameIds[] = {
    0x500,
    0x501,
    0x502,
    0x503,
    0x504,
    0x505,
};
constexpr uint32_t kSnifferPrintIntervalMs = 100;
constexpr uint32_t kRealDashTestIntervalMs = 100;
constexpr uint32_t kLoopDelayMs = 5;
}
