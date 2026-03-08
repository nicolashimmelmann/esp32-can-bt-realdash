#pragma once

#include <Arduino.h>
#include <BluetoothSerial.h>

class BluetoothLink {
 public:
  bool begin(const char* deviceName);
  size_t write(const uint8_t* data, size_t length);
  bool isConnected();

 private:
  BluetoothSerial serialBt_;
};
