#ifndef MOTOR_CONTROL_CPP
#define MOTOR_CONTROL_CPP

#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "common.h"

int stepsX = 0;
int stepsY = 0;
int stepsZ = 0;

void setupMotors() {
    Serial.println("Setting up motors...");
    pinMode(DIRECTION_PIN_1, OUTPUT);
    pinMode(STEP_PIN_1, OUTPUT);
    pinMode(DIRECTION_PIN_2, OUTPUT);
    pinMode(STEP_PIN_2, OUTPUT);
    pinMode(DIRECTION_PIN_3, OUTPUT);
    pinMode(STEP_PIN_3, OUTPUT);
    pinMode(LIMIT_SWITCH_1, INPUT_PULLUP);
    pinMode(LIMIT_SWITCH_2, INPUT_PULLUP);
    pinMode(LIMIT_SWITCH_3, INPUT_PULLUP);
    pinMode(SOLENOID_PIN, OUTPUT);
    pinMode(ENABLE_PIN_3, OUTPUT);
    Serial.println("Motor setup complete.");
}

void handleMotorControl() {
    if (moveX) {
        stepMotor(1, directionX, stepSize, speedX);
        moveX = false;
    }
    if (moveY) {
        stepMotor(2, directionY, stepSize, speedY);
        moveY = false;
    }
    if (moveZ) {
        stepMotor(3, directionZ, stepSize, speedZ);
        moveZ = false;
    }
}

void stepMotor(int motor, int direction, int steps, int delayTime) {
    char logMessage[50];
    sprintf(logMessage, "Motor %d: Delay Time = %d", motor, delayTime);
    sendWebSocketLog(logMessage);
    int accelSteps = steps * 0.01;
    int constSpeedSteps = steps * 0.98;
    int decelSteps = steps * 0.01;
    int rampDelay = delayTime;
    int directionPin, stepPin;
    if (motor == 1) {
        directionPin = DIRECTION_PIN_1;
        stepPin = STEP_PIN_1;
    } else if (motor == 2) {
        directionPin = DIRECTION_PIN_2;
        stepPin = STEP_PIN_2;
    } else if (motor == 3) {
        directionPin = DIRECTION_PIN_3;
        stepPin = STEP_PIN_3;
    } else {
        return;
    }
    digitalWrite(directionPin, direction == 1 ? HIGH : LOW);
    for (int i = 0; i < accelSteps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(rampDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(100);
        rampDelay = max(rampDelay - 10, delayTime);
    }
    for (int i = 0; i < constSpeedSteps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(delayTime);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(100);
    }
    for (int i = 0; i < decelSteps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(rampDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(100);
        rampDelay += 10;
    }
    if (motor == 1) {
        stepsX += direction * steps;
    } else if (motor == 2) {
        stepsY += direction * steps;
    } else if (motor == 3) {
        stepsZ += direction * steps;
    }
    sendReadyStatus(); // Send ready status after motor movement
}

void moveMotorX(int steps) {
    stepsX += steps;
    stepMotor(1, steps > 0 ? 1 : -1, abs(steps), speedX);
}

void moveMotorY(int steps) {
    stepsY += steps;
    stepMotor(2, steps > 0 ? 1 : -1, abs(steps), speedY);
}

void moveMotorZ(int steps) {
    stepsZ += steps;
    stepMotor(3, steps > 0 ? 1 : -1, abs(steps), speedZ);
}

void homeMotorX() {
    stepsX = 0;
    homeAxis(1, homingSpeedX);
    sendReadyStatus(); // Send ready status after homing X
}

void homeMotorY() {
    stepsY = 0;
    homeAxis(2, homingSpeedY);
    sendReadyStatus(); // Send ready status after homing Y
}

void homeMotorZ() {
    stepsZ = 0;
    homeAxis(3, homingSpeedZ);
    sendReadyStatus(); // Send ready status after homing Z
}

int getStepsX() {
    return stepsX;
}

int getStepsY() {
    return stepsY;
}

int getStepsZ() {
    return stepsZ;
}

void stopMotors() {
    moveX = moveY = moveZ = false;
}

void throwRoutine(int newPowerLevel) {
    sendWebSocketLog("Starting throw routine...");
    if (digitalRead(LIMIT_SWITCH_3) == LOW) {
    homeAxis(3, homingSpeedZ);
}

    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.write(0x01);
    Wire.write(newPowerLevel >> 8);
    Wire.write(newPowerLevel & 0xFF);
    Wire.write(ballLoadingSteps >> 8);
    Wire.write(ballLoadingSteps & 0xFF);
    Wire.write(delayTimeOn >> 8);
    Wire.write(delayTimeOn & 0xFF);
    Wire.write(delayTimeOff >> 8);
    Wire.write(delayTimeOff & 0xFF);
    int error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("I2C transmission to slave successful.");
    } else {
        Serial.print("I2C transmission to slave failed with error: ");
        Serial.println(error);
}

    digitalWrite(ENABLE_PIN_3, HIGH);
    stepMotor(3, -1 * axisDirectionZ, newPowerLevel, loadingSpeed);
    sendWebSocketLog("Motor 3 moved backward by power level.");
    digitalWrite(SOLENOID_PIN, HIGH);
    delay(100);
    digitalWrite(SOLENOID_PIN, LOW);
    delay(100);
    homeAxis(3, homingSpeedZ);
    sendWebSocketLog("Throw routine completed.");
    digitalWrite(ENABLE_PIN_3, LOW);
    sendReadyStatus();
}

void testSolenoid() {
    digitalWrite(SOLENOID_PIN, HIGH);
    delay(100);
    digitalWrite(SOLENOID_PIN, LOW);
}

#endif // MOTOR_CONTROL_CPP