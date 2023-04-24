/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#ifndef EMM_CONFIG_FILE
#define EMM_CONFIG_FILE "/config.json"
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include <LittleFS.h>
#define EMM_FILE_READ "r"
#elif defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#define EMM_FILE_READ FILE_READ
#endif
#include <IPAddress.h>

#include <ArduinoJson.h>

#include "Logging.h"

namespace espMqttManagerHelpers {

class Config {
 public:
  char SSID[64];
  char PSK[64];
  IPAddress hostIP;
  char hostname[64];
  uint16_t port;
  char devicename[64];
  char username[64];
  char password[64];

  bool getConfig();
};

}  // end namespace espMqttManagerHelpers
