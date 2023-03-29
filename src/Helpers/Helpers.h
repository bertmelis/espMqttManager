/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uintx_t

#include <Arduino.h>  // millis()
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>  //  WiFi.RSSI()
#include <Updater.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <Update.h>
#else
#error Platform not supported
#endif

namespace espMqttManagerHelpers {

uint8_t signalQuality();
void handleUpdate(const uint8_t* payload, size_t length, size_t index, size_t total);
extern bool updated;

}  // end namespace espMqttManagerHelpers
