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
  (void)reason;
  (void)0;
}

void onDisconnected() {
  (void)0;
}

#if ESP_MQTT_MANAGER_SECURE
espMqttClientSecure espMqttManager::mqttClient;
#else
espMqttClient espMqttManager::mqttClient;
#endif

void onMqttClientConnected(bool sessionPresent);
void onMqttClientDisconnected(espMqttClientTypes::DisconnectReason reason);

namespace espMqttManagerInternals {

void idle();
void connectWiFi();
void waitWiFi();
void connectMqtt();
void setupSession();
void waitDisconnect();

enum class espMqttManagerState {
  OFFLINE,
  CONNECTWIFI,
  WAITWIFI,
  CONNECTMQTT,
  WAITMQTT,
  SETUPSESSION,
  CONNECTED,
  WAITDISCONNECT,
  DISCONNECTED
};
espMqttManagerState state;
uint32_t mqttReconnectTimer = 0;
uint32_t wifiReconnectTimer = 0;
uint32_t interval = 0;
espMqttManagerHelpers::Config config;

void setState(espMqttManagerState newState) {
  emm_log_i("new espMqttManagerInternals::state: %u", newState);
  state = newState;
}

}  // end namespace espMqttManagerInternals

using espMqttManagerInternals::espMqttManagerState;

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

  if (!espMqttManagerInternals::config.getConfig()) {
    emm_log_e("Error getting config");
    return;
  }

  mqttClient.setCleanSession(true);
  mqttClient.setKeepAlive(65);
  mqttClient.onConnect(onMqttClientConnected);
  mqttClient.onDisconnect(onMqttClientDisconnected);
  if (strlen(espMqttManagerInternals::config.hostname) > 0) {
    mqttClient.setServer(espMqttManagerInternals::config.hostname, espMqttManagerInternals::config.port);
  } else {
    mqttClient.setServer(espMqttManagerInternals::config.hostIP, espMqttManagerInternals::config.port);
  }
  if (strlen(espMqttManagerInternals::config.devicename) > 0) {
    mqttClient.setClientId(espMqttManagerInternals::config.devicename);
  }
  if (strlen(espMqttManagerInternals::config.username) > 0) {
    mqttClient.setCredentials(espMqttManagerInternals::config.username, espMqttManagerInternals::config.password);
  }
}

void espMqttManager::start() {
  espMqttManagerInternals::setState(espMqttManagerState::CONNECTWIFI);
  espMqttManagerInternals::wifiReconnectTimer = millis();
  espMqttManagerInternals::interval = 0;
}

void espMqttManager::loop() {  // espMqttManager doesn't use WiFi events so we have to monitor WiFi here
  if (WiFi.status() != WL_CONNECTED && espMqttManagerInternals::state != espMqttManagerState::CONNECTWIFI && espMqttManagerInternals::state != espMqttManagerState::WAITWIFI) {
    espMqttManagerInternals::setState(espMqttManagerState::CONNECTWIFI);
    espMqttManagerInternals::wifiReconnectTimer = millis();
    espMqttManagerInternals::interval = 0;
    onWiFiDisconnected();
  }
  switch (espMqttManagerInternals::state) {
    case espMqttManagerState::OFFLINE:
    espMqttManagerInternals::idle();
    break;
    case espMqttManagerState::CONNECTWIFI:
    espMqttManagerInternals::connectWiFi();
    break;
    case espMqttManagerState::WAITWIFI:
    espMqttManagerInternals::waitWiFi();
    break;
    case espMqttManagerState::CONNECTMQTT:
    espMqttManagerInternals::connectMqtt();
    break;
    case espMqttManagerState::WAITMQTT:
    espMqttManagerInternals::idle();
    break;
    case espMqttManagerState::SETUPSESSION:
    espMqttManagerInternals::setupSession();
    break;
    case espMqttManagerState::CONNECTED:
    espMqttManagerInternals::idle();
    break;
    case espMqttManagerState::WAITDISCONNECT:
    espMqttManagerInternals::waitDisconnect();
    break;
    case espMqttManagerState::DISCONNECTED:
    espMqttManagerInternals::idle();
    break;
  }
  #if defined(ARDUINO_ARCH_ESP8266)
  espMqttManager::mqttClient.loop();
  #endif
}

void espMqttManager::sessionReady() {
  if (espMqttManagerInternals::state == espMqttManagerState::SETUPSESSION) {
    emm_log_i("Session ready");
    espMqttManager::mqttClient.setCleanSession(false);
    espMqttManagerInternals::setState(espMqttManagerState::CONNECTED);
    onMqttConnected();
  }
}

bool espMqttManager::disconnect() {
  if (espMqttManagerInternals::state != espMqttManagerState::CONNECTED) {
    return false;
  }
  // set espMqttManagerInternals::state first to be in proper espMqttManagerInternals::state when handling onMqttClientDisconnected
  espMqttManagerInternals::state = espMqttManagerState::WAITDISCONNECT;
  espMqttManagerInternals::mqttReconnectTimer = millis();
  mqttClient.disconnect();
  return true;
}

bool espMqttManager::isConnected() {
  return (espMqttManagerInternals::state == espMqttManagerState::CONNECTED) ? true : false;
}

void espMqttManagerInternals::idle() {
  (void)0;
}

void espMqttManagerInternals::connectWiFi() {
  if (millis() - espMqttManagerInternals::wifiReconnectTimer > espMqttManagerInternals::interval) {
    if (WiFi.begin(config.SSID, espMqttManagerInternals::config.PSK) != WL_CONNECT_FAILED) {
      #if defined(EMM_LOWER_WIFIPOWER)
      WiFi.setTxPower(EMM_LOWER_WIFIPOWER);
      #endif
      espMqttManagerInternals::setState(espMqttManagerState::WAITWIFI);
    }
    espMqttManagerInternals::wifiReconnectTimer = millis();
    espMqttManagerInternals::interval = getBackoffTimerVal(interval);
  }
}

void espMqttManagerInternals::waitWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    emm_log_i("WiFi connected");
    espMqttManagerInternals::setState(espMqttManagerState::CONNECTMQTT);
    espMqttManagerInternals::mqttReconnectTimer = millis();
    espMqttManagerInternals::interval = 0;
    onWiFiConnected();
  } else if (millis() - espMqttManagerInternals::wifiReconnectTimer > espMqttManagerInternals::interval) {
    espMqttManagerInternals::setState(espMqttManagerState::CONNECTWIFI);
  }
}

void espMqttManagerInternals::connectMqtt() {
  if (millis() - espMqttManagerInternals::mqttReconnectTimer > espMqttManagerInternals::interval && espMqttManager::mqttClient.connect()) {
    emm_log_i("Connecting to MQTT");
    espMqttManagerInternals::setState(espMqttManagerState::WAITMQTT);
  }
}

void espMqttManagerInternals::setupSession() {
  onSetupSession();
  // espMqttManagerInternals::state released by user action using espMqttManager::sessionSetup()
}

void espMqttManagerInternals::waitDisconnect() {
  // espMqttManagerInternals::state released by espMqttClient onDisconnect callback or force close connection
  if (millis() - espMqttManagerInternals::mqttReconnectTimer > ESP_MQTT_MANAGER_DISCONNECT_TIMEOUT) {
    emm_log_i("Disconnecting from MQTT");
    espMqttManager::mqttClient.disconnect(true);
  }
}

void onMqttClientConnected(bool sessionPresent) {
  espMqttManagerInternals::interval = 0;
  if (espMqttManagerInternals::state == espMqttManagerState::WAITMQTT) {
    emm_log_i("Connected to MQTT (session: %s)", sessionPresent ? "y" : "n");
    if (sessionPresent) {
      espMqttManagerInternals::setState(espMqttManagerState::CONNECTED);
      onMqttConnected();
    } else {
      espMqttManagerInternals::setState(espMqttManagerState::SETUPSESSION);
    }
  }
}

void onMqttClientDisconnected(espMqttClientTypes::DisconnectReason reason) {
  espMqttManagerInternals::mqttReconnectTimer = millis();
  if (espMqttManagerInternals::state == espMqttManagerState::WAITDISCONNECT) {
    emm_log_i("Disconnected from MQTT");
    espMqttManagerInternals::setState(espMqttManagerState::DISCONNECTED);
    onDisconnected();
  } else {
    espMqttManagerInternals::interval = getBackoffTimerVal(espMqttManagerInternals::interval);
    emm_log_i("Disconnected from MQTT");
    onMqttDisconnected(reason);
    espMqttManagerInternals::setState(espMqttManagerState::CONNECTMQTT);
  }
}
