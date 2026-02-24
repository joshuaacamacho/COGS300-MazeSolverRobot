void drive() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed); analogWrite(enB, currentSpeed);
}

void backwards() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, currentSpeed); analogWrite(enB, currentSpeed);
}

void stop() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0); analogWrite(enB, 0);
}

void turnLeft() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed * 0.5); analogWrite(enB, currentSpeed);
}

void turnRight() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed); analogWrite(enB, currentSpeed * 0.5);
}

void speedUp() {
  currentSpeed = min(currentSpeed + 25, 255);
}

void slowDown() {
  currentSpeed = max(currentSpeed - 25, 0);
}