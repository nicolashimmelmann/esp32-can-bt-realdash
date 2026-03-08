#include "can_bus.h"

#include <cstring>

#include "config.h"

namespace {
CAN_SPEED resolveCanSpeed() {
  switch (config::kCanSpeed) {
    case 5000:
      return CAN_5KBPS;
    case 10000:
      return CAN_10KBPS;
    case 20000:
      return CAN_20KBPS;
    case 31250:
      return CAN_31K25BPS;
    case 33333:
      return CAN_33KBPS;
    case 40000:
      return CAN_40KBPS;
    case 50000:
      return CAN_50KBPS;
    case 80000:
      return CAN_80KBPS;
    case 83333:
      return CAN_83K3BPS;
    case 95000:
      return CAN_95KBPS;
    case 100000:
      return CAN_100KBPS;
    case 125000:
      return CAN_125KBPS;
    case 200000:
      return CAN_200KBPS;
    case 250000:
      return CAN_250KBPS;
    case 500000:
      return CAN_500KBPS;
    case 1000000:
      return CAN_1000KBPS;
    default:
      return CAN_500KBPS;
  }
}

CAN_CLOCK resolveCanClock() {
  switch (config::kCanClockMHz) {
    case 8:
      return MCP_8MHZ;
    case 16:
      return MCP_16MHZ;
    case 20:
      return MCP_20MHZ;
    default:
      return MCP_8MHZ;
  }
}
}

CanBus::CanBus() : spi_(VSPI), mcp2515_(config::kCanChipSelectPin, MCP2515::DEFAULT_SPI_CLOCK, &spi_) {}

bool CanBus::begin() {
  pinMode(config::kCanInterruptPin, INPUT);
  spi_.begin(config::kSpiSckPin, config::kSpiMisoPin, config::kSpiMosiPin, config::kCanChipSelectPin);

  mcp2515_.reset();

  if (!configureBitrate()) {
    return false;
  }

  mcp2515_.setNormalMode();
  return true;
}

bool CanBus::readFrame(CanFrame& frame) {
  struct can_frame rawFrame;
  const auto error = mcp2515_.readMessage(&rawFrame);
  if (error != MCP2515::ERROR_OK) {
    return false;
  }

  frame.isExtended = (rawFrame.can_id & CAN_EFF_FLAG) != 0;
  frame.id = frame.isExtended ? (rawFrame.can_id & CAN_EFF_MASK) : (rawFrame.can_id & CAN_SFF_MASK);
  frame.dlc = rawFrame.can_dlc;
  std::memcpy(frame.data, rawFrame.data, sizeof(frame.data));
  return true;
}

bool CanBus::configureBitrate() {
  return mcp2515_.setBitrate(resolveCanSpeed(), resolveCanClock()) == MCP2515::ERROR_OK;
}
