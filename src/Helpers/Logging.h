/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(ARDUINO_ARCH_ESP32)
  #include <esp32-hal-log.h>
  #if defined(DEBUG_ESP_MQTT_MANAGER)
  // Logging is en/disabled by Arduino framework macros
    extern portMUX_TYPE loggingMutex;
    #define emm_log_i(...) vPortEnterCritical(&loggingMutex); log_i(__VA_ARGS__); vPortExitCritical(&loggingMutex)
    #define emm_log_e(...) vPortEnterCritical(&loggingMutex); log_e(__VA_ARGS__); vPortExitCritical(&loggingMutex)
    #define emm_log_w(...) vPortEnterCritical(&loggingMutex); log_w(__VA_ARGS__); vPortExitCritical(&loggingMutex)
  #else
  // Logging is disabled
    #define emm_log_i(...)
    #define emm_log_e(...)
    #define emm_log_w(...)
  #endif
#elif defined(ARDUINO_ARCH_ESP8266)
  #if defined(DEBUG_ESP_PORT) && defined(DEBUG_ESP_MQTT_MANAGER)
    #include <Arduino.h>
    #define emm_log_i(...) DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.print("\n")
    #define emm_log_e(...) DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.print("\n")
    #define emm_log_w(...) DEBUG_ESP_PORT.printf(__VA_ARGS__); DEBUG_ESP_PORT.print("\n")
  #else
    #define emm_log_i(...)
    #define emm_log_e(...)
    #define emm_log_w(...)
  #endif
#else
  // when building for PC, always show debug statements as part of testing suite
  #include <iostream>
  #define emm_log_i(...) std::cout << "[I] " << __FILE__ ":" << __LINE__ << ": "; printf(__VA_ARGS__); std::cout << std::endl
  #define emm_log_e(...) std::cout << "[E] " << __FILE__ ":" << __LINE__ << ": "; printf(__VA_ARGS__); std::cout << std::endl
  #define emm_log_w(...) std::cout << "[W] " << __FILE__ ":" << __LINE__ << ": "; printf(__VA_ARGS__); std::cout << std::endl
#endif
