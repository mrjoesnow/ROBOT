#ifndef SETUP_AND_JSON_CPP
#define SETUP_AND_JSON_CPP

#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "common.h"

const char* ssid = "BARN WIFI";
const char* password = "512939692b";

WebSocketsServer webSocket = WebSocketsServer(81);

int stepSize = 100;
bool moveX = false, moveY = false, moveZ = false;
int directionX = 1, directionY = 1, directionZ = 1;
bool isMoving = false;
bool solenoidActivated = false;
bool homeSwitch1Triggered = false;
bool homeSwitch2Triggered = false;
bool homeSwitch3Triggered = false;
bool isHomed = false;

const int predefinedStepSizes[] = {1, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
const int numPredefinedStepSizes = sizeof(predefinedStepSizes) / sizeof(predefinedStepSizes[0]);

int axisDirectionX = -1;
int axisDirectionY = 1;
int axisDirectionZ = -1;

int homingSpeedX = 1000;
int homingSpeedY = 1000;
int homingSpeedZ = 280;

int homingDirectionX = -1;
int homingDirectionY = -1;
int homingDirectionZ = -1;

int powerLevel = 2500;
int loadingSpeed = 280;
int ballLoadingSteps = 4200;

int delayTimeOn = 280;
int delayTimeOff = 100;

int speedX = 1000;
int speedY = 1000;
int speedZ = 280;

const float stepsToDegreesX = 0.1;
const float stepsToDegreesY = 0.1;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket disconnected");
            break;
        case WStype_CONNECTED:
            Serial.println("WebSocket connected");
            flashLED(ONBOARD_LED);
            if (!isHomed) {
                homeMachine();
                moveToPresetZeroPosition();
                isHomed = true;
            }
    sendLog(num);
        sendReadyStatus();
            break;
        case WStype_TEXT:
            Serial.println("WebSocket text message received");
            handleWebSocketMessage(num, payload);
            break;
        case WStype_ERROR:
            Serial.println("WebSocket error");
            break;
        case WStype_BIN:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        case WStype_PING:
        case WStype_PONG:
            break;
    }
    }

void handleWebSocketMessage(uint8_t num, uint8_t * payload) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }
    if (doc["X"].is<int>()) {
        int value = doc["X"].as<int>();
        directionX = (value > 0) ? 1 : -1;
        moveX = (value != 0);
    }
    if (doc["Y"].is<int>()) {
        int value = doc["Y"].as<int>();
        directionY = (value > 0) ? 1 : -1;
        moveY = (value != 0);
    }
    if (doc["Z"].is<int>()) {
        int value = doc["Z"].as<int>();
        directionZ = (value > 0) ? 1 : -1;
        moveZ = (value != 0);
    }
    if (doc["STEPS"].is<int>()) {
        stepSize = doc["STEPS"].as<int>();
    }
    if (doc["THROW"].is<bool>() && doc["THROW"].as<bool>()) {
        if (doc["POWER_LEVEL"].is<int>()) {
            int newPowerLevel = doc["POWER_LEVEL"].as<int>();
            if (newPowerLevel != powerLevel) {
                powerLevel = newPowerLevel;
            }
        }
        if (doc["BALL_LOADING_STEPS"].is<int>()) {
            int newBallLoadingSteps = doc["BALL_LOADING_STEPS"].as<int>();
            if (newBallLoadingSteps != ballLoadingSteps) {
                ballLoadingSteps = newBallLoadingSteps;
            }
        }
        sendI2CData();
        throwRoutine(powerLevel);
    }
    if (doc["TEST_SOLENOID"].is<bool>() && doc["TEST_SOLENOID"].as<bool>()) {
        solenoidActivated = true;
    }
    if (doc["HOME_X"].is<bool>() && doc["HOME_X"].as<bool>()) {
        homeAxis(1, homingSpeedX);
        stepsX = 0;
        sendReadyStatus();
    }
    if (doc["HOME_Y"].is<bool>() && doc["HOME_Y"].as<bool>()) {
        homeAxis(2, homingSpeedY);
        stepsY = 0;
        sendReadyStatus();
    }
    if (doc["HOME_Z"].is<bool>() && doc["HOME_Z"].as<bool>()) {
        homeAxis(3, homingSpeedZ);
        stepsZ = 0;
        sendReadyStatus();
}
    if (doc["LOADING_SPEED"].is<int>()) {
        loadingSpeed = doc["LOADING_SPEED"].as<int>();
    }
    if (doc["HOMING_SPEED_X"].is<int>()) {
        homingSpeedX = doc["HOMING_SPEED_X"].as<int>();
    }
    if (doc["HOMING_SPEED_Y"].is<int>()) {
        homingSpeedY = doc["HOMING_SPEED_Y"].as<int>();
    }
    if (doc["HOMING_SPEED_Z"].is<int>()) {
        homingSpeedZ = doc["HOMING_SPEED_Z"].as<int>();
    }
    if (doc["SPEED_X"].is<int>()) {
        speedX = doc["SPEED_X"].as<int>();
    }
    if (doc["SPEED_Y"].is<int>()) {
        speedY = doc["SPEED_Y"].as<int>();
    }
    if (doc["SPEED_Z"].is<int>()) {
        speedZ = doc["SPEED_Z"].as<int>();
    }
    if (doc["POWER_LEVEL"].is<int>()) {
        powerLevel = doc["POWER_LEVEL"].as<int>();
    }
    if (doc["BALL_LOADING_STEPS"].is<int>()) {
        ballLoadingSteps = doc["BALL_LOADING_STEPS"].as<int>();
    }
    flashLED(ONBOARD_LED);
    sendLog(num);
    sendReadyStatus();
}

void sendLog(uint8_t num) {
    JsonDocument doc;
    doc["moveX"] = moveX;
    doc["moveY"] = moveY;
    doc["moveZ"] = moveZ;
    doc["directionX"] = directionX;
    doc["directionY"] = directionY;
    doc["directionZ"] = directionZ;
    doc["stepSize"] = stepSize;
    doc["solenoidActivated"] = solenoidActivated;
    doc["homeSwitch1Triggered"] = homeSwitch1Triggered;
    doc["homeSwitch2Triggered"] = homeSwitch2Triggered;
    doc["homeSwitch3Triggered"] = homeSwitch3Triggered;
    doc["axisDirectionX"] = axisDirectionX;
    doc["axisDirectionY"] = axisDirectionY;
    doc["axisDirectionZ"] = axisDirectionZ;
    doc["homingDirectionX"] = homingDirectionX;
    doc["homingDirectionY"] = homingDirectionY;
    doc["homingDirectionZ"] = homingDirectionZ;
    doc["powerLevel"] = powerLevel;
    doc["loadingSpeed"] = loadingSpeed;
    doc["ballLoadingSteps"] = ballLoadingSteps;
    doc["homingSpeedX"] = homingSpeedX;
    doc["homingSpeedY"] = homingSpeedY;
    doc["homingSpeedZ"] = homingSpeedZ;
    doc["speedX"] = speedX;
    doc["speedY"] = speedY;
    doc["speedZ"] = speedZ;
    doc["stepsX"] = stepsX;
    doc["stepsY"] = stepsY;
    doc["stepsZ"] = stepsZ;
    doc["pan"] = stepsX * stepsToDegreesX;
    doc["tilt"] = stepsY * stepsToDegreesY;
    doc["isMoving"] = isMoving;
    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.sendTXT(num, jsonString);
}

void sendReadyStatus() {
    JsonDocument doc;
    doc["status"] = "ready";
    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.broadcastTXT(jsonString);
}

void sendWebSocketLog(const char* message) {
    JsonDocument doc;
    doc["message"] = message;
    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.broadcastTXT(jsonString);
}

void flashLED(int ledPin) {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
}

void homeMachine() {
    homeAxis(1, homingSpeedX);
    homeAxis(2, homingSpeedY);
    homeAxis(3, homingSpeedZ);
}

void moveToPresetZeroPosition() {
    Serial.println("Moving to preset zero position...");
}

void homeAxis(int motor, int speed) {
    int directionPin, stepPin, limitSwitch;
    if (motor == 1) {
        directionPin = DIRECTION_PIN_1;
        stepPin = STEP_PIN_1;
        limitSwitch = LIMIT_SWITCH_1;
    } else if (motor == 2) {
        directionPin = DIRECTION_PIN_2;
        stepPin = STEP_PIN_2;
        limitSwitch = LIMIT_SWITCH_2;
    } else if (motor == 3) {
        directionPin = DIRECTION_PIN_3;
        stepPin = STEP_PIN_3;
        limitSwitch = LIMIT_SWITCH_3;
    } else {
        return;
    }
    digitalWrite(directionPin, homingDirectionX == 1 ? HIGH : LOW);
    while (digitalRead(limitSwitch) == LOW) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(speed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(100);
    }
    Serial.print("Motor ");
    Serial.print(motor);
    Serial.println(" homed.");
    if (motor == 2) {
        digitalWrite(directionPin, homingDirectionY == 1 ? LOW : HIGH);
        for (int i = 0; i < 4120; i++) {
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(speed);
            digitalWrite(stepPin, LOW);
            delayMicroseconds(100);
        }
        stepsY = 0;
    }
}

void sendI2CData() {
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.write(0x01);
    Wire.write(powerLevel >> 8);
    Wire.write(powerLevel & 0xFF);
    Wire.write(ballLoadingSteps >> 8);
    Wire.write(ballLoadingSteps & 0xFF);
    int error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("I2C transmission to slave successful.");
    } else {
        Serial.print("I2C transmission to slave failed with error: ");
        Serial.println(error);
    }
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.write(0x02);
    Wire.write(delayTimeOn >> 8);
    Wire.write(delayTimeOn & 0xFF);
    error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("I2C transmission of delayTimeOn to slave successful.");
    } else {
        Serial.print("I2C transmission of delayTimeOn to slave failed with error: ");
        Serial.println(error);
}
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.write(0x03);
    Wire.write(delayTimeOff >> 8);
    Wire.write(delayTimeOff & 0xFF);
    error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("I2C transmission of delayTimeOff to slave successful.");
    } else {
        Serial.print("I2C transmission of delayTimeOff to slave failed with error: ");
        Serial.println(error);
}
}

#endif

