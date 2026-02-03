/**
 * @file MyRobot-V1.ino
 * @brief Arduino-based robot controller with serial command interface
 */

#include <Arduino.h>
#include <WiFiS3.h>

// ===== WIFI =====
const char* ssid = "RobotNet";
const char* password = "robot1234";
WiFiServer server(8080);

// ===== MOTOR PINS =====
int enA = 9;
int in1 = 8;
int in2 = 7;

int enB = 5;
int in3 = 2;
int in4 = 4;

// ===== ENCODERS =====
int encA = 10;
int encB = 11;

int countA = 0;
int countB = 0;

float distanceA = 0;
float distanceB = 0;

// 🔧 REQUIRED for edge detection
int lastEncA = HIGH;
int lastEncB = HIGH;

// ===== SPEED =====
int currentSpeed = 150;
float rightSpeedScaled = 150;
const int SPEED_INCREMENT = 25;
const int MAX_SPEED = 255;

// ===== PROPORTIONAL CONTROL =====
const float Kp = 5.0;  // tweak this for responsiveness

// ===== SETUP =====
void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);

  stop();

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Server started");
}

// ===== LOOP =====
void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Client connected");

    while (client.connected()) {

      // ===== EDGE DETECTION =====
      int encAIn = digitalRead(encA);
      int encBIn = digitalRead(encB);

      if (lastEncA == LOW && encAIn == HIGH) countA++;
      if (lastEncB == LOW && encBIn == HIGH) countB++;

      lastEncA = encAIn;
      lastEncB = encBIn;

      // ===== UPDATE DISTANCES =====
      distanceA = countA * (8.0 / 20.0);
      distanceB = countB * (8.0 / 20.0);

      // ===== RIGHT WHEEL PROPORTIONAL CONTROL =====
      float error = distanceA - distanceB;  // positive if left ahead
      rightSpeedScaled = currentSpeed + Kp * error; 
      rightSpeedScaled = constrain(rightSpeedScaled, 0, MAX_SPEED); // never exceed MAX_SPEED

      // ===== READ CLIENT COMMAND =====
      if (client.available()) {
        char command = client.read();
        processCommand(command);
        client.print("OK:");
        client.println(command);
      }

      // ===== APPLY DRIVE SPEEDS IF MOVING FORWARD/BACKWARD =====
      // done in drive() or backwards()
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}

// ===== COMMAND HANDLER =====
void processCommand(char command) {
  Serial.print("Received: ");
  Serial.println(command);

  switch (command) {
    case 'w': drive(); break;
    case 's': backwards(); break;
    case 'a': turnLeft(); break;
    case 'd': turnRight(); break;
    case 'j': speedUp(); break;
    case 'k': slowDown(); break;
    case 'q': spinLeftInPlace(); break;
    case 'e': spinRightInPlace(); break;

    case ' ': {
      stop();

      // Print distances after scaling
      distanceA = countA * (8.0 / 20.0);
      distanceB = countB * (8.0 / 20.0);

      Serial.print("Left Wheel Distance: ");
      Serial.println(distanceA);
      Serial.print("Right Wheel Distance: ");
      Serial.println(distanceB);
      Serial.print("Right Wheel Scaled Speed: ");
      Serial.println(rightSpeedScaled);

      // Reset encoder counts
      countA = 0;
      countB = 0;

      // Reset right speed
      rightSpeedScaled = currentSpeed;
      break;
    }

    case '?':
      Serial.print("Speed: ");
      Serial.println(currentSpeed);
      break;

    default:
      Serial.print("Unknown command: ");
      Serial.println(command);
      break;
  }
}

// ===== MOTOR FUNCTIONS =====
void drive() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, currentSpeed);
  analogWrite(enB, rightSpeedScaled);

  Serial.println("FORWARD");
}

void backwards() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, currentSpeed);
  analogWrite(enB, rightSpeedScaled);

  Serial.println("BACKWARD");
}

void stop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  analogWrite(enA, 0);
  analogWrite(enB, 0);

  Serial.println("STOP");
}

void turnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, currentSpeed / 2);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, currentSpeed);
}

void turnRight() {
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, currentSpeed / 2);

  digitalWrite(in1, LOW);
  analogWrite(in2, HIGH);
  analogWrite(enA, currentSpeed);
}

void speedUp() {
  currentSpeed = min(currentSpeed + SPEED_INCREMENT, MAX_SPEED);
  rightSpeedScaled = currentSpeed;
}

void slowDown() {
  currentSpeed = max(currentSpeed - SPEED_INCREMENT, 0);
  rightSpeedScaled = currentSpeed;
  if (currentSpeed == 0) stop();
}

void spinLeftInPlace() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);

  Serial.println("Spinning LEFT");
}

void spinRightInPlace() {
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);

  Serial.println("Spinning RIGHT");
}
