/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Helpers.h"

namespace espMqttManagerHelpers {

uint8_t signalQuality() {
  uint8_t signal = 0;
  int32_t rssi = WiFi.RSSI();
  if (rssi <= -100) {
    signal = 0;
  } else if (rssi >= -50) {
    signal = 100;
  } else {
    signal = 2 * (rssi + 100);
  }
  return signal;
}

bool updated = false;

void handleUpdate(const uint8_t* payload, size_t length, size_t index, size_t total) {
  // The Updater class takes a non-const pointer to write data although it doesn't change the data
  uint8_t* data = const_cast<uint8_t*>(payload);
  static size_t written;
  #if defined(ESP8266)
  Update.runAsync(true);
  #endif
  if (index == 0) {
    if (Update.isRunning()) {
      Update.end();
      Update.clearError();
    }
    written = 0;
    Update.begin(total);
    written += Update.write(data, length);
  } else {
    if (!Update.isRunning()) return;
    written += Update.write(data, length);
  }
  if (Update.isFinished()) {
    if (Update.end()) {
      espMqttManagerHelpers::updated = true;
    } else {
      //Update.printError(Serial);
      Update.clearError();
    }
  }
}

} // end namespace espMqttManagerHelpers
