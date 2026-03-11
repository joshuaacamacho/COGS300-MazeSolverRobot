// ===== LINE FOLLOW =====
void lineFollow() {

  // Check serial during line follow so space works
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
    if (emergencyStop) return;
  }

  int left   = digitalRead(IR_LEFT);
  int center = digitalRead(IR_CENTER);
  int right  = digitalRead(IR_RIGHT);

  Serial.print("L:"); Serial.print(left);
  Serial.print(" C:"); Serial.print(center);
  Serial.print(" R:"); Serial.println(right);

  // ===== TRANSITION: line ended + wall detected on right =====
  float rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  bool lineGone   = (left == 1 && center == 1 && right == 1);
  bool wallSeen   = (rightDist > 0 && rightDist < 50.0);

  if (lineGone && wallSeen) {
    noLineCount++;
    wallSeenCount++;
  } else {
    noLineCount   = 0;
    wallSeenCount = 0;
  }

  if (noLineCount >= 5 && wallSeenCount >= 5) {
    Serial.println("Line ended + wall detected — switching to wall follow");
    noLineCount   = 0;
    wallSeenCount = 0;
    noWallCount   = 0;
    stopMotors();
    delay(500);
    mode    = 'f';
    lastLED = 'f';
    return;
  }

  // ===== LINE FOLLOW LOGIC =====
  if (center == 0 && left == 1 && right == 1) {
    drive(DRIVE_SPEED);
  }
  else if (left == 0 && center == 0 && right == 1) {
    turnLeft();
    delay(50);
  }
  else if (right == 0 && center == 0 && left == 1) {
    turnRight();
    delay(50);
  }
  else {
    stopMotors();
  }
}


// ===== RIGHT WALL FOLLOW =====
void rightWallFollow() {

  // Check serial so space works
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
    if (emergencyStop) return;
  }

  float frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  float rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);

  Serial.print("Front: "); Serial.print(frontDist);
  Serial.print("  Right: "); Serial.println(rightDist);

  // ===== TRANSITION: wall ended → object detection =====
  if (rightDist <= 0 || rightDist > 50.0) {
    noWallCount++;
    Serial.print("No wall count: "); Serial.println(noWallCount);
  } else {
    noWallCount = 0;
  }

  if (noWallCount >= 10) {
    Serial.println("Wall ended — switching to object detection");
    noWallCount        = 0;
    objectLocked       = false;
    currentFacingSteps = 0;
    for (int i = 0; i < NUM_ANGLES; i++)
      belief[i] = 1.0 / NUM_ANGLES;
    stopMotors();
    delay(300);
    mode    = 'o';
    lastLED = 'o';
    return;
  }

  // ===== OBSTACLE AHEAD =====
  // Front wall detected — stop and turn left in place until clear
  if (frontDist > 0 && frontDist < frontStopDist) {
    Serial.println("Front obstacle — turning left");
    stopMotors();
    delay(100);

    // Turn left in place until front is clear
    while (true) {
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        processCommand(cmd);
        if (emergencyStop) return;
      }

      frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
      if (frontDist <= 0 || frontDist > frontStopDist) break;

      // Left turn in place
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(enA, TURN_SPEED);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(enB, TURN_SPEED * LEFT_SCALE);
      delay(50);
    }

    stopMotors();
    delay(200);
    return;
  }

  // ===== WALL FOLLOW PID =====
  // No valid right reading — just drive straight
  if (rightDist <= 0) {
    drive(currentSpeed);
    return;
  }

  float error      = rightDist - targetDistance; // positive = too far, negative = too close
  float correction = error * kp;

  // Clamp correction to avoid spinning
  correction = constrain(correction, -40, 40);

  // Too far from wall — steer right (slow left motor, speed up right)
  // Too close to wall — steer left (slow right motor, speed up left)
  int rightSpeed = constrain((int)(currentSpeed - correction), 60, 200);
  int leftSpeed  = constrain((int)(currentSpeed + correction), 60, 200);

  Serial.print("Error: "); Serial.print(error);
  Serial.print("  Correction: "); Serial.print(correction);
  Serial.print("  R: "); Serial.print(rightSpeed);
  Serial.print("  L: "); Serial.println(leftSpeed);

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, rightSpeed);
  analogWrite(enB, leftSpeed * LEFT_SCALE);
}


// ===== OBJECT DETECTION =====
void runObjectDetection() {

  // Check serial during object detection so space works
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
    if (emergencyStop) return;
  }

  float d = getDistance(TRIG_FRONT, ECHO_FRONT);

  if (d > 2 && d < 15) {
    stopMotors();
    Serial.println("OBJECT REACHED");
    while (true);
  }

  if (objectLocked) {
    driveForwardStep();
    objectLocked = false;
    delay(500);
    return;
  }

  if (currentFacingSteps > 0) {
    Serial.println("Resetting to forward before sweep...");
    rotateLeftSteps(currentFacingSteps);
    delay(300);
    currentFacingSteps = 0;
  }

  for (int i = 0; i < NUM_ANGLES; i++)
    belief[i] = 1.0 / NUM_ANGLES;

  sweep180();
  updateBelief();

  int best = getBestBeliefIndex();

  Serial.print("Best belief index: ");
  Serial.println(best);

  if (best < 0) {
    Serial.println("No valid object found, rescanning...");
    delay(500);
    return;
  }

  int stepsToFace = (NUM_ANGLES - 1) - best;

  Serial.print("Steps to face: ");
  Serial.println(stepsToFace);

  rotateLeftSteps(stepsToFace);
  delay(300);

  currentFacingSteps = best;
  objectLocked = true;
}


// ===== SWEEP =====
void sweep180() {

  Serial.println("---- DEPTH MAP ----");

  for (int i = 0; i < NUM_ANGLES; i++) {

    float d = getDistance(TRIG_FRONT, ECHO_FRONT);
    distances[i] = d;

    Serial.print("Index "); Serial.print(i);
    Serial.print(" Angle "); Serial.print(i * 15);
    Serial.print("  Distance: "); Serial.println(d);

    if (i < NUM_ANGLES - 1) {
      rotateRightSteps(1);
      delay(300);
    }
  }

  Serial.println("-------------------");
}


// ===== BAYES BELIEF UPDATE =====
void updateBelief() {

  for (int i = 0; i < NUM_ANGLES; i++) {
    if (distances[i] <= 0)
      belief[i] = 0;
  }

  for (int i = 1; i < NUM_ANGLES - 1; i++) {

    float left  = distances[i - 1];
    float mid   = distances[i];
    float right = distances[i + 1];

    if (mid <= 0 || left <= 0 || right <= 0) continue;

    float dropLeft   = left - mid;
    float dropRight  = right - mid;
    float likelihood = max(0.0f, dropLeft + dropRight);

    belief[i] = belief[i] * (1.0f + likelihood / 100.0f);
  }

  if (distances[0] > 0 && distances[1] > 0) {
    float drop = distances[1] - distances[0];
    belief[0] = belief[0] * (1.0f + max(0.0f, drop) / 100.0f);
  }

  if (distances[NUM_ANGLES-1] > 0 && distances[NUM_ANGLES-2] > 0) {
    float drop = distances[NUM_ANGLES-2] - distances[NUM_ANGLES-1];
    belief[NUM_ANGLES-1] = belief[NUM_ANGLES-1] * (1.0f + max(0.0f, drop) / 100.0f);
  }

  float sum = 0;
  for (int i = 0; i < NUM_ANGLES; i++) sum += belief[i];
  for (int i = 0; i < NUM_ANGLES; i++) belief[i] /= sum;
}


// ===== BEST BELIEF INDEX =====
int getBestBeliefIndex() {

  float bestScore = -1;
  int   bestIndex = -1;

  for (int i = 0; i < NUM_ANGLES; i++) {

    if (distances[i] <= 0) continue;

    float refDist = distances[i];
    int   width   = 1;

    for (int j = i - 1; j >= 0; j--) {
      if (distances[j] > 0 && abs(distances[j] - refDist) < 30.0) width++;
      else break;
    }

    for (int j = i + 1; j < NUM_ANGLES; j++) {
      if (distances[j] > 0 && abs(distances[j] - refDist) < 30.0) width++;
      else break;
    }

    float score = belief[i] * width;

    Serial.print("Index "); Serial.print(i);
    Serial.print(" belief "); Serial.print(belief[i]);
    Serial.print(" width "); Serial.print(width);
    Serial.print(" score "); Serial.println(score);

    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  return bestIndex;
}