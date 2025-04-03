#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char* ssid = "BARN WIFI";
const char* password = "512939692b";

WebSocketsServer webSocket = WebSocketsServer(81);

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

int homingSpeedX = 1000; // Default homing speed for X
int homingSpeedY = 1000; // Default homing speed for Y
int homingSpeedZ = 220;  // Default homing speed for Z

int homingDirectionX = -1; // -1 for backward, 1 for forward
int homingDirectionY = -1; // -1 for backward, 1 for forward
int homingDirectionZ = -1; // -1 for backward, 1 for forward

int powerLevel = 2500; // Default power level
int loadingSpeed = 220; // Default loading speed
int ballLoadingSteps = 4200; // Default number of steps for ball loading

int speedX = 1000; // Default speed for X
int speedY = 2000; // Default speed for Y
int speedZ = 220;  // Default speed for Z

int stepsX = 0; // Steps moved for X
int stepsY = 0; // Steps moved for Y
int stepsZ = 0; // Steps moved for Z

const float stepsToDegreesX = 0.1; // Conversion factor for X axis
const float stepsToDegreesY = 0.1; // Conversion factor for Y axis

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleWebSocketMessage(uint8_t num, uint8_t * payload);
void stepMotor(int motor, int direction, int steps, int delayTime);
void stopMotors();
void flashLED(int ledPin);
void throwRoutine(int newPowerLevel);
void testSolenoid();
void sendLog(uint8_t num);
void sendWebSocketLog(const char* message);
void homeAxis(int motor, int speed);
void sendReadyStatus();
void homeMachine();
void moveToPresetZeroPosition();

void setup() {
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(ENABLE_PIN_1, OUTPUT);
  pinMode(ENABLE_PIN_2, OUTPUT);
  pinMode(ENABLE_PIN_3, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);
  digitalWrite(SOLENOID_PIN, LOW);
  digitalWrite(ENABLE_PIN_1, LOW);
  digitalWrite(ENABLE_PIN_2, LOW);
  digitalWrite(ENABLE_PIN_3, LOW);

  pinMode(DIRECTION_PIN_1, OUTPUT);
  pinMode(STEP_PIN_1, OUTPUT);
  pinMode(DIRECTION_PIN_2, OUTPUT);
  pinMode(STEP_PIN_2, OUTPUT);
  pinMode(DIRECTION_PIN_3, OUTPUT);
  pinMode(STEP_PIN_3, OUTPUT);

  pinMode(LIMIT_SWITCH_1, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_2, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_3, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ONBOARD_LED, LOW);
    delay(200);
    digitalWrite(ONBOARD_LED, HIGH);
    delay(200);
  }
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Wire.begin();
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.endTransmission();
  delay(2000);
}

void loop() {
  webSocket.loop();

  if (!isMoving) {
    if (moveX) {
      isMoving = true;
      digitalWrite(ENABLE_PIN_1, HIGH);
      stepMotor(1, directionX * axisDirectionX, stepSize, speedX);
      digitalWrite(ENABLE_PIN_1, LOW);
      moveX = false;
      flashLED(ONBOARD_LED);
      isMoving = false;
      sendLog(0); // Send updated log after movement
      sendReadyStatus(); // Send ready status
    }
    if (moveY) {
      isMoving = true;
      digitalWrite(ENABLE_PIN_2, HIGH);
      stepMotor(2, directionY * axisDirectionY, stepSize, speedY);
      digitalWrite(ENABLE_PIN_2, LOW);
      moveY = false;
      flashLED(ONBOARD_LED);
      isMoving = false;
      sendLog(0); // Send updated log after movement
      sendReadyStatus(); // Send ready status
    }
    if (moveZ) {
      isMoving = true;
      digitalWrite(ENABLE_PIN_3, HIGH);
      stepMotor(3, directionZ * axisDirectionZ, stepSize, speedZ);
      digitalWrite(ENABLE_PIN_3, LOW);
      moveZ = false;
      flashLED(ONBOARD_LED);
      isMoving = false;
      sendLog(0); // Send updated log after movement
      sendReadyStatus(); // Send ready status
    }
  }

  if (solenoidActivated) {
    testSolenoid();
    solenoidActivated = false;
    sendReadyStatus(); // Send ready status
  }

  delay(10);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      flashLED(ONBOARD_LED);
      if (!isHomed) {
        homeMachine();
        moveToPresetZeroPosition();
        isHomed = true;
      }
      sendLog(num); // Send initial log with default values
      break;
    case WStype_TEXT:
      handleWebSocketMessage(num, payload);
      break;
    case WStype_ERROR:
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
    throwRoutine(powerLevel); // Directly call the throw routine
  }
  if (doc["TEST_SOLENOID"].is<bool>() && doc["TEST_SOLENOID"].as<bool>()) {
    solenoidActivated = true;
  }
  if (doc["HOME_X"].is<bool>() && doc["HOME_X"].as<bool>()) {
    homeAxis(1, homingSpeedX);
    stepsX = 0; // Reset steps for X
    sendReadyStatus(); // Send ready status
  }
  if (doc["HOME_Y"].is<bool>() && doc["HOME_Y"].as<bool>()) {
    homeAxis(2, homingSpeedY);
    stepsY = 0; // Reset steps for Y
    sendReadyStatus(); // Send ready status
  }
  if (doc["HOME_Z"].is<bool>() && doc["HOME_Z"].as<bool>()) {
    homeAxis(3, homingSpeedZ);
    stepsZ = 0; // Reset steps for Z
    sendReadyStatus(); // Send ready status
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
  doc["pan"] = stepsX * stepsToDegreesX; // Convert steps to degrees for pan
  doc["tilt"] = stepsY * stepsToDegreesY; // Convert steps to degrees for tilt
  doc["isMoving"] = isMoving; // Send the movement state
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

void stepMotor(int motor, int direction, int steps, int delayTime) {
  char logMessage[50];
  sprintf(logMessage, "Motor %d: Delay Time = %d", motor, delayTime);
  sendWebSocketLog(logMessage);
  int accelSteps = steps * 0.01; // 1% of total steps for acceleration
  int constSpeedSteps = steps * 0.98; // 98% of total steps for constant speed
  int decelSteps = steps * 0.01; // 1% of total steps for deceleration
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
    delayMicroseconds(rampDelay);
    rampDelay = max(rampDelay - 10, delayTime);
  }
  for (int i = 0; i < constSpeedSteps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(delayTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(delayTime);
  }
  for (int i = 0; i < decelSteps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(rampDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(rampDelay);
    rampDelay += 10;
  }
  // Update steps moved
  if (motor == 1) {
    stepsX += direction * steps;
  } else if (motor == 2) {
    stepsY += direction * steps;
  } else if (motor == 3) {
    stepsZ += direction * steps;
  }
}

void stopMotors() {
  moveX = moveY = moveZ = false;
}

void flashLED(int ledPin) {
  digitalWrite(ledPin, LOW);
  delay(100);
  digitalWrite(ledPin, HIGH);
}

void throwRoutine(int newPowerLevel) {
  sendWebSocketLog("Starting throw routine...");
  // Check if Z-axis is already homed
  if (digitalRead(LIMIT_SWITCH_3) == LOW) {
    homeAxis(3, homingSpeedZ);
  }

  // Send power level and ball loading steps to slave
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.write(0x01); // Register for power level
  Wire.write(newPowerLevel >> 8); // Send the high byte of power level
  Wire.write(newPowerLevel & 0xFF); // Send the low byte of power level
  Wire.write(ballLoadingSteps >> 8); // Send the high byte of ball loading steps
  Wire.write(ballLoadingSteps & 0xFF); // Send the low byte of ball loading steps
  int error = Wire.endTransmission();
  if (error == 0) {
    Serial.println("I2C transmission to slave successful.");
  } else {
    Serial.print("I2C transmission to slave failed with error: ");
    Serial.println(error);
  }

  // Move backwards 'power level' at 'loading speed'
  digitalWrite(ENABLE_PIN_3, HIGH);
  stepMotor(3, -1 * axisDirectionZ, newPowerLevel, loadingSpeed);
  sendWebSocketLog("Motor 3 moved backward by power level.");
  // Activate solenoid
  digitalWrite(SO
