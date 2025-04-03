#include <Arduino.h>
#include "common.h"

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ;
    }
    Serial.println("Serial communication initialized.");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
  }
    Serial.println("Connected to WiFi");

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started");

    Wire.begin();
    Serial.println("I2C initialized");

    setupMotors();
}

void loop() {
    webSocket.loop();
    handleMotorControl();
}


