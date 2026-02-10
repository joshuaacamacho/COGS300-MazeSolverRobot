/**
 * @file MyRobot-V1.ino
 * @brief Improved auto-calibration with backward movement support
 */

#include <Arduino.h>
#include <WiFiS3.h>

// WIFI CREDENTIALS 
const char* ssid     = "RobotNet";
const char* password = "robot1234";

// SERVER SETTINGS 
const int port = 8080;
WiFiServer server(port);

// MOTOR PINS
const int enA = 5;   // Right motor PWM
const int in1 = 2;
const int in2 = 4;

const int enB = 9;   // Left motor PWM
const int in3 = 8;
const int in4 = 7;

// ENCODER PINS
const int LEFT_ENCODER_PIN = A3;
const int RIGHT_ENCODER_PIN = A2;

// ULTRASONIC SENSOR PINS
const int TRIG_FRONT = 12;
const int ECHO_FRONT = 11;

const int TRIG_RIGHT = 13;
const int ECHO_RIGHT = 10;

// PHYSICAL CONSTANTS 
const int ENCODER_HOLES = 20;
const float WHEEL_DIAMETER = 6;
const float WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER * PI;
const float COUNTS_PER_ROTATION = ENCODER_HOLES;

//  ENHANCED CALIBRATION SYSTEM 
// Separate calibration for forward and backward!
float leftCalibrationForward = 1.0;
float rightCalibrationForward = 1.0;
float leftCalibrationBackward = 1.0;
float rightCalibrationBackward = 1.0;

float calibrationFactor = 0.05;  // Increased for faster adjustment
const float MAX_CALIBRATION = 2.0;  // Allow up to 200% power
const float MIN_CALIBRATION = 0.5;  // Allow down to 50% power
bool calibrationEnabled = true;

// Track movement direction
enum Direction { FORWARD, BACKWARD, TURNING, STOPPED };
Direction currentDirection = STOPPED;

// ENCODER VARIABLES 
int leftEncoderCount = 0;
int rightEncoderCount = 0;
int lastLeftState = LOW;
int lastRightState = LOW;

// MEASUREMENT VARIABLES 
float leftDistance = 0.0;
float rightDistance = 0.0;

// MOVEMENT TRACKING 
bool isMoving = false;
unsigned long movementStartTime = 0;
float movementStartLeftDistance = 0.0;
float movementStartRightDistance = 0.0;

// AUTOMODE SETTINGS
float targetDistance = 20.0;  // Target distance from right wall
int frontStopDist = 15; // Distance to turn at corners
float kp = 15.0; // PID Proportional gain and Derivative gain
bool isAutoMode = false;
float frontDist, rightDist;

// SPEED SETTINGS 
int currentSpeed = 150;
const int SPEED_INCREMENT = 25;
const int MAX_SPEED = 255;

// SETUP 
void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Motor pins
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Encoder pins
  pinMode(LEFT_ENCODER_PIN, INPUT_PULLUP);
  pinMode(RIGHT_ENCODER_PIN, INPUT_PULLUP);

  // Ultrasonic sensor pins
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);

  stop();

 Serial.begin(9600);
  while(!Serial);  // wait for serial monitor

  // Connect to WiFi 
  Serial.print("[INFO] Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[INFO] WiFi connected!");
  Serial.print("[INFO] Robot IP: ");
  Serial.println(WiFi.localIP());

  // Start server
  server.begin();
  Serial.println("[INFO] Server started on port 8080");
}

// MAIN LOOP 
void loop() {
  readEncoders();
  
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
      readEncoders();
      if (isAutoMode) {
        rightWallFollow();
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}