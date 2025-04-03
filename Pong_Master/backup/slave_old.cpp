#include <WiFi.h>
#include <Wire.h>

// Wi-Fi Credentials
const char* ssid = "BARN WIFI";
const char* password = "512939692b";

// Solenoid Pin
#define SOLENOID_PIN D3

// LED Pin
#define ONBOARD_LED D0  // GPIO 16

// Predefined wait periods for each speed setting
const int waitPeriods[] = {1000, 900, 800, 700, 600, 500, 400, 300, 200, 100};

// Function declarations
void receiveEvent(int howMany);
void setup();
void loop();

// I2C event handler
void receiveEvent(int howMany) {
  while (Wire.available()) {
    int speedLevel = Wire.read();
    if (speedLevel >= 1 && speedLevel <= 10) {
      // Wait for the specific duration and then release the solenoid
      delay(waitPeriods[speedLevel - 1]);
      digitalWrite(SOLENOID_PIN, HIGH);
      delay(100); // Keep solenoid ON for 100ms
      digitalWrite(SOLENOID_PIN, LOW);
    }
  }
}

void setup() {
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH); // LED OFF initially
  digitalWrite(SOLENOID_PIN, LOW); // Solenoid OFF initially
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ONBOARD_LED, LOW);
    delay(200);
    digitalWrite(ONBOARD_LED, HIGH);
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize I2C as slave with address 8
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
}

void loop() {
  // Nothing to do here, I2C events are handled in the receiveEvent function
}
