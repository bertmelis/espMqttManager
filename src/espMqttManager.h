/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#else
#error Platform not supported
#endif

#include <espMqttClient.h>

#include "Helpers/Config.h"
#include "Helpers/Helpers.h"
#include "Helpers/Uptime.h"

#ifndef ESP_MQTT_MANAGER_SECURE
#define ESP_MQTT_MANAGER_SECURE 0
#endif

// time to wait for clean disconnect before forcibly disconnectin
// in milliseconds
#ifndef ESP_MQTT_MANAGER_DISCONNECT_TIMEOUT
#define ESP_MQTT_MANAGER_DISCONNECT_TIMEOUT 10000
#endif

void onSetupSession() __attribute__((weak));
void onWiFiConnected() __attribute__((weak));
void onWiFiDisconnected() __attribute__((weak));
void onMqttConnected() __attribute__((weak));
void onMqttDisconnected(espMqttClientTypes::DisconnectReason reason) __attribute__((weak));
void onReset() __attribute__((weak));

namespace espMqttManager {

void setup();
void start();
void loop();
void sessionReady();
bool disconnect(bool clearSession = false);
bool isConnected();
#if ESP_MQTT_MANAGER_SECURE
extern espMqttClientSecure mqttClient;
#else
extern espMqttClient mqttClient;
#endif

}  // end namespace espMqttManager
