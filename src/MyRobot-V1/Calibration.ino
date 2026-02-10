// ENCODER FUNCTIONS 
void readEncoders() {
  float leftState = digitalRead(LEFT_ENCODER_PIN);
  float rightState = digitalRead(RIGHT_ENCODER_PIN);
  
  if (leftState == HIGH && lastLeftState == LOW) {
    leftEncoderCount++;
    updateDistanceCalculations();
  }
  if (rightState == HIGH && lastRightState == LOW) {
    rightEncoderCount++;
    updateDistanceCalculations();
  }
  
  lastLeftState = leftState;
  lastRightState = rightState;
}

void updateDistanceCalculations() {
  leftDistance = (leftEncoderCount / (float)COUNTS_PER_ROTATION) * WHEEL_CIRCUMFERENCE;
  rightDistance = (rightEncoderCount / (float)COUNTS_PER_ROTATION) * WHEEL_CIRCUMFERENCE;
}

void applyCalibration() {
  if (!isMoving) return;
  
  int leftDelta, rightDelta;
  
  switch(currentDirection) {
    case FORWARD:
      leftDelta = currentSpeed * leftCalibrationForward;
      rightDelta = currentSpeed * rightCalibrationForward;
      break;
    case BACKWARD:
      leftDelta = currentSpeed * leftCalibrationBackward;
      rightDelta = currentSpeed * rightCalibrationBackward;
      break;
    default:
      leftDelta = currentSpeed;
      rightDelta = currentSpeed;
      break;
  }
  
  leftDelta = constrain(leftDelta, 0, 255);
  rightDelta = constrain(rightDelta, 0, 255);
  
  analogWrite(enA, (int)leftDelta);
  analogWrite(enB, (int)rightDelta);
}

void autoCalibrateMovement() {
  if (!isMoving || !calibrationEnabled) return;
  
  float leftTraveled = leftDistance - movementStartLeftDistance;
  float rightTraveled = rightDistance - movementStartRightDistance;
  
  // Only calibrate after meaningful movement
  if (abs(leftTraveled) < 10.0 || abs(rightTraveled) < 10.0) return;
  
  // Determine which calibration to adjust
  float* leftCalib = (currentDirection == FORWARD) ? &leftCalibrationForward : &leftCalibrationBackward;
  float* rightCalib = (currentDirection == FORWARD) ? &rightCalibrationForward : &rightCalibrationBackward;
  
  // Calculate speed ratio
  float leftDelta = abs(leftTraveled);
  float rightDelta = abs(rightTraveled);
  if (rightDelta < 0.001 || leftDelta < 0.001) return;
  float ratio = leftDelta / rightDelta;
  
  // Apply calibration
  if (ratio > 1.1) {  // Left is >10% faster
    if (*rightCalib < MAX_CALIBRATION) {
      *rightCalib *= (1.0 + calibrationFactor);  // Increase right by calibrationFactor
    } else if (*leftCalib > MIN_CALIBRATION) {
      *leftCalib *= (1.0 - calibrationFactor);   // Decrease left by calibrationFactor
    }
  } else if (ratio < 0.9) {  // Right is >10% faster
    if (*leftCalib < MAX_CALIBRATION) {
      *leftCalib *= (1.0 + calibrationFactor);   // Increase left by calibrationFactor
    } else if (*rightCalib > MIN_CALIBRATION) {
      *rightCalib *= (1.0 - calibrationFactor);  // Decrease right by calibrationFactor
    }
  }
    
  // Apply bounds
  *leftCalib = constrain(*leftCalib, MIN_CALIBRATION, MAX_CALIBRATION);
  *rightCalib = constrain(*rightCalib, MIN_CALIBRATION, MAX_CALIBRATION);
  
  // Apply new calibration immediately
  applyCalibration();
  
  Serial.print("Auto-calibrated ");
  Serial.print(currentDirection == FORWARD ? "FORWARD" : "BACKWARD");
  Serial.print(": L=");
  Serial.print(*leftCalib * 100, 0);
  Serial.print("%, R=");
  Serial.print(*rightCalib * 100, 0);
  Serial.println("%");
}

void runQuickCalibration() {
  Serial.println("\n=== QUICK CALIBRATION ===");
  
  // Calibrate forward
  Serial.println("Calibrating FORWARD...");
  currentDirection = FORWARD;
  startMovementTracking();
  driveRaw();  // Drive without calibration
  delay(1500);
  autoCalibrateMovement();
  stop();
  endMovementTracking();
  
  delay(1000);
  
  // Calibrate backward
  Serial.println("Calibrating BACKWARD...");
  currentDirection = BACKWARD;
  startMovementTracking();
  backwardsRaw();  // Drive without calibration
  delay(1500);
  autoCalibrateMovement();
  stop();
  endMovementTracking();
  
  printCalibrationStatus();
}

void driveRaw() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
}

void backwardsRaw() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
}

void startMovementTracking() {
  if (!isMoving) {
    isMoving = true;
    movementStartTime = millis();
    movementStartLeftDistance = leftDistance;
    movementStartRightDistance = rightDistance;
  }
}

void endMovementTracking() {
  if (isMoving) {
    isMoving = false;
    unsigned long duration = millis() - movementStartTime;
    float leftTraveled = leftDistance - movementStartLeftDistance;
    float rightTraveled = rightDistance - movementStartRightDistance;
    float avgDistance = (abs(leftTraveled) + abs(rightTraveled)) / 2.0;
    float syncError = abs(leftTraveled - rightTraveled);
    float errorPercent = (avgDistance > 0) ? (syncError / avgDistance * 100) : 0;
    
    // Auto-calibrate at end of movement
    if (calibrationEnabled && avgDistance > 5.0 && (currentDirection == FORWARD || currentDirection == BACKWARD)) {
      autoCalibrateMovement();
    }
    
    // Print report
    Serial.println("\n=== MOVEMENT REPORT ===");
    Serial.print("Direction: ");
    Serial.println(currentDirection == FORWARD ? "FORWARD" : "BACKWARD");
    Serial.print("Duration: ");
    Serial.print(duration);
    Serial.println(" ms");
    Serial.print("Avg distance: ");
    Serial.print(avgDistance, 1);
    Serial.println(" cm");
    Serial.print("Left traveled: ");
    Serial.print(leftTraveled, 1);
    Serial.print(" cm | Right: ");
    Serial.print(rightTraveled, 1);
    Serial.println(" cm");
    Serial.print("Sync error: ");
    Serial.print(syncError, 1);
    Serial.print(" cm (");
    Serial.print(errorPercent, 1);
    Serial.println("%)");
    
    if (leftTraveled > rightTraveled) {
      float percentFaster = ((leftTraveled / rightTraveled) - 1.0) * 100;
      Serial.print("Left wheel ");
      Serial.print(percentFaster, 0);
      Serial.println("% faster → Veers RIGHT");
    } else if (rightTraveled > leftTraveled) {
      float percentFaster = ((rightTraveled / leftTraveled) - 1.0) * 100;
      Serial.print("Right wheel ");
      Serial.print(percentFaster, 0);
      Serial.println("% faster → Veers LEFT");
    }
    
    printCurrentCalibration();
    Serial.println("======================\n");
  }
}

// ===== UTILITY FUNCTIONS =====
void printCalibrationStatus() {
  Serial.println("\n=== CALIBRATION STATUS ===");
  Serial.println("FORWARD:");
  Serial.print("  Left: ");
  Serial.print(leftCalibrationForward * 100, 0);
  Serial.print("% | Right: ");
  Serial.print(rightCalibrationForward * 100, 0);
  Serial.println("%");
  Serial.println("BACKWARD:");
  Serial.print("  Left: ");
  Serial.print(leftCalibrationBackward * 100, 0);
  Serial.print("% | Right: ");
  Serial.print(rightCalibrationBackward * 100, 0);
  Serial.println("%");
  Serial.print("Auto-calibration: ");
  Serial.println(calibrationEnabled ? "ENABLED" : "DISABLED");
  Serial.println("========================\n");
}

void printCurrentCalibration() {
  Serial.print("Calibration applied: ");
  if (currentDirection == FORWARD) {
    Serial.print("FWD L=");
    Serial.print(leftCalibrationForward * 100, 0);
    Serial.print("%, R=");
    Serial.print(rightCalibrationForward * 100, 0);
    Serial.println("%");
  } else if (currentDirection == BACKWARD) {
    Serial.print("BWD L=");
    Serial.print(leftCalibrationBackward * 100, 0);
    Serial.print("%, R=");
    Serial.print(rightCalibrationBackward * 100, 0);
    Serial.println("%");
  }
}