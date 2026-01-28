/**
 * @file MyRobot-V1.ino
 * @brief Arduino-based robot controller with serial command interface
 * 
 * This sketch controls a two-wheeled robot using an H-bridge motor driver.
 * It accepts serial commands for movement and speed control.
 * 
 * Hardware Requirements:
 * - Arduino board (Uno, Nano, Mega, etc.)
 * - Dual H-bridge motor driver (L298N, L293D, etc.)
 * - Two DC motors with wheels
 * - Power supply for motors
 * 
 * Serial Commands:
 * - 'w': Move forward
 * - 's': Move backward
 * - 'a': Turn left (pivot)
 * - 'd': Turn right (pivot)
 * - 'j': Increase speed
 * - 'k': Decrease speed
 * - ' ': Stop motors
 * 
 * Motor Connections:
 * - Motor A (Left): enA=9, in1=8, in2=7
 * - Motor B (Right): enB=5, in3=2, in4=4
 * 
 * @author Joshua, Waleed, Jaemoon, Morgan
 * @version 1.0
 * @date 2026
 */

#include <Arduino.h>
#include <WiFiS3.h>  // WiFi library for UNO R4 WiFi

// ===== WIFI CREDENTIALS =====
/** @brief WiFi network name - CHANGE THIS */
const char* ssid = "RobotNet";
/** @brief WiFi password - CHANGE THIS */
const char* password = "robot1234";

// ===== SERVER SETTINGS =====
/** @brief TCP port for robot control server */
const int port = 8080;
/** @brief WiFi server object */
WiFiServer server(port);

// ===== MOTOR PIN DEFINITIONS =====
/** @brief Enable pin for Motor A (PWM) - left wheel */
int enA = 9;
/** @brief Direction pin 1 for Motor A */
int in1 = 8;
/** @brief Direction pin 2 for Motor A */
int in2 = 7;

/** @brief Enable pin for Motor B (PWM) - right wheel */
int enB = 5;
/** @brief Direction pin 1 for Motor B */
int in3 = 2;
/** @brief Direction pin 2 for Motor B */
int in4 = 4;

// ===== SPEED SETTINGS =====
/** @brief Current motor speed (0-255) */
int currentSpeed = 150;
/** @brief Speed increment/decrement amount */
const int SPEED_INCREMENT = 25;
/** @brief Maximum PWM value */
const int MAX_SPEED = 255;

// ===== SETUP =====
void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  while (!Serial);  // Wait for serial connection
  
  // Configure motor pins
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  // Stop motors initially
  stop();
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Start the server
  server.begin();
  Serial.print("Server started on port ");
  Serial.println(port);
  Serial.println("Ready for commands...");
}

// ===== MAIN LOOP =====
void loop() {
  // Check for incoming client connections
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected");
    
    // Handle client while connected
    while (client.connected()) {
      if (client.available()) {
        char command = client.read();
        processCommand(command);
        
        // Send acknowledgment back to client
        client.print("OK:");
        client.println(command);
      }
    }
    
    // Client disconnected
    client.stop();
    Serial.println("Client disconnected");
  }
}

// ===== COMMAND PROCESSOR =====
/**
 * @brief Processes incoming commands from WiFi client
 * @param command Single character command from client
 */
void processCommand(char command) {
  Serial.print("Received: ");
  Serial.println(command);
  
  switch(command) {
    case 'w':  // Forward
      drive();
      break;
    case 's':  // Backward
      backwards();
      break;
    case 'a':  // Turn left
      turnLeft();
      break;
    case 'd':  // Turn right
      turnRight();
      break;
    case 'j':  // Speed up
      speedUp();
      break;
    case 'k':  // Slow down
      slowDown();
      break;
    case 'q':  // Spin left
      spinLeftInPlace();
      break;
    case 'e':  // Spin right
      spinRightInPlace();
      break;
    case ' ':  // Stop
      stop();
      break;
    case '?':  // Status request
      Serial.print("Speed: ");
      Serial.println(currentSpeed);
      break;
    default:
      Serial.print("Unknown command: ");
      Serial.println(command);
      break;
  }
}