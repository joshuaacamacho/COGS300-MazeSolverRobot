float getDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) return -1;

  return duration * 0.034 / 2;
}

void rightWallFollow() {

  frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);

  if (frontDist > 0 && frontDist < frontStopDist) {
    turnLeft();
    delay(300);
    return;
  }

  if (rightDist <= 0) {
    rightDist = targetDistance;
  }

  float error = targetDistance - rightDist;
  float correction = error * kp;

  int leftSpeed  = currentSpeed + correction;
  int rightSpeed = currentSpeed - correction;

  leftSpeed  = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, rightSpeed);
  analogWrite(enB, leftSpeed);
}