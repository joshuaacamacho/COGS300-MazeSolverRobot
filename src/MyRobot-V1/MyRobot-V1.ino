/**
 * @file MyRobot-V1.ino
 * @brief Improved auto-calibration with backward movement support
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

// ===== ENCODER PIN DEFINITIONS =====
const int LEFT_ENCODER_PIN = A2;
const int RIGHT_ENCODER_PIN = A3;

// ===== PHYSICAL CONSTANTS =====
const int ENCODER_HOLES = 20;
const float WHEEL_DIAMETER = 6.5;
const float WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER * PI;
const float COUNTS_PER_ROTATION = ENCODER_HOLES * 2.0;

// ===== ENHANCED CALIBRATION SYSTEM =====
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

// ===== ENCODER VARIABLES =====
int leftEncoderCount = 0;
int rightEncoderCount = 0;
int lastLeftState = LOW;
int lastRightState = LOW;

// ===== MEASUREMENT VARIABLES =====
float leftDistance = 0.0;
float rightDistance = 0.0;

// ===== MOVEMENT TRACKING =====
bool isMoving = false;
unsigned long movementStartTime = 0;
float movementStartLeftDistance = 0.0;
float movementStartRightDistance = 0.0;

// ===== SPEED SETTINGS =====
int currentSpeed = 150;
const int SPEED_INCREMENT = 25;
const int MAX_SPEED = 255;

// ===== SETUP =====
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
  pinMode(LEFT_ENCODER_PIN, INPUT);
  pinMode(RIGHT_ENCODER_PIN, INPUT);

  stop();

  // WiFi connection
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
  
  Serial.println("\n=== ENHANCED CALIBRATION ROBOT ===");
  Serial.println("Separate forward/backward calibration!");
  printCalibrationStatus();
  Serial.println("Commands: w/s/a/d, space, c=calibrate, p=status");
  Serial.println("===================================\n");
}

// ===== MAIN LOOP =====
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
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

// ===== ENCODER FUNCTIONS =====
void readEncoders() {
  int leftState = digitalRead(LEFT_ENCODER_PIN);
  int rightState = digitalRead(RIGHT_ENCODER_PIN);
  
  if (leftState == HIGH && lastLeftState == LOW) {
    leftEncoderCount++;
    updateDistanceCalculations();
  }
  if (rightState == HIGH && lastRightState == LOW) {
    rightEncoderCount++;
    updateDistanceCalculations();
  }
  
  lastLeftState = leftState;
  lastRightState = rightState;
}

void updateDistanceCalculations() {
  leftDistance = (leftEncoderCount / COUNTS_PER_ROTATION) * WHEEL_CIRCUMFERENCE;
  rightDistance = (rightEncoderCount / COUNTS_PER_ROTATION) * WHEEL_CIRCUMFERENCE;
}

// ===== CALIBRATION FUNCTIONS =====
void applyCalibration() {
  if (!isMoving) return;
  
  int leftSpeed, rightSpeed;
  
  switch(currentDirection) {
    case FORWARD:
      leftSpeed = currentSpeed * leftCalibrationForward;
      rightSpeed = currentSpeed * rightCalibrationForward;
      break;
    case BACKWARD:
      leftSpeed = currentSpeed * leftCalibrationBackward;
      rightSpeed = currentSpeed * rightCalibrationBackward;
      break;
    default:
      leftSpeed = currentSpeed;
      rightSpeed = currentSpeed;
      break;
  }
  
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);
  
  analogWrite(enA, leftSpeed);
  analogWrite(enB, rightSpeed);
}

void autoCalibrateMovement() {
  if (!isMoving || !calibrationEnabled) return;
  
  float leftTraveled = leftDistance - movementStartLeftDistance;
  float rightTraveled = rightDistance - movementStartRightDistance;
  
  // Only calibrate after meaningful movement
  if (abs(leftTraveled) < 10.0 || abs(rightTraveled) < 10.0) return;
  
  // Determine which calibration to adjust
  float* leftCalib = (currentDirection == FORWARD) ? &leftCalibrationForward : &leftCalibrationBackward;
  float* rightCalib = (currentDirection == FORWARD) ? &rightCalibrationForward : &rightCalibrationBackward;
  
  // Calculate speed ratio
  float leftSpeed = abs(leftTraveled);
  float rightSpeed = abs(rightTraveled);
  float speedRatio = leftSpeed / rightSpeed;
  
  // Apply calibration
  if (speedRatio > 1.1) {  // Left is >10% faster
    // Left too fast - reduce left or increase right
    if (*rightCalib < MAX_CALIBRATION) {
      *rightCalib *= 1.05;  // Increase right by 5%
    } else if (*leftCalib > MIN_CALIBRATION) {
      *leftCalib *= 0.95;   // Decrease left by 5%
    }
  } else if (speedRatio < 0.9) {  // Right is >10% faster
    // Right too fast - reduce right or increase left
    if (*leftCalib < MAX_CALIBRATION) {
      *leftCalib *= 1.05;   // Increase left by 5%
    } else if (*rightCalib > MIN_CALIBRATION) {
      *rightCalib *= 0.95;  // Decrease right by 5%
    }
  }
  
  // Apply bounds
  *leftCalib = constrain(*leftCalib, MIN_CALIBRATION, MAX_CALIBRATION);
  *rightCalib = constrain(*rightCalib, MIN_CALIBRATION, MAX_CALIBRATION);
  
  // Apply new calibration immediately
  applyCalibration();
  
  Serial.print("Auto-calibrated ");
  Serial.print(currentDirection == FORWARD ? "FORWARD" : "BACKWARD");
  Serial.print(": L=");
  Serial.print(*leftCalib * 100, 0);
  Serial.print("%, R=");
  Serial.print(*rightCalib * 100, 0);
  Serial.println("%");
}

void runQuickCalibration() {
  Serial.println("\n=== QUICK CALIBRATION ===");
  
  // Calibrate forward
  Serial.println("Calibrating FORWARD...");
  currentDirection = FORWARD;
  startMovementTracking();
  driveRaw();  // Drive without calibration
  delay(1500);
  autoCalibrateMovement();
  stop();
  endMovementTracking();
  
  delay(1000);
  
  // Calibrate backward
  Serial.println("Calibrating BACKWARD...");
  currentDirection = BACKWARD;
  startMovementTracking();
  backwardsRaw();  // Drive without calibration
  delay(1500);
  autoCalibrateMovement();
  stop();
  endMovementTracking();
  
  printCalibrationStatus();
}

void driveRaw() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
}

void backwardsRaw() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
}

// ===== MOVEMENT FUNCTIONS =====
void startMovementTracking() {
  if (!isMoving) {
    isMoving = true;
    movementStartTime = millis();
    movementStartLeftDistance = leftDistance;
    movementStartRightDistance = rightDistance;
  }
}

void endMovementTracking() {
  if (isMoving) {
    isMoving = false;
    unsigned long duration = millis() - movementStartTime;
    float leftTraveled = leftDistance - movementStartLeftDistance;
    float rightTraveled = rightDistance - movementStartRightDistance;
    float avgDistance = (abs(leftTraveled) + abs(rightTraveled)) / 2.0;
    float syncError = abs(leftTraveled - rightTraveled);
    float errorPercent = (avgDistance > 0) ? (syncError / avgDistance * 100) : 0;
    
    // Auto-calibrate at end of movement
    if (calibrationEnabled && avgDistance > 5.0) {
      autoCalibrateMovement();
    }
    
    // Print report
    Serial.println("\n=== MOVEMENT REPORT ===");
    Serial.print("Direction: ");
    Serial.println(currentDirection == FORWARD ? "FORWARD" : "BACKWARD");
    Serial.print("Duration: ");
    Serial.print(duration);
    Serial.println(" ms");
    Serial.print("Avg distance: ");
    Serial.print(avgDistance, 1);
    Serial.println(" cm");
    Serial.print("Left traveled: ");
    Serial.print(leftTraveled, 1);
    Serial.print(" cm | Right: ");
    Serial.print(rightTraveled, 1);
    Serial.println(" cm");
    Serial.print("Sync error: ");
    Serial.print(syncError, 1);
    Serial.print(" cm (");
    Serial.print(errorPercent, 1);
    Serial.println("%)");
    
    if (leftTraveled > rightTraveled) {
      float percentFaster = ((leftTraveled / rightTraveled) - 1.0) * 100;
      Serial.print("Left wheel ");
      Serial.print(percentFaster, 0);
      Serial.println("% faster → Veers RIGHT");
    } else if (rightTraveled > leftTraveled) {
      float percentFaster = ((rightTraveled / leftTraveled) - 1.0) * 100;
      Serial.print("Right wheel ");
      Serial.print(percentFaster, 0);
      Serial.println("% faster → Veers LEFT");
    }
    
    printCurrentCalibration();
    Serial.println("======================\n");
  }
}

// ===== MOTOR CONTROL =====
void drive() {
  currentDirection = FORWARD;
  startMovementTracking();
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  applyCalibration();
  Serial.println("Moving FORWARD (calibrated)");
}

void backwards() {
  currentDirection = BACKWARD;
  startMovementTracking();
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  applyCalibration();
  Serial.println("Moving BACKWARD (calibrated)");
}

void stop() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0); analogWrite(enB, 0);
  currentDirection = STOPPED;
  Serial.println("STOPPED");
}

// ===== UTILITY FUNCTIONS =====
void printCalibrationStatus() {
  Serial.println("\n=== CALIBRATION STATUS ===");
  Serial.println("FORWARD:");
  Serial.print("  Left: ");
  Serial.print(leftCalibrationForward * 100, 0);
  Serial.print("% | Right: ");
  Serial.print(rightCalibrationForward * 100, 0);
  Serial.println("%");
  Serial.println("BACKWARD:");
  Serial.print("  Left: ");
  Serial.print(leftCalibrationBackward * 100, 0);
  Serial.print("% | Right: ");
  Serial.print(rightCalibrationBackward * 100, 0);
  Serial.println("%");
  Serial.print("Auto-calibration: ");
  Serial.println(calibrationEnabled ? "ENABLED" : "DISABLED");
  Serial.println("========================\n");
}

void printCurrentCalibration() {
  Serial.print("Calibration applied: ");
  if (currentDirection == FORWARD) {
    Serial.print("FWD L=");
    Serial.print(leftCalibrationForward * 100, 0);
    Serial.print("%, R=");
    Serial.print(rightCalibrationForward * 100, 0);
    Serial.println("%");
  } else if (currentDirection == BACKWARD) {
    Serial.print("BWD L=");
    Serial.print(leftCalibrationBackward * 100, 0);
    Serial.print("%, R=");
    Serial.print(rightCalibrationBackward * 100, 0);
    Serial.println("%");
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
    case ' ': 
      endMovementTracking();
      stop();
      break;
    case 'c': 
      runQuickCalibration();
      break;
    case 'p': 
      printCalibrationStatus();
      break;
    case 'j': speedUp(); break;
    case 'k': slowDown(); break;
    default:
      Serial.println("Unknown command");
      break;
  }
}

// ===== OTHER MOTOR FUNCTIONS (simplified) =====
void turnLeft() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed * 0.5);
  analogWrite(enB, currentSpeed);
  Serial.println("Turning LEFT");
}

void turnRight() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed * 0.5);
  Serial.println("Turning RIGHT");
}

void speedUp() {
  currentSpeed = min(currentSpeed + 25, 255);
  if (isMoving) applyCalibration();
  Serial.print("Speed: ");
  Serial.println(currentSpeed);
}

void slowDown() {
  currentSpeed = max(currentSpeed - 25, 0);
  if (isMoving) applyCalibration();
  Serial.print("Speed: ");
  Serial.println(currentSpeed);
  if (currentSpeed == 0) {
    endMovementTracking();
    stop();
  }
}