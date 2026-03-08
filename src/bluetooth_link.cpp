#include "bluetooth_link.h"

bool BluetoothLink::begin(const char* deviceName) {
  return serialBt_.begin(deviceName);
}

size_t BluetoothLink::write(const uint8_t* data, size_t length) {
  return serialBt_.write(data, length);
}

bool BluetoothLink::isConnected() {
  return serialBt_.hasClient();
}
