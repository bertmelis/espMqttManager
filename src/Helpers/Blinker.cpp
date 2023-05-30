/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Blinker.h"

namespace espMqttManagerHelpers {

#ifdef RGB_BUILTIN
Colour blue = {0, 0, 25};
Colour green = {0, 25, 0};
Colour red = {25, 0, 0};
Colour orange = {25, 17, 0};
#endif

Blinker::Blinker(int pin, uint8_t valOff)
: _pin(pin)
, _valOff(valOff)
, _ledState(false)
, _interval(100)
, _lastInterval(0)
#ifdef RGB_BUILTIN
, _colour(blue)
#endif
{
  // empty
}

#ifdef RGB_BUILTIN
void Blinker::blink(uint32_t interval, Colour colour) {
#else
void Blinker::blink(uint32_t interval) {
#endif
  #ifdef RGB_BUILTIN
  _colour = colour;
  #endif
  _interval = interval;
  _lastInterval = millis() - _interval;
  _ledState = false;
  loop();
}

#ifdef RGB_BUILTIN
void Blinker::on(Colour colour) {
#else
void Blinker::on() {
#endif
  _interval = 0;
  #ifdef RGB_BUILTIN
  neopixelWrite(_pin, colour.red, colour.green, colour.blue);
  #else
  digitalWrite(_pin, ~_valOff);
  #endif
}

void Blinker::off() {
  #ifdef RGB_BUILTIN
  neopixelWrite(_pin, 0, 0, 0);
  #else
  digitalWrite(_pin, _valOff);
  #endif
}

void Blinker::loop() {
  if (_interval == 0) return;
  if (millis() - _lastInterval >= _interval) {
    _lastInterval = millis();
    #ifdef RGB_BUILTIN
    if (_ledState) {
      neopixelWrite(_pin, 0, 0, 0);
    } else {
      neopixelWrite(_pin, _colour.red, _colour.green, _colour.blue);
    }
    #else
    if (_ledState) {
      digitalWrite(_pin, LOW);
    } else {
      digitalWrite(_pin, HIGH);
    }
    #endif
    _ledState = !_ledState;
  }
}

}  // end namespace espMqttManagerHelpers
