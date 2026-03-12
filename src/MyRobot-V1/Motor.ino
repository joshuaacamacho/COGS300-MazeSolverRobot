#include <Arduino.h>

extern const int enA;
extern const int in1;
extern const int in2;
extern const int enB;
extern const int in3;
extern const int in4;
extern const int DRIVE_SPEED;
extern const int TURN_SPEED;
extern const float LEFT_SCALE;
extern int sweepTurnDelay;
extern bool objectLocked;
extern const int TRIG_FRONT;
extern const int ECHO_FRONT;

float getDistance(int trigPin, int echoPin);


// ===== BASIC DRIVE =====
void drive(int speed) {

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, speed);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, speed * LEFT_SCALE);
}


// ===== STOP =====
void stopMotors() {

  digitalWrite(in1, LOW);  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}


// ===== TURN LEFT =====
void turnLeft() {

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, TURN_SPEED);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, TURN_SPEED * LEFT_SCALE);
}


// ===== TURN RIGHT =====
void turnRight() {

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, TURN_SPEED);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, TURN_SPEED * LEFT_SCALE);
}


// ===== ROTATE LEFT STEPS =====
void rotateLeftSteps(int steps) {

  for (int i = 0; i < steps; i++) {

    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, TURN_SPEED);

    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    analogWrite(enB, TURN_SPEED * LEFT_SCALE);

    delay(sweepTurnDelay);
    stopMotors();
    delay(150);
  }
}


// ===== ROTATE RIGHT STEPS =====
void rotateRightSteps(int steps) {

  for (int i = 0; i < steps; i++) {

    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, TURN_SPEED);

    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, TURN_SPEED * LEFT_SCALE);

    delay(sweepTurnDelay);
    stopMotors();
    delay(150);
  }
}


// ===== DRIVE FORWARD STEP =====
void driveForwardStep() {

  int lostCount = 0;
  const int LOST_THRESHOLD = 30;
  unsigned long startTime = millis();
  const unsigned long MAX_DRIVE_MS = 3000;

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
      while (true);
    }

    if (d <= 0 || d > 200) {
      lostCount++;
      if (lostCount >= LOST_THRESHOLD) {
        Serial.println("Object lost — rescanning");
        stopMotors();
        objectLocked = false;
        return;
      }
      drive(DRIVE_SPEED);
      delay(20);
      continue;
    }

    lostCount = 0;
    drive(DRIVE_SPEED);
    delay(20);
  }
}