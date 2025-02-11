#include <Arduino.h>

#include <espMqttManager.h>

void setup() {
  delay(10000);
  Serial.begin(74880);
  espMqttManager::setup();
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
