/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "freertos/FreeRTOS.h"
#include "Logging.h"

#if defined(DEBUG_ESP_MQTT_MANAGER)
portMUX_TYPE loggingMutex;
#endif
