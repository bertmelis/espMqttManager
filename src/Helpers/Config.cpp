/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Config.h"

#if defined(ARDUINO_ARCH_ESP8266)
#define EMM_FILESYSTEM LittleFS
#elif defined(ARDUINO_ARCH_ESP32)
#define EMM_FILESYSTEM SPIFFS
#endif

namespace espMqttManagerHelpers {

bool Config::getConfig() {
  if (!EMM_FILESYSTEM.begin()) {
     emm_log_e("Error mounting filesystem");
     return false;
  }
  File file = EMM_FILESYSTEM.open(EMM_CONFIG_FILE, EMM_FILE_READ);
  if (!file) {
    emm_log_e("Error opening settings.json");
    return false;
  }
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    emm_log_e("JSON deserialization error: %s", error.c_str());
    return false;
  }
  strlcpy(SSID,
          doc["SSID"] | "",
          sizeof(SSID));
  strlcpy(PSK,
          doc["PSK"] | "",
          sizeof(PSK));
  JsonArray hostIPArray = doc["IPAddress"];
  uint8_t IPAddress_0 = hostIPArray[0] | 0;
  uint8_t IPAddress_1 = hostIPArray[1] | 0;
  uint8_t IPAddress_2 = hostIPArray[2] | 0;
  uint8_t IPAddress_3 = hostIPArray[3] | 0;
  hostIP = IPAddress(IPAddress_0, IPAddress_1, IPAddress_2, IPAddress_3);
  strlcpy(hostname,
          doc["hostname"] | "",
          sizeof(hostname));
  port = doc["port"] | 1883;
  strlcpy(devicename,
          doc["devicename"] | "",
          sizeof(devicename));
  strlcpy(username,
          doc["username"] | "",
          sizeof(username));
  strlcpy(password,
          doc["password"] | "",
          sizeof(password));
  file.close();
  EMM_FILESYSTEM.end();
  return true;
}

}  // end namespace espMqttManagerHelpers
