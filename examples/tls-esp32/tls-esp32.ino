#include <Arduino.h>

#include <espMqttManager.h>

#include "certs.h"

void setup() {
  delay(10000);
  Serial.begin(115200);
  espMqttManager::setup();

  // add or modify the MQTT clients properties between setup and start
  espMqttManager::mqttClient.setCertificate(clientCert);
  espMqttManager::mqttClient.setPrivateKey(clientKey);
  espMqttManager::mqttClient.setCACert(serverCert);

  espMqttManager::start();
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    uint8_t signalQuality = espMqttManagerHelpers::signalQuality();
    char sqStr[5];
    snprintf(sqStr, sizeof(sqStr), "%d%", signalQuality);
    espMqttManager::mqttClient.publish("test/topic", 0, false, sqStr);
  }
  espMqttManager::loop();
}
