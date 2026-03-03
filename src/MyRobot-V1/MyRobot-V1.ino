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

// ===== SPEED SETTINGS =====
const int DRIVE_SPEED = 110;
const int TURN_SPEED  = 110;
const float LEFT_SCALE = 0.85;

// ===== GLOBAL STATE =====
int currentSpeed = DRIVE_SPEED;
bool manualControl = true;
bool isAutoMode = false;

// ===== AUTO VARIABLES =====
float targetDistance = 20.0;
int frontStopDist = 15;
float kp = 15.0;

float frontDist;
float rightDist;

int lastTurn = 0;

// ===== SWEEP SETTINGS =====
const int NUM_ANGLES = 12;
float distances[NUM_ANGLES];

int sweepTurnDelay = 144;

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

// ===== SETUP =====
void loop() {

  float d = getDistance(TRIG_FRONT, ECHO_FRONT);

if (d > 0 && d < 10) {
  stopMotors();
  Serial.println("OBJECT REACHED");
  while(true);
}

  sweep180();

  int best = getClosestIndex();

  Serial.print("Best index: ");
  Serial.println(best);

  if (distances[best] <= 0) {
    Serial.println("Invalid scan — rescanning");
    return;
  }

  if (distances[best] > 0 && distances[best] < 12) {
    stopMotors();
    Serial.println("OBJECT VERY CLOSE");
    while(true);
  }

  int stepsBack = NUM_ANGLES - best - 1;

  rotateLeftSteps(stepsBack);

  float d2 = getDistance(TRIG_FRONT, ECHO_FRONT);

  if (d2 > 0 && d2 < 8) {
    stopMotors();
    Serial.println("OBJECT REACHED");
    while(true);
  }

  driveForwardStep();
}


// ===== SWEEP =====
void sweep180() {

  Serial.println("---- DEPTH MAP ----");

  for (int i = 0; i < NUM_ANGLES; i++) {

    float d = getDistance(TRIG_FRONT, ECHO_FRONT);
    distances[i] = d;

    Serial.print("Index ");
    Serial.print(i);
    Serial.print("  Angle ");
    Serial.print(i * 15);
    Serial.print("°  Distance: ");
    Serial.println(d);

    if (i < NUM_ANGLES - 1) {
    turnRightSmallStep();
    delay(150);
  }
  }

  Serial.println("-------------------");
}


// ===== FIND CLOSEST OBJECT =====
int getClosestIndex() {

  int minIndex = 0;
  float minDist = distances[0];

  for (int i = 1; i < NUM_ANGLES; i++) {

    if (distances[i] < minDist && distances[i] > 0) {

      minDist = distances[i];
      minIndex = i;
    }
  }

  return minIndex;
}