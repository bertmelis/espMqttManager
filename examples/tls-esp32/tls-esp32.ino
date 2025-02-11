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
    int8_t signalQuality = WiFi.RSSI();
    char buf[5];
    snprintf(buf, sizeof(buf), "%d", signalQuality);
    espMqttManager::mqttClient.publish("test/topic", 0, false, buf);
  }
  espMqttManager::loop();
}
