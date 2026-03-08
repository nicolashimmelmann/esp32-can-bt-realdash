#include "realdash_protocol.h"

#include <cstring>

namespace realdash {
namespace {
size_t encodeFrame(uint32_t frameId, const uint8_t* payload, size_t payloadLength, uint8_t* outBuffer, size_t outBufferSize) {
  if (payload == nullptr || outBuffer == nullptr || outBufferSize < kTransportFrameSize) {
    return 0;
  }

  std::memcpy(outBuffer, kFrameHeader, sizeof(kFrameHeader));
  outBuffer[4] = static_cast<uint8_t>(frameId & 0xFF);
  outBuffer[5] = static_cast<uint8_t>((frameId >> 8) & 0xFF);
  outBuffer[6] = static_cast<uint8_t>((frameId >> 16) & 0xFF);
  outBuffer[7] = static_cast<uint8_t>((frameId >> 24) & 0xFF);

  std::memset(outBuffer + 8, 0, 8);
  const size_t limitedPayloadLength = payloadLength > 8 ? 8 : payloadLength;
  std::memcpy(outBuffer + 8, payload, limitedPayloadLength);

  return kTransportFrameSize;
}
}

size_t encode44Frame(const CanFrame& sourceFrame, uint8_t* outBuffer, size_t outBufferSize) {
  return encodeFrame(sourceFrame.id, sourceFrame.data, sourceFrame.dlc, outBuffer, outBufferSize);
}

size_t encodeVirtual44Frame(const VirtualFrame& sourceFrame, uint8_t* outBuffer, size_t outBufferSize) {
  return encodeFrame(sourceFrame.id, sourceFrame.data, sizeof(sourceFrame.data), outBuffer, outBufferSize);
}
}
