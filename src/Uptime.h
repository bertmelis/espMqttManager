/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uintx_t

#if !defined(ARDUINO_ARCH_ESP32) || !defined(ARDUINO_ARCH_ESP8266)
#error Platform not supported
#endif

#include <Arduino.h>  // millis(), Update

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (SECS_PER_MIN * 60UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) ((_time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) (_time_ / SECS_PER_DAY)

namespace espMqttManagerHelpers {

class Uptime {
 public:
  Uptime();
  uint64_t getUptime();
  char* getUptimeStr();

 private:
  char _uptimeStr[18];
  uint64_t _uptime;
  uint32_t _lastMillis;
};

extern Uptime uptime;

}  // end namespace espMqttManagerHelpers
