/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_ESP8266)
#error Platform not supported
#endif

#include <Arduino.h>

namespace espMqttManagerHelpers {

#ifdef RGB_BUILTIN
struct Colour {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};
extern Colour blue;
extern Colour green;
extern Colour red;
extern Colour orange;
#endif

class Blinker {
 public:
  explicit Blinker(int pin, uint8_t valOff = LOW);
  #ifdef RGB_BUILTIN
  void on(Colour colour, uint32_t interval = 0);
  #else
  void on(uint32_t interval = 0);
  #endif
  void off();
  void loop();

 protected:
  const int _pin;
  const uint8_t _valOff;
  bool _ledState;
  uint32_t _interval;
  uint32_t _lastInterval;
  #ifdef RGB_BUILTIN
  Colour _colour;
  #endif
};

}  // end namespace espMqttManagerHelpers
