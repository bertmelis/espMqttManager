/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <Arduino.h>

#include "espMqttManager.h"
#include "Helpers/Logging.h"
#include "Helpers/Blinker.h"

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
espMqttManagerHelpers::Config config;

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
  WiFi.persistent(false);

  if (!config.getConfig()) {
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

bool espMqttManager::disconnect(bool clearSession) {
  if (state != connected) {
    return false;
  }
  // set state first to be in proper state when handling onMqttClientDisconnected
  if (clearSession) {
    state = waitForDisconnectCleanSession;
    mqttClient.setCleanSession(true);
  } else {
    state = waitForDisconnect;
  }
  timer = millis();
  mqttClient.disconnect();
  return true;
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
    emm_log_i("Disconnecting from MQTT");
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
  timer = millis();
  if (state == waitForWiFi ||
      state == reconnectWaitMqtt ||
      state == waitForMqtt ||
      state == setupSession ||
      state == connected) {
    #ifdef RGB_BUILTIN
    blinker.blink(250, espMqttManagerHelpers::blue);
    #elif defined (LED_BUILTIN)
    blinker.blink(250);
    #endif
    interval = getBackoffTimerVal(interval);
    emm_log_i("Disconnected from MQTT");
    state = reconnectWaitMqtt;
    onMqttDisconnected();
  } else if (state == waitForDisconnect) {
    emm_log_i("Disconnected from MQTT");
    state = idle;
    onMqttDisconnected();
  } else if (state == waitForDisconnectCleanSession) {
    emm_log_i("Disconnected from MQTT (CS-1)");
    interval = 0;
    state = reconnectWaitCleanSession;
  } else if (state == waitForMqttCleanSession) {
    emm_log_i("Disconnected from MQTT (CS-2)");
    interval = getBackoffTimerVal(interval);
    state = reconnectWaitCleanSession;
  } else if (state == waitForDisconnectFinal) {
    emm_log_i("Disconnected from to MQTT (Final)");
    onReset();
  }
}
