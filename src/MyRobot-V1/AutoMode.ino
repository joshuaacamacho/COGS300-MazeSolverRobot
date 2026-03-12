// ===== LINE FOLLOW =====
void lineFollow() {

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
  // 0 = on tape, 1 = off tape

  // Perfectly centered — drive straight
  if (center == 0 && left == 1 && right == 1) {
    drive(DRIVE_SPEED);
  }

  // Minor drift left (center + right on tape) — turn RIGHT
  // R:0 means robot drifted left, right sensor now on tape, turn right to correct
  else if (center == 0 && right == 0 && left == 1) {
    noLineCount = 0;
    Serial.println("Minor drift left — right pivot");

    while (true) {
      if (Serial.available() > 0) {
        char c = Serial.read();
        processCommand(c);
        if (emergencyStop) return;
      }
      right = digitalRead(IR_RIGHT);
      if (right == 1) break;
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      analogWrite(enA, 0);                        // right motor stopped
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      analogWrite(enB, DRIVE_SPEED * LEFT_SCALE); // left motor drives
    }
    stopMotors();
    delay(50);
  }

  // Minor drift right (center + left on tape) — turn LEFT
  // L:0 means robot drifted right, left sensor now on tape, turn left to correct
  else if (center == 0 && left == 0 && right == 1) {
    noLineCount = 0;
    Serial.println("Minor drift right — left pivot");

    while (true) {
      if (Serial.available() > 0) {
        char c = Serial.read();
        processCommand(c);
        if (emergencyStop) return;
      }
      left = digitalRead(IR_LEFT);
      if (left == 1) break;
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(enA, DRIVE_SPEED);              // right motor drives
      digitalWrite(in3, LOW);
      digitalWrite(in4, LOW);
      analogWrite(enB, 0);                        // left motor stopped
    }
    stopMotors();
    delay(50);
  }

  // Major drift left (only right on tape, center gone) — turn RIGHT hard
  // R:0 C:1 means robot drifted far left, turn right hard to recover
  else if (center == 1 && right == 0 && left == 1) {
    noLineCount = 0;
    Serial.println("Major drift left — hard right pivot");

    while (true) {
      if (Serial.available() > 0) {
        char c = Serial.read();
        processCommand(c);
        if (emergencyStop) return;
      }
      center = digitalRead(IR_CENTER);
      if (center == 0) break;
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      analogWrite(enA, 0);                        // right motor stopped
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      analogWrite(enB, DRIVE_SPEED * LEFT_SCALE); // left motor drives
    }
    stopMotors();
    delay(100);
  }

  // Major drift right (only left on tape, center gone) — turn LEFT hard
  // L:0 C:1 means robot drifted far right, turn left hard to recover
  else if (center == 1 && left == 0 && right == 1) {
    noLineCount = 0;
    Serial.println("Major drift right — hard left pivot");

    while (true) {
      if (Serial.available() > 0) {
        char c = Serial.read();
        processCommand(c);
        if (emergencyStop) return;
      }
      center = digitalRead(IR_CENTER);
      if (center == 0) break;
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(enA, DRIVE_SPEED);              // right motor drives
      digitalWrite(in3, LOW);
      digitalWrite(in4, LOW);
      analogWrite(enB, 0);                        // left motor stopped
    }
    stopMotors();
    delay(100);
  }

  // Lost line completely — stop
  else {
    stopMotors();
  }
}


// ===== RIGHT WALL FOLLOW =====
void rightWallFollow() {

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
  if (frontDist > 0 && frontDist < frontStopDist) {
    Serial.println("Front obstacle — turning left");
    stopMotors();
    delay(100);

    while (true) {
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        processCommand(cmd);
        if (emergencyStop) return;
      }

      frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
      if (frontDist <= 0 || frontDist > frontStopDist) break;

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
  if (rightDist <= 0) {
    drive(currentSpeed);
    return;
  }

  float error      = rightDist - targetDistance;
  float correction = constrain(error * kp, -40, 40);

  int rightSpeed = constrain((int)(currentSpeed - correction), 70, 200);
  int leftSpeed  = constrain((int)(currentSpeed + correction), 70, 200);

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