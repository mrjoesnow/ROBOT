#ifndef COMMON_H
#define COMMON_H

#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>

// Pin definitions
#define ENABLE_PIN_1 18
#define DIRECTION_PIN_1 19
#define STEP_PIN_1 3
#define ENABLE_PIN_2 5
#define DIRECTION_PIN_2 17
#define STEP_PIN_2 16
#define ENABLE_PIN_3 26
#define DIRECTION_PIN_3 27
#define STEP_PIN_3 14
#define SOLENOID_PIN 23
#define ONBOARD_LED 2
#define LIMIT_SWITCH_1 34
#define LIMIT_SWITCH_2 15
#define LIMIT_SWITCH_3 33
#define I2C_DEV_ADDR 0x55

// Function declarations
void throwRoutine(int newPowerLevel);
void stepMotor(int motor, int direction, int steps, int delayTime);
void stopMotors();
void testSolenoid();
void sendWebSocketLog(const char* message);
void flashLED(int ledPin);
void homeMachine();
void moveToPresetZeroPosition();
void homeAxis(int motor, int speed);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleWebSocketMessage(uint8_t num, uint8_t * payload);
void sendLog(uint8_t num);
void sendReadyStatus();
void sendI2CData();
void setupMotors();
void handleMotorControl();
// External variables
extern const char* ssid;
extern const char* password;
extern WebSocketsServer webSocket;
extern int stepSize;
extern bool moveX, moveY, moveZ;
extern int directionX, directionY, directionZ;
extern bool isMoving;
extern bool solenoidActivated;
extern bool homeSwitch1Triggered, homeSwitch2Triggered, homeSwitch3Triggered;
extern bool isHomed;
extern int axisDirectionX, axisDirectionY, axisDirectionZ;
extern int homingSpeedX, homingSpeedY, homingSpeedZ;
extern int homingDirectionX, homingDirectionY, homingDirectionZ;
extern int powerLevel, loadingSpeed, ballLoadingSteps;
extern int speedX, speedY, speedZ;
extern int stepsX, stepsY, stepsZ;
extern const float stepsToDegreesX, stepsToDegreesY;
extern int delayTimeOn, delayTimeOff;

#endif // COMMON_H