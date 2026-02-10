void rightWallFollow() {
  frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  if (rightDist < 0) {
      rightDist = targetDistance;
    }
    
    // calculate PID error
    // error > 0: too close -> turn left (away from wall)
    // error < 0: too far -> turn right (towards wall)
    float error = targetDistance - rightDist;
    float correction = error * kp;
    correction = constrain(correction, -25, 25);
    
    int leftSpeed  = currentSpeed + correction;
    int rightSpeed = currentSpeed - correction;

    leftSpeed  = constrain(leftSpeed,  100, 200);
    rightSpeed = constrain(rightSpeed, 100, 200);

    if (frontDist > 0 && frontDist < frontStopDist + 10) {
      Serial.println("Corner Detected! Turning Left.");
      turnLeft();
      delay(400); // time to turn 90 degrees
    } else {
      drive(leftSpeed, rightSpeed);
    }
  delay(50);
}

float getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 12000);

  if (duration == 0) return -1; // no echo

  float dist = duration * 0.034 / 2;

  if (dist < 2 || dist > 200) return -1;

  return dist;
}

void drive(int leftSpeed, int rightSpeed) {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enB, leftSpeed); analogWrite(enA, rightSpeed);
}

void turnLeft(int leftSpeed, int rightSpeed) {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, rightSpeed);
  analogWrite(enB, leftSpeed * 0.5);
}