/**
 * @file MyRobot-V1.ino
 * @brief Arduino-based robot controller with serial + WiFi command interface
 *
 * Hardware:
 * - Arduino UNO R4 WiFi
 * - Dual H-bridge motor driver (L298N / L293D)
 * - Two DC motors with encoders
 *
 * Commands:
 * w = forward
 * s = backward
 * a = left
 * d = right
 * j = speed up
 * k = speed down
 * q = spin left
 * e = spin right
 * space = stop
 */

#include <Arduino.h>
#include <WiFiS3.h>

// ===== WIFI CREDENTIALS =====
const char* ssid     = "RobotNet";
const char* password = "robot1234";

// ===== SERVER SETTINGS =====
const int port = 8080;
WiFiServer server(port);

// ===== MOTOR PIN DEFINITIONS =====
int enA = 9;   // Left motor PWM
int in1 = 8;
int in2 = 7;

int enB = 5;   // Right motor PWM
int in3 = 2;
int in4 = 4;

// ===== SPEED SETTINGS =====
int currentSpeed = 150;
const int SPEED_INCREMENT = 25;
const int MAX_SPEED = 255;

// ===== SETUP =====
void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);\

  stop();

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Server started");
}

// ===== MAIN LOOP =====
void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    while (client.connected()) {
      if (client.available()) {
        char command = client.read();
        processCommand(command);
        client.print("OK:");
        client.println(command);
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

// ===== COMMAND PROCESSOR =====
void processCommand(char command) {
  Serial.print("Command: ");
  Serial.println(command);

  switch (command) {
    case 'w': drive(); break;
    case 's': backwards(); break;
    case 'a': turnLeft(); break;
    case 'd': turnRight(); break;
    case 'q': spinLeftInPlace(); break;
    case 'e': spinRightInPlace(); break;
    case 'j': speedUp(); break;
    case 'k': slowDown(); break;
    case ' ':
      stop();
      break;
    default:
      Serial.println("Unknown command");
      break;
  }
}


