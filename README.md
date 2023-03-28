## espMqttManger

Boilerplate code to manage WiFi and MQTT for Espressif ESP32 and ESP8266 devices using the Arduino framework.

Based on [espMqttClient](https://github.com/bertmelis/espMqttClient)

## Usage

#### Create `/config.json` in the ESPs filesystem (SPIFFS)

```
{
  "SSID": "network-ssid",
  "PSK": "network-psk",
  "IPAddress": [0,0,0,0],
  "hostname" : "mqtt.domain.tld",
  "port": 1883,
  "devicename": "testdevice"
}
```

* You don't have to specify an IP address and hostname of the MQTT broker. The Broker's hostname takes precedence over IP.
* `port` and `devicename` are optional.

Use `pio run -t uploadfs` to upload when using Platformio.

#### Add espMqttManager to your code

Below is a simple version of how to use espMqttManager. Just add the header, call setup, start and loop.

The methods are not wrapped in a class, only in a namespace. The MQTT client is available using `espMQttManager::mqttClient`.

```cpp
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
    char sqStr[5];
    snprintf(sqStr, sizeof(sqStr), "%d%", signalQuality);
    espMqttManager::mqttClient.publish("test/topic", 0, false, sqStr);
  }
  espMqttManager::loop();
}
```

## Advanced usage

#### Standard function

```cpp
void setup();
```

Setup espMqttManager. Call from Arduino setup.

```cpp
void start();
```

Start espMqttManager. Call from Arduino setup. After calling this, the device will try to connect to WiFi.
Make so to set any custom properties of the MQTT client before calling `start()`.

```cpp
void loop();
```

Worker task of the manager. Call from Arduino loop().

```cpp
void sessionReady();
```

See Event hooks, onSetupSession

```cpp
void disconnect(bool clearSession = false);
```

Disconnect from MQTT. WiFi will not disconnect. espMqttManager will try to cleanly disconnect from MQTT but forcibly after 10 seconds.
When setting `clearSession` to `true`, espMqttManager will clear the session from the broker and disconnect.
onMqttDisconnected will be called when disconnected or onReset when disconnected when `clearSession` is set to `true`.

Note that espMqttManager will not reconnect automatically after calling disconnect.

#### Event hooks

There are a few functions defined but only weakly linked. You can override these in your code.

```cpp
void onSetupSession();
```

onSetupSession is called when a connection to the MQTT broker has been
established but no session was found (clean session = true).

You can use this function to set up the session eg. make subscriptions etc.
When done, make sure to call espMqttManager::sessionReady to complete the setup.

```cpp
void onMqttConnected();
```

Called when the device is fully connected to MQTT and the session has been setup

```cpp
void onMqttDisconnected();
```

Called whenever the device disconnects from MQTT.
espMqttManager will automatically reconnect, you don't have to do this.

```cpp
void onReset();
```
onReset is called when the device is fully disconnected from MQTT and the session
has been deleted. This is useful after updating the firmware.

#### Helpers

```cpp
uint8_t espMqttManagerHelpers::signalQuality();
```

Returns the WiFi signal quality as a percentage.

```cpp
void espMqttManagerHelpers::handleUpdate(const uint8_t* payload, size_t length, size_t index, size_t total);
bool espMqttManagerHelpers::updated;
```

Can be used to update the firmware of the device. The arguments match the ones from the onMessage callback from espMqttClient.
The `updated` boolean is a flag that is set when the update has been (successfully) completed.

You can disconnect with `clearSession` set to `true` after the update.

## License

This library is released under the MIT Licence. A copy is included in the repo.
