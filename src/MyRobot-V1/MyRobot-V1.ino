/**
 * @file MyRobot-V1.ino
 * @brief Improved auto-calibration with backward movement support
 */

#include <Arduino.h>

// MOTOR PINS
const int enA = 5;   // Right motor PWM
const int in1 = 2;
const int in2 = 4;

const int enB = 9;   // Left motor PWM
const int in3 = 8;
const int in4 = 7;

// ULTRASONIC SENSOR PINS
const int TRIG_FRONT = 12;
const int ECHO_FRONT = 11;

const int TRIG_RIGHT = 13;
const int ECHO_RIGHT = 10;

// IR SENSOR PINS
const int LEFT_IR = A5;
const int RIGHT_IR = A4;
const int MIDDLE_IR = A2;

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

  // Ultrasonic sensor pins
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);

  // IR Sensor Pins
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  pinMode(MIDDLE_IR, INPUT);

  stop();
}

// MAIN LOOP 
void loop() {
  int L = analogRead(LEFT_IR);
  int M = analogRead(MIDDLE_IR);
  int R = analogRead(RIGHT_IR);

  if (M < 50 && L >= 50 && R >= 50) { // Only Center detects line
    drive();
  } else if (L < 59) { // Only Left
    turnLeft();
  } else if (R < 50) { // Only Right
    turnRight();
  } else if (L >= 50 && M >= 50 && R >= 50) { // No line
    stop();
  }
  delay(100);
}