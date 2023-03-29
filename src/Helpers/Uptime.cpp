/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Uptime.h"

namespace espMqttManagerHelpers {

Uptime::Uptime()
: _uptimeStr{'\0'}
, _uptime(0)
, _lastMillis(0) {
  // empty
}

uint64_t Uptime::getUptime() {  // Call at least every 50 days to avoid overflow ;-)
  uint32_t currMillis = millis();
  _uptime += (currMillis - _lastMillis);
  _lastMillis = currMillis;
  return _uptime;
}

char* Uptime::getUptimeStr() {
  getUptime();
  uint64_t ut = _uptime / 1000;
  uint8_t days = elapsedDays(ut);
  uint8_t hours = numberOfHours(ut);
  uint8_t minutes = numberOfMinutes(ut);
  uint8_t seconds = numberOfSeconds(ut);
  snprintf(_uptimeStr, sizeof(_uptimeStr), "%d Days %02d:%02d:%02d", days, hours, minutes, seconds);
  return _uptimeStr;
}

}  // end namespace espMqttManagerHelpers
