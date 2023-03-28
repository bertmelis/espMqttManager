/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

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
  Blinker(int pin);
  #ifdef RGB_BUILTIN
  void blink(uint32_t interval, Colour colour);
  #else
  void blink(uint32_t interval);
  #endif
  void off();
  void loop();

 protected:
  const int _pin;
  bool _state;
  bool _ledState;
  uint32_t _interval;
  uint32_t _lastInterval;
  #ifdef RGB_BUILTIN
  Colour _colour;
  #endif
};

}  // end namespace espMqttManagerHelpers