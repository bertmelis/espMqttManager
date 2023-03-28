/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <LittleFS.h>
#define EMM_FILE_READ r
#elif defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#define EMM_FILE_READ FILE_READ
#endif

#include <IPAddress.h>

#include <ArduinoJson.h>

#include "espMqttManager.h"
#include "Logging.h"

#ifndef EMM_CONFIG_FILE
#define EMM_CONFIG_FILE "/config.json"
#endif

void onSetupSession() {
  espMqttManager::sessionReady();
}

void onMqttConnected() {
  (void)0;
}

void onMqttDisconnected() {
  (void)0;
}

void onReset() {
  ESP.restart();
}

#if ESP_MQTT_MANAGER_SECURE
espMqttClientSecure espMqttManager::mqttClient;
#else
espMqttClient espMqttManager::mqttClient;
#endif
#ifdef RGB_BUILTIN
espMqttManagerHelpers::Blinker blinker(RGB_BUILTIN);
#else
espMqttManagerHelpers::Blinker blinker(LED_BUILTIN);
#endif

void idle();
void waitForWiFi();
void reconnectWaitMqtt();
void waitForMqtt();
void setupSession();
void connected();
void waitForDisconnect();
void waitForDisconnectCleanSession();
void reconnectWaitCleanSession();
void waitForMqttCleanSession();
void connectedCleanSession();
void waitForDisconnectFinal();

void onMqttClientConnected(bool sessionPresent);
void onMqttClientDisconnected(espMqttClientTypes::DisconnectReason reason);

typedef void (*stateFunction)();
stateFunction state = idle;
uint32_t timer = 0;
uint32_t interval = 0;

struct Config {
  char SSID[64];
  char PSK[64];
  IPAddress hostIP;
  char hostname[64];
  uint16_t port;
  char devicename[64];
} config;

bool getConfig() {
  #if defined(ARDUINO_ARCH_ESP8266)
  #define EMM_FILESYSTEM LittleFS
  #elif defined(ARDUINO_ARCH_ESP32)
  #define EMM_FILESYSTEM SPIFFS
  #endif

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
  strlcpy(config.SSID,
          doc["SSID"] | "",
          sizeof(config.SSID));
  strlcpy(config.PSK,
          doc["PSK"] | "",
          sizeof(config.PSK));
  JsonArray hostIP = doc["IPAddress"];
  uint8_t IPAddress_0 = hostIP[0] | 0;
  uint8_t IPAddress_1 = hostIP[1] | 0;
  uint8_t IPAddress_2 = hostIP[2] | 0;
  uint8_t IPAddress_3 = hostIP[3] | 0;
  config.hostIP = IPAddress(IPAddress_0, IPAddress_1, IPAddress_2, IPAddress_3);
  strlcpy(config.hostname,
          doc["hostname"] | "",
          sizeof(config.hostname));
  config.port = doc["port"] | 1883;
  strlcpy(config.devicename,
          doc["devicename"] | "",
          sizeof(config.devicename));
  file.close();
  EMM_FILESYSTEM.end();
  return true;
}

uint32_t getBackoffTimerVal(uint32_t currentInterval) {
  if (currentInterval == 0) currentInterval = 2000;
  currentInterval *= 2;
  if (currentInterval > 130000) {
    currentInterval = 130000;
  }
  return currentInterval;
}

void espMqttManager::setup() {
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(false);

  if (!getConfig()) {
    emm_log_e("Error getting config");
    return;
  }

  mqttClient.setCleanSession(false);
  mqttClient.setKeepAlive(65);
  mqttClient.onConnect(onMqttClientConnected);
  mqttClient.onDisconnect(onMqttClientDisconnected);
  if (strlen(config.hostname) > 0) {
    mqttClient.setServer(config.hostname, config.port);
  } else {
    mqttClient.setServer(config.hostIP, config.port);
  }
  if (strlen(config.devicename) > 0) {
    mqttClient.setClientId(config.devicename);
  }
  blinker.off();
}

void espMqttManager::start() {
  WiFi.begin(config.SSID, config.PSK);
  state = waitForWiFi;
  #ifdef RGB_BUILTIN
  blinker.blink(100, espMqttManagerHelpers::green);
  #elif defined (LED_BUILTIN)
  blinker.blink(100);
  #endif
}

void espMqttManager::loop() {
  // espMqttManager doesn't use WiFi events so we have to monitor WiFi here
  if (WiFi.status() != WL_CONNECTED) {
    // mqttClient.disconnect(true);  // this will set state back to _mqttConnecting
    state = waitForWiFi;
  }
  blinker.loop();
  state();
}

void espMqttManager::sessionReady() {
  if (state == setupSession) {
    blinker.off();
    emm_log_i("Session ready");
    state = connected;
  }
}

void espMqttManager::disconnect(bool clearSession) {
  mqttClient.disconnect();
  if (clearSession) {
    state = waitForDisconnectCleanSession;
  } else {
    state = waitForDisconnect;
  }
}

void idle() {
  (void)0;
}

void waitForWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    emm_log_i("WiFi connected");
    state = reconnectWaitMqtt;
    timer = millis();
    interval = 0;
    #ifdef RGB_BUILTIN
    blinker.blink(250, espMqttManagerHelpers::green);
    #elif defined (LED_BUILTIN)
    blinker.blink(250);
    #endif
  }
}

void reconnectWaitMqtt() {
  if (millis() - timer > interval) {
    if (espMqttManager::mqttClient.connect()) {
      emm_log_i("Connecting to MQTT");
      #ifdef RGB_BUILTIN
      blinker.blink(100, espMqttManagerHelpers::blue);
      #elif defined (LED_BUILTIN)
      blinker.blink(100);
      #endif
      state = waitForMqtt;
    }
  }
}

void waitForMqtt() {
  (void)0;
  // state released by espMqttClient onConnect callback
}

void setupSession() {
  (void)0;
  // state released by user action using espMqttManager::sessionSetup()
}

void connected() {
  // send stats
  (void)0;
}

void waitForDisconnect() {
  // state released by espMqttClient onDisconnect callback or force close connection
  if (millis() - timer > ESP_MQTT_MANAGER_DISCONNECT_TIMEOUT) {
    emm_log_i("Disconnecting from MQTT (CS)");
    espMqttManager::mqttClient.disconnect(true);
  }
}

void waitForDisconnectCleanSession() {
  // state released by espMqttClient onDisconnect callback or force close connection
  if (millis() - timer > ESP_MQTT_MANAGER_DISCONNECT_TIMEOUT) {
    emm_log_i("Disconnecting from MQTT (CS)");
    espMqttManager::mqttClient.disconnect(true);
  }
}

void reconnectWaitCleanSession() {
  if (millis() - timer > interval) {
    if (espMqttManager::mqttClient.connect()) {
      emm_log_i("Reconnecting to MQTT (CS)");
      state = waitForMqttCleanSession;
    }
  }
}

void waitForMqttCleanSession() {
  (void)0;
  // state released by espMqttClient onConnect callback
}

void connectedCleanSession() {
  espMqttManager::mqttClient.disconnect();
  emm_log_i("Disconnecting to MQTT (Final)");
  state = waitForDisconnectFinal;
}

void waitForDisconnectFinal() {
  (void)0;
  // state released by espMqttClient onDisconnect callback
}

void onMqttClientConnected(bool sessionPresent) {
  interval = 0;
  if (state == waitForMqtt) {
    if (sessionPresent) {
      emm_log_i("Connected to MQTT");
      blinker.off();
      state = connected;
      onMqttConnected();
    } else {
      emm_log_i("Connected to MQTT, no session present");
      #ifdef RGB_BUILTIN
      blinker.blink(50, espMqttManagerHelpers::blue);
      #elif defined (LED_BUILTIN)
      blinker.blink(50);
      #endif
      state = setupSession;
      onSetupSession();
    }
  } else if (state == waitForMqttCleanSession) {
    emm_log_i("Connected to MQTT (CS)");
    state = connectedCleanSession;
  }
}

void onMqttClientDisconnected(espMqttClientTypes::DisconnectReason reason) {
  (void) reason;
  timer = millis();
  if (state > idle && state <= connected) {
    #ifdef RGB_BUILTIN
    blinker.blink(250, espMqttManagerHelpers::blue);
    #elif defined (LED_BUILTIN)
    blinker.blink(250);
    #endif
    interval = getBackoffTimerVal(interval);
    emm_log_i("Disconnected from to MQTT");
    state = reconnectWaitMqtt;
    onMqttDisconnected();
  } else if (state == waitForDisconnect) {
    emm_log_i("Disconnected from to MQTT");
    state = idle;
    onMqttDisconnected();
  } else if (state == waitForDisconnectCleanSession) {
    emm_log_i("Disconnected from to MQTT (CS-1)");
    interval = 0;
    state = reconnectWaitCleanSession;
  } else if (state == waitForMqttCleanSession) {
    emm_log_i("Disconnected from to MQTT (CS-2)");
    interval = getBackoffTimerVal(interval);
    state = reconnectWaitCleanSession;
  } else if (state == waitForDisconnectFinal) {
    emm_log_i("Disconnected from to MQTT (Final)");
    onReset();
  }
}
