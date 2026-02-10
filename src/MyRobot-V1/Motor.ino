/**
 * @file Motor.ino
 * @brief Simple H-bridge motor control helpers.
 *
 * Provides a minimal interface for driving a DC motor using
 * digital GPIO pins (e.g. Arduino-style platforms).
 *
 * The motor direction is controlled via two input pins,
 * while a separate enable pin turns the motor on or off.
 */

void drive() {
  currentDirection = FORWARD;
  startMovementTracking();
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  applyCalibration();
  Serial.println("Moving FORWARD (calibrated)");
}

void backwards() {
  currentDirection = BACKWARD;
  startMovementTracking();
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  applyCalibration();
  Serial.println("Moving BACKWARD (calibrated)");
}

void stop() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0); analogWrite(enB, 0);
  currentDirection = STOPPED;
  Serial.println("STOPPED");
}

void turnLeft() {
  currentDirection = TURNING;   
  isMoving = false; 
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed * 0.5);
  analogWrite(enB, currentSpeed);
  Serial.println("Turning LEFT");
}

void turnRight() {
  currentDirection = TURNING;   
  isMoving = false;   
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed * 0.5);
  Serial.println("Turning RIGHT");
}

void speedUp() {
  currentSpeed = min(currentSpeed + 25, 255);
  if (isMoving) applyCalibration();
  Serial.print("Speed: ");
  Serial.println(currentSpeed);
}

void slowDown() {
  currentSpeed = max(currentSpeed - 25, 0);
  if (isMoving) applyCalibration();
  Serial.print("Speed: ");
  Serial.println(currentSpeed);
  if (currentSpeed == 0) {
    endMovementTracking();
    stop();
  }
}