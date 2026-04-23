#include <Arduino.h>

// =============================================================================
// MyRobot-V1.ino
// Main entry point. Defines all hardware configuration, global state, and the
// Arduino setup() / loop() functions. All autonomous behaviour is implemented
// in AutoMode.ino and Motor.ino.
//
// Serial commands:
//   w / a / s / d  — manual drive (forward / left / reverse / right)
//   l              — line follow mode
//   f              — wall follow mode
//   o              — object detection mode
//   space          — emergency stop
//   j / k          — increase / decrease manual speed by 25
// =============================================================================


// -----------------------------------------------------------------------------
// Motor driver pins (L298N)
// Left motor  = channel A (enA, in1, in2)
// Right motor = channel B (enB, in3, in4)
// -----------------------------------------------------------------------------
const int enA = 9;
const int in1 = 8;
const int in2 = 7;
const int enB = 5;
const int in3 = 4;
const int in4 = 2;

// -----------------------------------------------------------------------------
// Ultrasonic sensor pins (HC-SR04)
// -----------------------------------------------------------------------------
const int TRIG_FRONT = 12;
const int ECHO_FRONT = 11;
const int TRIG_RIGHT = 13;
const int ECHO_RIGHT = 10;

// -----------------------------------------------------------------------------
// IR line sensor pins
// 0 = on tape, 1 = off tape
// -----------------------------------------------------------------------------
const int IR_LEFT   = A5;
const int IR_CENTER = A2;
const int IR_RIGHT  = A4;

// -----------------------------------------------------------------------------
// Mode indicator LED pins
// -----------------------------------------------------------------------------
const int LED_LINE   = 3;
const int LED_WALL   = 6;
const int LED_OBJECT = A0;

// -----------------------------------------------------------------------------
// Speed constants
// LEFT_SCALE compensates for left/right motor imbalance.
// DRIVE_SPEED and PIVOT_SPEED are used throughout AutoMode.ino.
// TURN_SPEED is used for the stepped rotations in object detection.
// -----------------------------------------------------------------------------
const int   DRIVE_SPEED = 55;
const int   PIVOT_SPEED = 55;
const int   TURN_SPEED  = 110;
const float LEFT_SCALE  = 0.95;

// -----------------------------------------------------------------------------
// Object detection sweep settings
// NUM_ANGLES: number of angular samples taken during a 180-degree sweep.
// sweepTurnDelay: milliseconds per rotation step during sweep.
// -----------------------------------------------------------------------------
const int NUM_ANGLES  = 12;
int       sweepTurnDelay = 144;

// -----------------------------------------------------------------------------
// Global state
// -----------------------------------------------------------------------------
bool emergencyStop       = false;
char manualCommand       = ' ';
char mode                = 'm';  // 'm'=manual, 'l'=line, 'f'=wall, 'o'=object
char lastLED             = 'm';

// Line follow state
int  noLineCount         = 0;
int  wallSeenCount       = 0;
int  lastTurnDirection   = 0;
bool approachingLeftTurn = false;
bool approachingRightTurn= false;

// Wall follow state
int  noWallCount         = 0;

// Object detection state
bool  objectLocked       = false;
int   currentFacingSteps = 0;
float distances[NUM_ANGLES];
float belief[NUM_ANGLES];

// Manual speed (adjustable via j/k commands)
int currentSpeed = 65;


// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------
float getDistance(int trigPin, int echoPin);
void  stopMotors();
void  drive(int speed);
void  turnLeft();
void  turnRight();
void  rotateLeftSteps(int steps);
void  rotateRightSteps(int steps);
void  driveForwardStep();
void  processCommand(char cmd);
void  lineFollow();
void  rightWallFollow();
void  runObjectDetection();
void  sweep180();
void  updateBelief();
int   getBestBeliefIndex();
void  updateModeLEDs();


// =============================================================================
// setup
// Initialises all pins, warms up the ultrasonic sensors with a dummy read,
// and seeds the object detection belief array.
// =============================================================================
void setup() {
  Serial.begin(9600);

  // Motor driver
  pinMode(enA, OUTPUT); pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT); pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);

  // Ultrasonic sensors
  pinMode(TRIG_FRONT, OUTPUT); pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT); pinMode(ECHO_RIGHT, INPUT);

  // IR sensors
  pinMode(IR_LEFT,   INPUT);
  pinMode(IR_CENTER, INPUT);
  pinMode(IR_RIGHT,  INPUT);

  // LEDs
  pinMode(LED_LINE,   OUTPUT);
  pinMode(LED_WALL,   OUTPUT);
  pinMode(LED_OBJECT, OUTPUT);

  stopMotors();

  // Warm-up reads to settle the ultrasonic sensors before autonomous modes begin
  getDistance(TRIG_FRONT, ECHO_FRONT); delay(100);
  getDistance(TRIG_RIGHT, ECHO_RIGHT); delay(100);

  // Uniform prior over all sweep angles
  for (int i = 0; i < NUM_ANGLES; i++)
    belief[i] = 1.0f / NUM_ANGLES;

  Serial.println("Ready. Commands: w/a/s/d=manual  l=line  f=wall  o=object  space=stop  j/k=speed");
}


// =============================================================================
// loop
// Reads any incoming serial command, then dispatches to the active mode.
// Emergency stop halts all motion and skips mode dispatch until cleared.
// =============================================================================
void loop() {
  if (Serial.available() > 0)
    processCommand(Serial.read());

  if (emergencyStop) {
    stopMotors();
    updateModeLEDs();
    return;
  }

  switch (mode) {
    case 'm':
      if      (manualCommand == 'w') drive(DRIVE_SPEED);
      else if (manualCommand == 's') {
        // Reverse — mirrors drive() but with both motors running backward
        digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(enA, DRIVE_SPEED);
        digitalWrite(in3, LOW);  digitalWrite(in4, HIGH); analogWrite(enB, (int)(DRIVE_SPEED * LEFT_SCALE));
      }
      else if (manualCommand == 'a') turnLeft();
      else if (manualCommand == 'd') turnRight();
      break;

    case 'l': lineFollow();          break;
    case 'f': rightWallFollow();     break;
    case 'o': runObjectDetection();  break;
  }

  updateModeLEDs();
}


// =============================================================================
// updateModeLEDs
// Lights the LED corresponding to the last committed mode. Updated every loop
// iteration so LEDs remain correct after mode transitions.
// =============================================================================
void updateModeLEDs() {
  digitalWrite(LED_LINE,   lastLED == 'l' ? HIGH : LOW);
  digitalWrite(LED_WALL,   lastLED == 'f' ? HIGH : LOW);
  digitalWrite(LED_OBJECT, lastLED == 'o' ? HIGH : LOW);
}


// =============================================================================
// getDistance
// Fires one HC-SR04 pulse and returns the measured distance in centimetres.
// Returns -1 if the echo times out or the result is implausibly small.
//
// Parameters:
//   trigPin — OUTPUT pin connected to the sensor TRIG line
//   echoPin — INPUT pin connected to the sensor ECHO line
// =============================================================================
float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 38000);
  if (duration == 0) return -1;

  float d = duration * 0.034f / 2.0f;
  return (d < 2) ? -1 : d;
}
