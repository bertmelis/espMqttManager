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

void onWiFiConnected() {
  (void)0;
}

void onWiFiDisconnected() {
  (void)0;
}

void onMqttConnected() {
  (void)0;
}

void onMqttDisconnected(espMqttClientTypes::DisconnectReason reason) {
  (void) reason;
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

void idle();
void startWiFi();
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

namespace espMqttManagerInternals {

typedef void (*stateFunction)();
stateFunction state = idle;
uint32_t mqttReconnectTimer = 0;
uint32_t wifiReconnectTimer = 0;
uint32_t interval = 0;
bool disconnectCalled = false;
espMqttManagerHelpers::Config config;

}  // end namespace espMqttManagerInternals

using espMqttManagerInternals::state;
using espMqttManagerInternals::mqttReconnectTimer;
using espMqttManagerInternals::wifiReconnectTimer;
using espMqttManagerInternals::interval;
using espMqttManagerInternals::disconnectCalled;
using espMqttManagerInternals::config;

uint32_t getBackoffTimerVal(uint32_t currentInterval) {
  if (currentInterval == 0) currentInterval = 2000;
  currentInterval *= 2;
  if (currentInterval > 130000) {
    currentInterval = 130000;
  }
  return currentInterval;
}

void espMqttManager::setup() {
  WiFi.setAutoReconnect(false);
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
  if (strlen(config.username) > 0) {
    mqttClient.setCredentials(config.username, config.password);
  }
}

void espMqttManager::start() {
  state = startWiFi;
  mqttReconnectTimer = millis();
  interval = 0;
}

void espMqttManager::loop() {  // espMqttManager doesn't use WiFi events so we have to monitor WiFi here
  if (WiFi.status() != WL_CONNECTED && state != startWiFi && state != waitForWiFi) {
    if (!disconnectCalled) {
      disconnectCalled = true;
    onWiFiDisconnected();
    }
    if (millis() - wifiReconnectTimer > 10000) {
      
      wifiReconnectTimer = millis();
      if (WiFi.reconnect()) {
        emm_log_i("Trying to reconnect to WiFi");
      } else {
        emm_log_i("Reconnect to WiFi failed");
      }
    }
  } else if (WiFi.status() == WL_CONNECTED) {
    disconnectCalled = false;
  }
  static uint32_t lastTime = millis();
  if (millis() - lastTime > 10000) {
    lastTime = millis();
  }
  #if defined(ARDUINO_ARCH_ESP8266)
  espMqttManager::mqttClient.loop();
  #endif
  state();
}

void espMqttManager::sessionReady() {
  if (state == setupSession) {
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
  mqttReconnectTimer = millis();
  mqttClient.disconnect();
  return true;
}

bool espMqttManager::isConnected() {
  return (state == connected) ? true : false;
}

void idle() {
  (void)0;
}

void startWiFi() {
  if (millis() - mqttReconnectTimer > interval) {
    if (WiFi.begin(config.SSID, config.PSK) != WL_CONNECT_FAILED) {
      #if defined(EMM_LOWER_WIFIPOWER)
      WiFi.setTxPower(EMM_LOWER_WIFIPOWER);
      #endif
  state = waitForWiFi;
    }
    mqttReconnectTimer = millis();
    interval = getBackoffTimerVal(interval);
  }
}

void waitForWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    emm_log_i("WiFi connected");
    state = reconnectWaitMqtt;
    mqttReconnectTimer = millis();
    interval = 0;
    onWiFiConnected();
  }
}

void reconnectWaitMqtt() {
  if (WiFi.status() == WL_CONNECTED && millis() - mqttReconnectTimer > interval) {
    if (espMqttManager::mqttClient.connect()) {
      emm_log_i("Connecting to MQTT");
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
  if (millis() - mqttReconnectTimer > ESP_MQTT_MANAGER_DISCONNECT_TIMEOUT) {
    emm_log_i("Disconnecting from MQTT");
    espMqttManager::mqttClient.disconnect(true);
  }
}

void waitForDisconnectCleanSession() {
  // state released by espMqttClient onDisconnect callback or force close connection
  if (millis() - mqttReconnectTimer > ESP_MQTT_MANAGER_DISCONNECT_TIMEOUT) {
    emm_log_i("Disconnecting from MQTT (CS)");
    espMqttManager::mqttClient.disconnect(true);
  }
}

void reconnectWaitCleanSession() {
  if (WiFi.status() == WL_CONNECTED && millis() - mqttReconnectTimer > interval) {
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
      state = connected;
      onMqttConnected();
    } else {
      emm_log_i("Connected to MQTT, no session present");
      state = setupSession;
      onSetupSession();
    }
  } else if (state == waitForMqttCleanSession) {
    emm_log_i("Connected to MQTT (CS)");
    state = connectedCleanSession;
  }
}

void onMqttClientDisconnected(espMqttClientTypes::DisconnectReason reason) {
  mqttReconnectTimer = millis();
  if (state == waitForWiFi ||
      state == reconnectWaitMqtt ||
      state == waitForMqtt ||
      state == setupSession ||
      state == connected) {
    interval = getBackoffTimerVal(interval);
    emm_log_i("Disconnected from MQTT");
    state = reconnectWaitMqtt;
    onMqttDisconnected(reason);
  } else if (state == waitForDisconnect) {
    emm_log_i("Disconnected from MQTT");
    state = idle;
    onMqttDisconnected(reason);
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
