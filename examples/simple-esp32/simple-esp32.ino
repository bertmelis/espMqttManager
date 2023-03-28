#include <Arduino.h>

#include <espMqttManager.h>

void setup() {
  delay(10000);
  Serial.begin(115200);
  espMqttManager::setup();
  espMqttManager::start();
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    uint8_t signalQuality = espMqttManagerHelpers::signalQuality();
    char sqStr[4];
    snprintf(sqStr, sizeof(sqStr), "%d", signalQuality);
    espMqttManager::mqttClient.publish("test/topic", 0, false, sqStr);
  }
  espMqttManager::loop();
}
