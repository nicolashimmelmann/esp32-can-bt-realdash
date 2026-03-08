#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <mcp2515.h>

struct CanFrame {
  uint32_t id;
  uint8_t dlc;
  bool isExtended;
  uint8_t data[8];
};

class CanBus {
 public:
  CanBus();

  bool begin();
  bool readFrame(CanFrame& frame);

 private:
  SPIClass spi_;
  MCP2515 mcp2515_;

  bool configureBitrate();
};
