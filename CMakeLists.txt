set(COMPONENT_SRCDIRS
    "src" "src/Helpers"
)

set(COMPONENT_ADD_INCLUDEDIRS
    "src" "src/Helpers"
)

set(COMPONENT_REQUIRES
    "arduino-esp32"
    "spiffs"
    "esp_wifi"
    "network_provisioning"
    "espMqttClient"
    "ArduinoJson"
)

register_component()

target_compile_options(${COMPONENT_TARGET} PRIVATE -fno-rtti)
