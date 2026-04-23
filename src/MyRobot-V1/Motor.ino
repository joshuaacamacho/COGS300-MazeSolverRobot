// =============================================================================
// Motor.ino
// Low-level motor primitives. All functions write directly to the L298N driver
// pins declared in MyRobot-V1.ino. Higher-level motion (autonomous modes) is
// built on top of these in AutoMode.ino.
// =============================================================================

#include <Arduino.h>

extern const int enA, in1, in2;
extern const int enB, in3, in4;
extern const int DRIVE_SPEED;
extern const int PIVOT_SPEED;
extern const int TURN_SPEED;
extern const float LEFT_SCALE;
extern const int TRIG_FRONT;
extern const int ECHO_FRONT;
extern int  sweepTurnDelay;
extern bool objectLocked;

float getDistance(int trigPin, int echoPin);


// =============================================================================
// drive
// Drives both motors forward at the requested speed, applying LEFT_SCALE to
// the right motor to compensate for any hardware imbalance.
//
// Parameters:
//   speed — PWM value (0-255) applied to the left motor
// =============================================================================
void drive(int speed) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, speed);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, (int)(speed * LEFT_SCALE));
}


// =============================================================================
// stopMotors
// Cuts power to both motors immediately. Does not brake — wheels will coast.
// =============================================================================
void stopMotors() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(enA, 0);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW); analogWrite(enB, 0);
}


// =============================================================================
// turnLeft / turnRight
// Continuous differential turns used during manual mode. Both motors run but
// in opposite directions, pivoting the robot around its centre.
// =============================================================================
void turnLeft() {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, TURN_SPEED);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH); analogWrite(enB, (int)(TURN_SPEED * LEFT_SCALE));
}

void turnRight() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(enA, TURN_SPEED);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, (int)(TURN_SPEED * LEFT_SCALE));
}


// =============================================================================
// rotateLeftSteps / rotateRightSteps
// Stepped rotation used during the object detection sweep. Each step rotates
// the robot by a fixed angular increment governed by sweepTurnDelay, with a
// brief pause between steps to let the chassis settle before the next reading.
//
// Parameters:
//   steps — number of discrete rotation increments to execute
// =============================================================================
void rotateLeftSteps(int steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, TURN_SPEED);
    digitalWrite(in3, LOW);  digitalWrite(in4, HIGH); analogWrite(enB, (int)(TURN_SPEED * LEFT_SCALE));
    delay(sweepTurnDelay);
    stopMotors();
    delay(150);
  }
}

void rotateRightSteps(int steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(enA, TURN_SPEED);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, (int)(TURN_SPEED * LEFT_SCALE));
    delay(sweepTurnDelay);
    stopMotors();
    delay(150);
  }
}


// =============================================================================
// driveForwardStep
// Drives toward a locked object until it is within 15 cm of the front sensor.
// Tolerates brief sensor dropouts (up to LOST_THRESHOLD consecutive bad reads)
// before declaring the object lost and clearing the lock. Hard-stops and halts
// the program when the object is reached.
// =============================================================================
void driveForwardStep() {
  const int          LOST_THRESHOLD = 30;
  const unsigned long MAX_DRIVE_MS  = 3000;

  int           lostCount = 0;
  unsigned long startTime = millis();

  while (true) {
    if (millis() - startTime > MAX_DRIVE_MS) {
      Serial.println("Drive timeout — rescanning");
      stopMotors();
      objectLocked = false;
      return;
    }

    float d = getDistance(TRIG_FRONT, ECHO_FRONT);

    if (d > 2 && d < 15) {
      stopMotors();
      Serial.println("OBJECT REACHED");
      while (true);  // Intentional halt — robot has completed its task
    }

    if (d <= 0 || d > 200) {
      if (++lostCount >= LOST_THRESHOLD) {
        Serial.println("Object lost — rescanning");
        stopMotors();
        objectLocked = false;
        return;
      }
    } else {
      lostCount = 0;
    }

    drive(DRIVE_SPEED);
    delay(20);
  }
}
