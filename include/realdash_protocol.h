#pragma once

#include <Arduino.h>

#include <cstddef>

#include "can_bus.h"

namespace realdash {
constexpr uint8_t kFrameHeader[4] = {0x44, 0x33, 0x22, 0x11};
constexpr size_t kTransportFrameSize = 16;

struct VirtualFrame {
  uint32_t id;
  uint8_t data[8];
};

size_t encode44Frame(const CanFrame& sourceFrame, uint8_t* outBuffer, size_t outBufferSize);
size_t encodeVirtual44Frame(const VirtualFrame& sourceFrame, uint8_t* outBuffer, size_t outBufferSize);
}
