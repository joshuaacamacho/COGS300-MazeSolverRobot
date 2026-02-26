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
int currentSpeed = 50;
bool manualControl = true;
bool isAutoMode = false;

// ===== AUTO VARIABLES =====
float targetDistance = 20.0;
int frontStopDist = 15;
float kp = 15.0;

float frontDist;
float rightDist;

int lastTurn = 0;   // -1 left, 1 right, 0 straight

// ===== SWEEP SETTINGS =====
const int NUM_ANGLES = 12;     // 180° / 15°
float distances[NUM_ANGLES];

int sweepTurnDelay = 144;      // adjust for ~15° turn

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
bool hasSwept = false;

void loop() {

  if (!hasSwept) {

    sweep180();

    int best = getClosestIndex();

    Serial.print("Closest index: ");
    Serial.println(best);

    int stepsBack = NUM_ANGLES - best;

    rotateLeftSteps(stepsBack);

    hasSwept = true;
  }
}

void sweep180() {

  Serial.println("---- DEPTH MAP ----");

  for (int i = 0; i < NUM_ANGLES; i++) {

    turnRightSmallStep();
    delay(80);

    float d = getDistance(TRIG_FRONT, ECHO_FRONT);
    distances[i] = d;

    Serial.print("Index ");
    Serial.print(i);
    Serial.print("  Angle ");
    Serial.print(i * 15);
    Serial.print("°  Distance: ");
    Serial.println(d);
  }

  Serial.println("-------------------");
}

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

void rotateLeftSteps(int steps) {

  for (int i = 0; i < steps; i++) {

    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);

    analogWrite(enA, 70);
    analogWrite(enB, 70);

    delay(sweepTurnDelay);
    stopMotors();
    delay(80);
  }
}