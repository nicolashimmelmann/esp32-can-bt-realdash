#pragma once

#include <Arduino.h>

namespace frame_utils {
void writeUInt16(uint8_t* data, size_t offset, uint16_t value);
void writeInt16(uint8_t* data, size_t offset, int16_t value);
}
