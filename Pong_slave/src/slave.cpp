#include <Arduino.h>
#include <Wire.h>

#define SOLENOID_PIN 26
#define ONBOARD_LED 2
#define I2C_DEV_ADDR 0x55

#define ENABLE_PIN_SLAVE1 18
#define DIRECTION_PIN_SLAVE1 19
#define STEP_PIN_SLAVE1 3
#define LIMIT_SWITCH_SLAVE1 34

int powerLevel = 2500;
int loadingSpeed = 280;
int ballLoadingSteps = 4250;
bool throwSignalReceived = false;
bool motorMoved = false;
int delayTimeOn = 280;
int delayTimeOff = 100;

void receiveEvent(int howMany);
void stepMotor(int direction, int steps, int delayTimeOn, int delayTimeOff);
void homeMotor();

void receiveEvent(int howMany) {
    Serial.print("Received I2C message with ");
    Serial.print(howMany);
    Serial.println(" bytes.");

    if (howMany == 5) {
        int registerAddress = Wire.read();
        Serial.print("Register address: ");
        Serial.println(registerAddress, HEX);

        if (registerAddress == 0x01) {
            powerLevel = (Wire.read() << 8) | Wire.read();
            ballLoadingSteps = (Wire.read() << 8) | Wire.read();
            Serial.print("Received I2C message with power level: ");
            Serial.print(powerLevel);
            Serial.print(" and ball loading steps: ");
            Serial.println(ballLoadingSteps);
            throwSignalReceived = true;
        } else {
            Serial.println("Unexpected register address received.");
        }
    } else if (howMany == 3) {
        int registerAddress = Wire.read();
        Serial.print("Register address: ");
        Serial.println(registerAddress, HEX);

        if (registerAddress == 0x02) {
            delayTimeOn = (Wire.read() << 8) | Wire.read();
            Serial.print("Received I2C message with delayTimeOn: ");
            Serial.println(delayTimeOn);
        } else if (registerAddress == 0x03) {
            delayTimeOff = (Wire.read() << 8) | Wire.read();
            Serial.print("Received I2C message with delayTimeOff: ");
            Serial.println(delayTimeOff);
        } else {
            Serial.println("Unexpected register address received.");
        }
    } else {
        Serial.print("Unexpected number of bytes received: ");
        Serial.println(howMany);
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Slave setup started.");
    pinMode(ONBOARD_LED, OUTPUT);
    pinMode(SOLENOID_PIN, OUTPUT);
    pinMode(ENABLE_PIN_SLAVE1, OUTPUT);
    pinMode(DIRECTION_PIN_SLAVE1, OUTPUT);
    pinMode(STEP_PIN_SLAVE1, OUTPUT);
    pinMode(LIMIT_SWITCH_SLAVE1, INPUT_PULLUP);
    digitalWrite(ONBOARD_LED, HIGH);
    digitalWrite(SOLENOID_PIN, LOW);
    digitalWrite(ENABLE_PIN_SLAVE1, LOW);
    Wire.begin(I2C_DEV_ADDR);
    Wire.onReceive(receiveEvent);
    Serial.println("Slave setup complete.");
}

void loop() {
    Serial.println("Entering loop.");
    if (throwSignalReceived && !motorMoved) {
        Serial.println("Throw signal received and motor not moved.");
        int presetDelay = 20;
        int variableDelay = (powerLevel / 5000.0) * 3000;
        int totalDelay = presetDelay + variableDelay;
        Serial.print("Calculated delay: ");
        Serial.println(totalDelay);
        delay(totalDelay);
        Serial.println("Delay completed. Moving motor.");
        digitalWrite(ENABLE_PIN_SLAVE1, HIGH);
        stepMotor(1, ballLoadingSteps, delayTimeOn, delayTimeOff);
        digitalWrite(ENABLE_PIN_SLAVE1, LOW);
        motorMoved = true;
        Serial.println("Motor moved. Waiting for 100ms before homing.");
        delay(100);
        Serial.println("Waiting completed. Homing motor.");
        homeMotor();
        throwSignalReceived = false;
        motorMoved = false;
        Serial.println("Motor homed. Throw routine completed.");
    }
    delay(100);
}

void stepMotor(int direction, int steps, int delayTimeOn, int delayTimeOff) {
    Serial.print("Stepping motor with direction: ");
    Serial.print(direction);
    Serial.print(", steps: ");
    Serial.print(steps);
    Serial.print(", delayOn: ");
    Serial.print(delayTimeOn);
    Serial.print(", delayOff: ");
    Serial.println(delayTimeOff);
    int accelSteps = steps * 0.01;
    int constSpeedSteps = steps * 0.98;
    int decelSteps = steps * 0.01;
    int rampDelayOn = delayTimeOn;
    int rampDelayOff = delayTimeOff;
    digitalWrite(DIRECTION_PIN_SLAVE1, direction == 1 ? HIGH : LOW);
    for (int i = 0; i < accelSteps; i++) {
        digitalWrite(STEP_PIN_SLAVE1, HIGH);
        delayMicroseconds(rampDelayOn);
        digitalWrite(STEP_PIN_SLAVE1, LOW);
        delayMicroseconds(rampDelayOff);
        rampDelayOn = max(rampDelayOn - 10, delayTimeOn);
        rampDelayOff = max(rampDelayOff - 5, delayTimeOff);
    }
    for (int i = 0; i < constSpeedSteps; i++) {
        digitalWrite(STEP_PIN_SLAVE1, HIGH);
        delayMicroseconds(delayTimeOn);
        digitalWrite(STEP_PIN_SLAVE1, LOW);
        delayMicroseconds(delayTimeOff);
    }
    for (int i = 0; i < decelSteps; i++) {
        digitalWrite(STEP_PIN_SLAVE1, HIGH);
        delayMicroseconds(rampDelayOn);
        digitalWrite(STEP_PIN_SLAVE1, LOW);
        delayMicroseconds(rampDelayOff);
        rampDelayOn += 10;
        rampDelayOff += 5;
    }
}

void homeMotor() {
    Serial.println("Homing motor.");
    digitalWrite(ENABLE_PIN_SLAVE1, HIGH);
    digitalWrite(DIRECTION_PIN_SLAVE1, LOW);
    while (digitalRead(LIMIT_SWITCH_SLAVE1) == LOW) {
        digitalWrite(STEP_PIN_SLAVE1, HIGH);
        delayMicroseconds(280);
        digitalWrite(STEP_PIN_SLAVE1, LOW);
        delayMicroseconds(140);
    }
    digitalWrite(ENABLE_PIN_SLAVE1, LOW);
    Serial.println("Motor homed.");
}