#include <Arduino.h>

// ===== MOTOR PINS =====
const int enA = 9;
const int in1 = 8;
const int in2 = 7;

const int enB = 5;
const int in3 = 4;
const int in4 = 2;

// ===== IR PINS =====
const int LEFT_IR   = A5;
const int MIDDLE_IR = A2;
const int RIGHT_IR  = A4;

// ===== ULTRASONIC =====
const int TRIG_FRONT = 12;
const int ECHO_FRONT = 11;

const int TRIG_RIGHT = 13;
const int ECHO_RIGHT = 10;

// ===== GLOBAL STATE =====
int currentSpeed = 70;
bool manualControl = true;
bool isAutoMode = false;

// ===== AUTO VARIABLES =====
float targetDistance = 20.0;
int frontStopDist = 15;
float kp = 15.0;

float frontDist;
float rightDist;

int lastTurn = 0;   // -1 left, 1 right, 0 straight

// ===== SETUP =====
void setup() {
  Serial.begin(9600);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(LEFT_IR, INPUT);
  pinMode(MIDDLE_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);

  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);

  stopMotors();
}

// ===== LOOP =====
void loop() {

  if (Serial.available()) {
    char cmd = Serial.read();
    processCommand(cmd);
  }

  if (isAutoMode) {
    rightWallFollow();
    return;
  }

  if (!manualControl) {
    int L = analogRead(LEFT_IR);
    int M = analogRead(MIDDLE_IR);
    int R = analogRead(RIGHT_IR);

    int threshold = 500;

if (M < threshold) {
    drive();
    lastTurn = 0;
}
else if (L < threshold) {
    turnLeft();
    lastTurn = -1;
}
else if (R < threshold) {
    turnRight();
    lastTurn = 1;
}
else {
    // all sensors off → recover in last direction
    if (lastTurn == -1) {
        turnLeft();
    }
    else if (lastTurn == 1) {
        turnRight();
    }
    else {
        drive();
    }
}
    
Serial.print("L: ");
Serial.print(L);

Serial.print("  M: ");
Serial.print(M);

Serial.print("  R: ");
Serial.println(R);


    return;
  }
}