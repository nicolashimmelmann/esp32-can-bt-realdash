#include "frame_utils.h"

namespace frame_utils {
void writeUInt16(uint8_t* data, size_t offset, uint16_t value) {
  data[offset] = static_cast<uint8_t>(value & 0xFF);
  data[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void writeInt16(uint8_t* data, size_t offset, int16_t value) {
  writeUInt16(data, offset, static_cast<uint16_t>(value));
}
}
