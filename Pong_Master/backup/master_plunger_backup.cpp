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
#define SOLENOID_PIN 13
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

int powerLevel = 1000; // New variable for power level
int loadingSpeed = 220; // Default loading speed

int speedX = 1000; // Default speed for X
int speedY = 2000; // Default speed for Y
int speedZ = 220;  // Default speed for Z

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
    }
    if (moveY) {
      isMoving = true;
      digitalWrite(ENABLE_PIN_2, HIGH);
      stepMotor(2, directionY * axisDirectionY, stepSize, speedY);
      digitalWrite(ENABLE_PIN_2, LOW);
      moveY = false;
      flashLED(ONBOARD_LED);
      isMoving = false;
    }
    if (moveZ) {
      isMoving = true;
      digitalWrite(ENABLE_PIN_3, HIGH);
      stepMotor(3, directionZ * axisDirectionZ, stepSize, speedZ);
      digitalWrite(ENABLE_PIN_3, LOW);
      moveZ = false;
      flashLED(ONBOARD_LED);
      isMoving = false;
    }
  }

  if (solenoidActivated) {
    testSolenoid();
    solenoidActivated = false;
  }

  delay(10);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      flashLED(ONBOARD_LED);
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
    throwRoutine(powerLevel); // Directly call the throw routine
  }
  if (doc["TEST_SOLENOID"].is<bool>() && doc["TEST_SOLENOID"].as<bool>()) {
    solenoidActivated = true;
  }
  if (doc["HOME_X"].is<bool>() && doc["HOME_X"].as<bool>()) {
    homeAxis(1, homingSpeedX);
  }
  if (doc["HOME_Y"].is<bool>() && doc["HOME_Y"].as<bool>()) {
    homeAxis(2, homingSpeedY);
  }
  if (doc["HOME_Z"].is<bool>() && doc["HOME_Z"].as<bool>()) {
    homeAxis(3, homingSpeedZ);
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
  doc["homingSpeedX"] = homingSpeedX;
  doc["homingSpeedY"] = homingSpeedY;
  doc["homingSpeedZ"] = homingSpeedZ;
  doc["speedX"] = speedX;
  doc["speedY"] = speedY;
  doc["speedZ"] = speedZ;
  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.sendTXT(num, jsonString);
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

  // Move backwards 'power level' at 'loading speed'
  digitalWrite(ENABLE_PIN_3, HIGH);
  stepMotor(3, -1 * axisDirectionZ, newPowerLevel, loadingSpeed);
  sendWebSocketLog("Motor 3 moved backward by power level.");

  // Activate solenoid
  digitalWrite(SOLENOID_PIN, HIGH);
  delay(100); // Adjust delay as needed
  digitalWrite(SOLENOID_PIN, LOW);

  delay(100); // Additional delay before homing Z-axis again

  // Home Z-axis again
  homeAxis(3, homingSpeedZ);

  sendWebSocketLog("Throw routine completed.");
  digitalWrite(ENABLE_PIN_3, LOW);
}

void testSolenoid() {
  digitalWrite(SOLENOID_PIN, HIGH);
  delay(1000);
  digitalWrite(SOLENOID_PIN, LOW);
}

void homeAxis(int motor, int speed) {
  int steps = 0;
  int limitSwitchPin, enablePin, directionPin, stepPin;
  int homingDirection;

  if (motor == 1) {
    limitSwitchPin = LIMIT_SWITCH_1;
    enablePin = ENABLE_PIN_1;
    directionPin = DIRECTION_PIN_1;
    stepPin = STEP_PIN_1;
    homingDirection = homingDirectionX;
  } else if (motor == 2) {
    limitSwitchPin = LIMIT_SWITCH_2;
    enablePin = ENABLE_PIN_2;
    directionPin = DIRECTION_PIN_2;
    stepPin = STEP_PIN_2;
    homingDirection = homingDirectionY;
  } else if (motor == 3) {
    limitSwitchPin = LIMIT_SWITCH_3;
    enablePin = ENABLE_PIN_3;
    directionPin = DIRECTION_PIN_3;
    stepPin = STEP_PIN_3;
    homingDirection = homingDirectionZ;
  } else {
    return;
  }

  digitalWrite(enablePin, HIGH);
  digitalWrite(directionPin, homingDirection == 1 ? HIGH : LOW);
  while (digitalRead(limitSwitchPin) == LOW) {
    for (int i = 0; i < 10; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(speed);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(speed);
    }

    if (digitalRead(limitSwitchPin) == HIGH) {
      break;
    }
  }
  digitalWrite(enablePin, LOW);
}