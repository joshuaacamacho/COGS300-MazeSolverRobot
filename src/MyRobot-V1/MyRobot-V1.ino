#include <Arduino.h>

// ===== MOTOR PINS =====
const int enA = 9;
const int in1 = 8;
const int in2 = 7;

const int enB = 5;
const int in3 = 4;
const int in4 = 2;

// ===== ULTRASONIC PINS =====
const int TRIG_FRONT = 12;
const int ECHO_FRONT = 11;
const int TRIG_RIGHT = 13;
const int ECHO_RIGHT = 10;

// ===== IR SENSOR PINS =====
const int IR_LEFT   = A5;
const int IR_CENTER = A2;
const int IR_RIGHT  = A4;

// ===== LED PINS =====
const int LED_LINE   = 3;
const int LED_WALL   = 6;
const int LED_OBJECT = A0;

// ===== SPEED SETTINGS =====
const int DRIVE_SPEED  = 110;
const int TURN_SPEED   = 110;
const float LEFT_SCALE = 0.78;

// ===== WALL FOLLOW SETTINGS =====
const float kp             = 0.8;
const float targetDistance = 10.0;
const float frontStopDist  = 25.0;
int currentSpeed           = 110;

// ===== SWEEP SETTINGS =====
const int NUM_ANGLES = 12;
float distances[NUM_ANGLES];
float belief[NUM_ANGLES];
int sweepTurnDelay = 144;

// ===== GLOBAL STATE =====
bool objectLocked       = false;
int  currentFacingSteps = 0;
bool emergencyStop      = false; // new stop flag

// ===== TRANSITION COUNTERS =====
int noLineCount   = 0;
int wallSeenCount = 0;
int noWallCount   = 0;

// ===== MODE =====
char mode    = 'm';
char lastLED = 'm'; // tracks last mode for LED display


// ===== FUNCTION DECLARATIONS =====
float getDistance(int trigPin, int echoPin);
void  stopMotors();
void  drive(int speed);
void  turnLeft();
void  turnRight();
void  rotateLeftSteps(int steps);
void  rotateRightSteps(int steps);
void  driveForwardStep();
void  sweep180();
void  updateBelief();
int   getBestBeliefIndex();
void  processCommand(char cmd);
void  lineFollow();
void  rightWallFollow();
void  runObjectDetection();
void  updateModeLEDs();


// ===== SETUP =====
void setup() {
  Serial.begin(9600);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_RIGHT,  OUTPUT);
  pinMode(ECHO_RIGHT,  INPUT);

  pinMode(IR_LEFT,   INPUT);
  pinMode(IR_CENTER, INPUT);
  pinMode(IR_RIGHT,  INPUT);

  pinMode(LED_LINE,   OUTPUT);
  pinMode(LED_WALL,   OUTPUT);
  pinMode(LED_OBJECT, OUTPUT);

  stopMotors();

  getDistance(TRIG_FRONT, ECHO_FRONT);
  delay(100);
  getDistance(TRIG_RIGHT, ECHO_RIGHT);
  delay(100);

  for (int i = 0; i < NUM_ANGLES; i++)
    belief[i] = 1.0 / NUM_ANGLES;

  Serial.println("Ready. Commands: w/a/s/d = manual, l = line, f = wall, o = object, space = stop");
}


// ===== MAIN LOOP =====
void loop() {

  // Always check serial first
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
  }

  // Emergency stop overrides everything
  if (emergencyStop) {
    stopMotors();
    updateModeLEDs();
    return;
  }

  switch (mode) {
    case 'm': break;
    case 'l': lineFollow();         break;
    case 'f': rightWallFollow();    break;
    case 'o': runObjectDetection(); break;
  }

  updateModeLEDs();
}


// ===== UPDATE MODE LEDS =====
void updateModeLEDs() {

  // LEDs show lastLED not mode — so space keeps LED on
  digitalWrite(LED_LINE,   lastLED == 'l' ? HIGH : LOW);
  digitalWrite(LED_WALL,   lastLED == 'f' ? HIGH : LOW);
  digitalWrite(LED_OBJECT, lastLED == 'o' ? HIGH : LOW);
}


// ===== ULTRASONIC =====
float getDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 38000);

  if (duration == 0) return -1;

  float d = duration * 0.034f / 2.0f;
  if (d < 2) return -1;
  return d;
}