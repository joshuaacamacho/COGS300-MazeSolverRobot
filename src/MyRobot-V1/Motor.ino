void drive() {

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, DRIVE_SPEED);
  analogWrite(enB, DRIVE_SPEED * LEFT_SCALE);
}


void backwards() {

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, DRIVE_SPEED);
  analogWrite(enB, DRIVE_SPEED * LEFT_SCALE);
}


void stopMotors() {

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  analogWrite(enA, 0);
  analogWrite(enB, 0);
}


void turnLeft() {

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, TURN_SPEED);
  analogWrite(enB, TURN_SPEED * LEFT_SCALE);
}


void turnRight() {

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, TURN_SPEED);
  analogWrite(enB, TURN_SPEED * LEFT_SCALE);
}


// ===== SMALL SWEEP STEP =====
void turnRightSmallStep() {

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, TURN_SPEED);
  analogWrite(enB, TURN_SPEED * LEFT_SCALE);

  delay(sweepTurnDelay);

  stopMotors();
}


// ===== ROTATE BACK =====
void rotateLeftSteps(int steps) {

  for (int i = 0; i < steps; i++) {

    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);

    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);

    analogWrite(enA, TURN_SPEED);
    analogWrite(enB, TURN_SPEED * LEFT_SCALE);

    delay(sweepTurnDelay);

    stopMotors();
    delay(120);
  }
}

void rotateRightSteps(int steps) {

  for (int i = 0; i < steps; i++) {

    // RIGHT motor backward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);

    // LEFT motor forward
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);

    analogWrite(enA, TURN_SPEED);
    analogWrite(enB, TURN_SPEED * LEFT_SCALE);

    delay(sweepTurnDelay);

    stopMotors();
    delay(120);
  }
}


void driveForwardStep() {

  unsigned long start = millis();

  while (millis() - start < 900) {

    float d = getDistance(TRIG_FRONT, ECHO_FRONT);

   if (d > 0 && d < 10) {
  stopMotors();
  Serial.println("OBJECT REACHED");
  return;
}

    if (d > 0 && d < 20) {
      analogWrite(enA, DRIVE_SPEED * 0.6);
      analogWrite(enB, DRIVE_SPEED * LEFT_SCALE * 0.55);
    } else {
      drive();
    }

    delay(50);
  }

  stopMotors();
}