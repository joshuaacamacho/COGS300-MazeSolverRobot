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
  static int ultrasonicCounter = 0;
  ultrasonicCounter++;
  if (ultrasonicCounter >= 10) {
    ultrasonicCounter = 0;
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
  }

  // ===== LINE FOLLOW LOGIC =====
  // 0 = on tape, 1 = off tape

  // Perfectly centered — drive straight, reset flags
  if (center == 0 && left == 1 && right == 1) {
    lastTurnDirection    = 0;
    approachingLeftTurn  = false;
    approachingRightTurn = false;
    drive(DRIVE_SPEED);
  }

  // Minor drift left (center + right on tape) — turn RIGHT
  else if (center == 0 && right == 0 && left == 1) {
    lastTurnDirection    = 1;
    approachingRightTurn = true;
    approachingLeftTurn  = false;
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
      analogWrite(enA, 0);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      analogWrite(enB, PIVOT_SPEED);
    }
    stopMotors();
    delay(50);
  }

  // Minor drift right (center + left on tape) — turn LEFT
  else if (center == 0 && left == 0 && right == 1) {
    lastTurnDirection    = -1;
    approachingLeftTurn  = true;
    approachingRightTurn = false;
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
      analogWrite(enA, PIVOT_SPEED);
      digitalWrite(in3, LOW);
      digitalWrite(in4, LOW);
      analogWrite(enB, 0);
    }
    stopMotors();
    delay(50);
  }

  // Major drift left (only right on tape) — turn RIGHT hard
  else if (center == 1 && right == 0 && left == 1) {
    lastTurnDirection    = 1;
    approachingRightTurn = true;
    approachingLeftTurn  = false;
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
      analogWrite(enA, 0);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      analogWrite(enB, PIVOT_SPEED);
    }
    stopMotors();
    delay(100);
  }

  // Major drift right (only left on tape) — turn LEFT hard
  else if (center == 1 && left == 0 && right == 1) {
    lastTurnDirection    = -1;
    approachingLeftTurn  = true;
    approachingRightTurn = false;
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
      analogWrite(enA, PIVOT_SPEED);
      digitalWrite(in3, LOW);
      digitalWrite(in4, LOW);
      analogWrite(enB, 0);
    }
    stopMotors();
    delay(100);
  }

  // Lost line — use flags for direction
  else {
    if (approachingLeftTurn) {
      // Drive forward briefly to see if right sensor catches junction tape
      Serial.println("Lost line — forward burst to check for junction");
      unsigned long burstStart = millis();
      bool rightFound = false;

      while (millis() - burstStart < 200) {
        if (Serial.available() > 0) {
          char c = Serial.read();
          processCommand(c);
          if (emergencyStop) return;
        }
        drive(DRIVE_SPEED);
        int r = digitalRead(IR_RIGHT);
        if (r == 0) { rightFound = true; break; }
      }

      stopMotors();
      delay(50);

      if (rightFound) {
        // Junction detected — override to right pivot
        Serial.println("Junction detected — overriding to right pivot");
        approachingLeftTurn  = false;
        approachingRightTurn = false;

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
          analogWrite(enA, 0);
          digitalWrite(in3, HIGH);
          digitalWrite(in4, LOW);
          analogWrite(enB, PIVOT_SPEED);
        }
        stopMotors();
        delay(100);
        lastTurnDirection = 1;
      }
      else {
        // No junction — overshot, reverse then pivot left
        Serial.println("Not a junction — reversing then left pivot");
        approachingLeftTurn = false;

        // Reverse briefly to get back to turn
        unsigned long revStart = millis();
        while (millis() - revStart < 200) {
          if (Serial.available() > 0) {
            char c = Serial.read();
            processCommand(c);
            if (emergencyStop) return;
          }
          digitalWrite(in1, HIGH);
          digitalWrite(in2, LOW);
          analogWrite(enA, DRIVE_SPEED);
          digitalWrite(in3, LOW);
          digitalWrite(in4, HIGH);
          analogWrite(enB, DRIVE_SPEED * LEFT_SCALE);
        }
        stopMotors();
        delay(50);

        // Pivot left until center finds tape
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
          analogWrite(enA, PIVOT_SPEED);
          digitalWrite(in3, LOW);
          digitalWrite(in4, LOW);
          analogWrite(enB, 0);
        }
        stopMotors();
        delay(100);
        lastTurnDirection = -1;
      }
    }

    else if (approachingRightTurn) {
      Serial.println("Lost line — aggressive right pivot");
      approachingRightTurn = false;

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
        analogWrite(enA, 0);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        analogWrite(enB, PIVOT_SPEED);
      }
      stopMotors();
      delay(100);
      lastTurnDirection = 1;
    }

    else {
      Serial.println("Lost line — driving straight");
      drive(DRIVE_SPEED);
    }
  }
}


// ===== FILTERED RIGHT DISTANCE =====
float getFilteredRight() {
  const int N = 5;
  static float readings[N] = {25, 25, 25, 25, 25};
  static float lastGood = 25.0;
  static int idx = 0;

  float d = getStableDistance(TRIG_RIGHT, ECHO_RIGHT);

  if (d <= 0 || d > 150 || abs(d - lastGood) > 25.0) {
    return lastGood;
  }

  lastGood = d;
  readings[idx] = d;
  idx = (idx + 1) % N;

  float sum = 0;
  for (int i = 0; i < N; i++) sum += readings[i];
  return sum / N;
}

// ===== RIGHT WALL FOLLOW =====
void rightWallFollow() {

  if (Serial.available() > 0) {
    char cmd = Serial.read();
    processCommand(cmd);
    if (emergencyStop) return;
  }

  float frontDist = getStableDistance(TRIG_FRONT, ECHO_FRONT);
  float rightDist = getFilteredRight();

  Serial.print("Front: "); Serial.print(frontDist);
  Serial.print("  Right: "); Serial.println(rightDist);

  // ===== TRANSITION: wall ended → object detection =====
  if (rightDist <= 0 || rightDist > 60.0) {
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

  // ===== FRONT WALL + RIGHT WALL CLOSE = LEFT TURN =====
  if (frontDist > 0 && frontDist < 20.0 && rightDist > 0 && rightDist < 30.0) {
    Serial.println("Front wall + right wall close — turning left until clear");
    stopMotors();
    delay(100);

    while (true) {
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        processCommand(cmd);
        if (emergencyStop) return;
      }

      frontDist = getStableDistance(TRIG_FRONT, ECHO_FRONT);
      if (frontDist <= 0 || frontDist > 20.0) break;

      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(enA, PIVOT_SPEED);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(enB, max((int)(PIVOT_SPEED * LEFT_SCALE), 65));
      delay(50);
    }

    stopMotors();
    delay(200);
    return;
  }

  // ===== RIGHT WALL LOST — dual motor turn right hard =====
  if (rightDist <= 0 || rightDist > 60.0) {
    Serial.println("Right wall lost — turning right hard");
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, PIVOT_SPEED);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, max((int)(PIVOT_SPEED * LEFT_SCALE), 65));
    return;
  }

  // ===== BAND: 40-60cm — single left motor, hard right correction =====
  if (rightDist > 40.0) {
    Serial.println("Band: 40-60cm — hard right correction");
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(enA, 0);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, max((int)(65 * LEFT_SCALE), 65));
    return;
  }

  // ===== BAND: 30-40cm — moderate right correction =====
  if (rightDist > 30.0) {
    Serial.println("Band: 30-40cm — moderate right correction");
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, 65);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, max((int)(85 * LEFT_SCALE), 65));
    return;
  }

  // ===== BAND: 20-30cm — sweet spot, drive straight =====
  if (rightDist >= 20.0) {
    Serial.println("Band: 20-30cm — straight");
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, 65);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, max((int)(65 * LEFT_SCALE), 65));
    return;
  }

  // ===== BAND: 10-20cm — nudge left =====
  if (rightDist >= 10.0) {
    Serial.println("Band: 10-20cm — nudge left");
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, 85);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, max((int)(65 * LEFT_SCALE), 65));
    return;
  }

  // ===== BAND: <10cm — dual motor pivot left hard =====
  Serial.println("Band: <10cm — pivot left hard");
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, PIVOT_SPEED);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, max((int)(PIVOT_SPEED * LEFT_SCALE), 65));
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