// =============================================================================
// AutoMode.ino
// Implements all three autonomous modes:
//   - lineFollow()         : follows a dark tape line using three IR sensors
//   - rightWallFollow()    : maintains a fixed distance from the right wall
//   - runObjectDetection() : sweeps, identifies, and drives toward an object
//
// Supporting routines for the sweep (sweep180, updateBelief, getBestBeliefIndex)
// are also defined here.
// =============================================================================

// Convenience macro — checks for an incoming serial command and processes it.
// Used inside blocking loops so the robot remains responsive during manoeuvres.
#define CHECK_CMD() \
  if (Serial.available() > 0) { processCommand(Serial.read()); if (emergencyStop) return; }


// =============================================================================
// lineFollow
// Follows a tape line using three IR sensors (left, center, right).
// 0 = on tape, 1 = off tape.
//
// Steering logic mirrors discrete IR states:
//   center only       — drive straight
//   center + right    — minor left drift, pivot right until right sensor clears
//   center + left     — minor right drift, pivot left until left sensor clears
//   right only        — major left drift, hard pivot right until center reacquires
//   left only         — major right drift, hard pivot left until center reacquires
//   all off           — lost line, behaviour depends on last known turn direction
//
// Transition: if the line has been gone for 5 consecutive ultrasonic checks
// and the right wall is visible, the mode switches to wall follow.
// =============================================================================
void lineFollow() {
  CHECK_CMD();

  int left   = digitalRead(IR_LEFT);
  int center = digitalRead(IR_CENTER);
  int right  = digitalRead(IR_RIGHT);

  Serial.print("L:"); Serial.print(left);
  Serial.print(" C:"); Serial.print(center);
  Serial.print(" R:"); Serial.println(right);

  // Check for line-end transition every 10 calls to avoid slowing the control loop
  static int ultrasonicCounter = 0;
  if (++ultrasonicCounter >= 10) {
    ultrasonicCounter = 0;
    float rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
    bool  lineGone  = (left == 1 && center == 1 && right == 1);
    bool  wallSeen  = (rightDist > 0 && rightDist < 50.0f);

    if (lineGone && wallSeen) { noLineCount++; wallSeenCount++; }
    else                      { noLineCount = 0; wallSeenCount = 0; }

    if (noLineCount >= 5 && wallSeenCount >= 5) {
      Serial.println("Line ended — switching to wall follow");
      noLineCount = wallSeenCount = noWallCount = 0;
      stopMotors(); delay(500);
      mode = lastLED = 'f';
      return;
    }
  }

  // On line — drive straight
  if (center == 0 && left == 1 && right == 1) {
    lastTurnDirection = 0;
    approachingLeftTurn = approachingRightTurn = false;
    drive(DRIVE_SPEED);
  }

  // Minor left drift — pivot right until right sensor clears tape
  else if (center == 0 && right == 0 && left == 1) {
    lastTurnDirection = 1; approachingRightTurn = true; approachingLeftTurn = false;
    noLineCount = 0;
    Serial.println("Minor left drift — right pivot");
    while (true) {
      CHECK_CMD();
      if (digitalRead(IR_RIGHT) == 1) break;
      digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  analogWrite(enA, 0);
      digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, PIVOT_SPEED);
    }
    stopMotors(); delay(50);
  }

  // Minor right drift — pivot left until left sensor clears tape
  else if (center == 0 && left == 0 && right == 1) {
    lastTurnDirection = -1; approachingLeftTurn = true; approachingRightTurn = false;
    noLineCount = 0;
    Serial.println("Minor right drift — left pivot");
    while (true) {
      CHECK_CMD();
      if (digitalRead(IR_LEFT) == 1) break;
      digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, PIVOT_SPEED);
      digitalWrite(in3, LOW);  digitalWrite(in4, LOW);  analogWrite(enB, 0);
    }
    stopMotors(); delay(50);
  }

  // Major left drift — hard pivot right until center reacquires tape
  else if (center == 1 && right == 0 && left == 1) {
    lastTurnDirection = 1; approachingRightTurn = true; approachingLeftTurn = false;
    noLineCount = 0;
    Serial.println("Major left drift — hard right pivot");
    while (true) {
      CHECK_CMD();
      if (digitalRead(IR_CENTER) == 0) break;
      digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  analogWrite(enA, 0);
      digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, PIVOT_SPEED);
    }
    stopMotors(); delay(100);
  }

  // Major right drift — hard pivot left until center reacquires tape
  else if (center == 1 && left == 0 && right == 1) {
    lastTurnDirection = -1; approachingLeftTurn = true; approachingRightTurn = false;
    noLineCount = 0;
    Serial.println("Major right drift — hard left pivot");
    while (true) {
      CHECK_CMD();
      if (digitalRead(IR_CENTER) == 0) break;
      digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, PIVOT_SPEED);
      digitalWrite(in3, LOW);  digitalWrite(in4, LOW);  analogWrite(enB, 0);
    }
    stopMotors(); delay(100);
  }

  // All sensors off — line lost
  else {
    if (approachingLeftTurn) {
      // Burst forward to check whether this is a junction or a genuine left turn
      Serial.println("Lost line — burst to check junction");
      bool rightFound = false;
      unsigned long burstStart = millis();
      while (millis() - burstStart < 200) {
        CHECK_CMD();
        drive(DRIVE_SPEED);
        if (digitalRead(IR_RIGHT) == 0) { rightFound = true; break; }
      }
      stopMotors(); delay(50);

      if (rightFound) {
        // Junction — override to right pivot
        Serial.println("Junction — right pivot");
        approachingLeftTurn = approachingRightTurn = false;
        while (true) {
          CHECK_CMD();
          if (digitalRead(IR_CENTER) == 0) break;
          digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  analogWrite(enA, 0);
          digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, PIVOT_SPEED);
        }
        stopMotors(); delay(100);
        lastTurnDirection = 1;
      } else {
        // Genuine left turn — reverse slightly then pivot left
        Serial.println("Left turn — reverse then left pivot");
        approachingLeftTurn = false;
        unsigned long revStart = millis();
        while (millis() - revStart < 200) {
          CHECK_CMD();
          digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(enA, DRIVE_SPEED);
          digitalWrite(in3, LOW);  digitalWrite(in4, HIGH); analogWrite(enB, (int)(DRIVE_SPEED * LEFT_SCALE));
        }
        stopMotors(); delay(50);
        while (true) {
          CHECK_CMD();
          if (digitalRead(IR_CENTER) == 0) break;
          digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, PIVOT_SPEED);
          digitalWrite(in3, LOW);  digitalWrite(in4, LOW);  analogWrite(enB, 0);
        }
        stopMotors(); delay(100);
        lastTurnDirection = -1;
      }
    }

    else if (approachingRightTurn) {
      // Committed right turn — pivot right until center reacquires
      Serial.println("Lost line — right pivot");
      approachingRightTurn = false;
      while (true) {
        CHECK_CMD();
        if (digitalRead(IR_CENTER) == 0) break;
        digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  analogWrite(enA, 0);
        digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, PIVOT_SPEED);
      }
      stopMotors(); delay(100);
      lastTurnDirection = 1;
    }

    else {
      // No context — drive straight and hope to reacquire
      Serial.println("Lost line — straight");
      drive(DRIVE_SPEED);
    }
  }
}


// =============================================================================
// rightWallFollow
// Maintains a fixed distance from the right wall using a rolling average of
// ultrasonic readings and differential motor correction.
//
//   - Rolling average over NUM_READINGS samples for noise rejection
//   - 10% deadband around target — no correction within this band
//   - Differential boost correction — both motors always running, one gets
//     a small speed boost to steer; a motor is never stopped mid-straight
//   - Front wall response: reverse then pivot left (always left for right-wall
//     following), using a turn_state flag to alternate pivot direction as a fallback
// =============================================================================
void rightWallFollow() {

  #define WF_MOTOR_MAX   55
  #define WF_CORRECTION  10
  #define WF_READINGS     5
  #define WF_TARGET       8   // target distance from right wall in cm
  #define WF_DEADBAND  0.1f   // 10% deadband fraction

  static int  rightBuf[WF_READINGS];
  static int  frontBuf[WF_READINGS];
  static bool init = false;
  static bool turn_state = false;

  if (!init) {
    for (int i = 0; i < WF_READINGS; i++) {
      rightBuf[i] = WF_TARGET;
      frontBuf[i] = 1023;
    }
    init = true;
  }

  CHECK_CMD();

  // Push a new reading into the front of the buffer and return the average.
  // Older readings shift back by one position each call.
  auto rollingAvg = [](float newVal, int* buf) -> float {
    float avg = newVal;
    for (int i = 0; i < WF_READINGS - 1; i++) {
      avg       += buf[i];
      buf[i + 1] = buf[i];
    }
    buf[0] = (int)newVal;
    return avg / WF_READINGS;
  };

  float frontDist = getDistance(TRIG_FRONT, ECHO_FRONT);
  if (frontDist == -1) frontDist = 1023; // or some large value to ignore

  float rightDist = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  if (rightDist == -1) rightDist = 1023;

  // Front wall response: reverse, then pivot left to turn away from the wall
  float frontAvg = rollingAvg(frontDist, frontBuf);
  if (frontAvg < WF_TARGET) {
    Serial.println("Front wall — reverse + left pivot");
    stopMotors(); delay(50);

    unsigned long rev = millis();
    while (millis() - rev < 500) {
      CHECK_CMD();
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(enA, WF_MOTOR_MAX);
      digitalWrite(in3, LOW);  digitalWrite(in4, HIGH); analogWrite(enB, WF_MOTOR_MAX);
    }
    stopMotors();

    unsigned long piv = millis();
    while (millis() - piv < 500) {
      CHECK_CMD();
      // Always pivot left — right wall follow always turns left at a front wall
      digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, 0);
      digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, WF_MOTOR_MAX);
    }
    turn_state = !turn_state;

    drive(WF_MOTOR_MAX);
    return;
  }

  CHECK_CMD();

  // Right wall tracking — differential correction within a 10% deadband
  float rightAvg = rollingAvg(rightDist, rightBuf);

  Serial.print("R:"); Serial.print(rightAvg);
  Serial.print(" T:"); Serial.println(WF_TARGET);

  float lower = WF_TARGET * (1.0f - WF_DEADBAND);
  float upper = WF_TARGET * (1.0f + WF_DEADBAND);

  if (rightAvg >= lower && rightAvg <= upper) {
    // On target — drive straight
    drive(WF_MOTOR_MAX);

  } else if (rightAvg > upper) {
    // Too far from wall — boost left motor to steer right toward wall
    Serial.println("Too far — steer right");
    digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, WF_MOTOR_MAX + WF_CORRECTION);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, WF_MOTOR_MAX);

  } else {
    // Too close to wall — boost right motor to steer left away from wall
    Serial.println("Too close — steer left");
    digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(enA, WF_MOTOR_MAX);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  analogWrite(enB, WF_MOTOR_MAX + WF_CORRECTION);
  }

  #undef WF_MOTOR_MAX
  #undef WF_CORRECTION
  #undef WF_READINGS
  #undef WF_TARGET
  #undef WF_DEADBAND
}


// =============================================================================
// runObjectDetection
// Scans 180 degrees, applies a Bayesian belief update to identify the most
// likely object location, rotates to face it, then drives toward it.
//
// Flow:
//   1. If already locked, drive toward object (driveForwardStep handles arrival)
//   2. Reset heading to forward if offset from a previous sweep
//   3. Sweep, update belief, find best index
//   4. Rotate to face best index and lock
// =============================================================================
void runObjectDetection() {
  CHECK_CMD();

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

  // Return to forward-facing heading before sweeping
  if (currentFacingSteps > 0) {
    Serial.println("Resetting heading before sweep");
    rotateLeftSteps(currentFacingSteps);
    delay(300);
    currentFacingSteps = 0;
  }

  for (int i = 0; i < NUM_ANGLES; i++)
    belief[i] = 1.0f / NUM_ANGLES;

  sweep180();
  updateBelief();

  int best = getBestBeliefIndex();
  Serial.print("Best index: "); Serial.println(best);

  if (best < 0) {
    Serial.println("No object found — rescanning");
    delay(500);
    return;
  }

  int stepsToFace = (NUM_ANGLES - 1) - best;
  Serial.print("Steps to face: "); Serial.println(stepsToFace);
  rotateLeftSteps(stepsToFace);
  delay(300);

  currentFacingSteps = best;
  objectLocked = true;
}


// =============================================================================
// sweep180
// Rotates the robot rightward one step at a time, recording a front distance
// measurement at each of the NUM_ANGLES angular positions.
// Results are stored in the global distances[] array.
// =============================================================================
void sweep180() {
  Serial.println("--- SWEEP ---");
  for (int i = 0; i < NUM_ANGLES; i++) {
    distances[i] = getDistance(TRIG_FRONT, ECHO_FRONT);
    Serial.print("i="); Serial.print(i);
    Serial.print(" deg="); Serial.print(i * 15);
    Serial.print(" dist="); Serial.println(distances[i]);
    if (i < NUM_ANGLES - 1) { rotateRightSteps(1); delay(300); }
  }
  Serial.println("--- END ---");
}


// =============================================================================
// updateBelief
// Bayesian update over the distances[] array. Angles where distances drop
// sharply relative to their neighbours are more likely to be objects — the
// likelihood term rewards local minima. The belief array is normalised after
// the update so it remains a valid probability distribution.
// =============================================================================
void updateBelief() {
  // Zero out invalid readings
  for (int i = 0; i < NUM_ANGLES; i++)
    if (distances[i] <= 0) belief[i] = 0;

  // Reward local minima — indices where both neighbours are farther away
  for (int i = 1; i < NUM_ANGLES - 1; i++) {
    float l = distances[i - 1], m = distances[i], r = distances[i + 1];
    if (m <= 0 || l <= 0 || r <= 0) continue;
    float likelihood = max(0.0f, (l - m) + (r - m));
    belief[i] *= (1.0f + likelihood / 100.0f);
  }

  // Edge cases for first and last angles
  if (distances[0] > 0 && distances[1] > 0)
    belief[0] *= (1.0f + max(0.0f, distances[1] - distances[0]) / 100.0f);

  if (distances[NUM_ANGLES - 1] > 0 && distances[NUM_ANGLES - 2] > 0)
    belief[NUM_ANGLES - 1] *= (1.0f + max(0.0f, distances[NUM_ANGLES - 2] - distances[NUM_ANGLES - 1]) / 100.0f);

  // Normalise
  float sum = 0;
  for (int i = 0; i < NUM_ANGLES; i++) sum += belief[i];
  if (sum > 0)
    for (int i = 0; i < NUM_ANGLES; i++) belief[i] /= sum;
}


// =============================================================================
// getBestBeliefIndex
// Finds the sweep index with the highest combined score of belief value and
// angular width of its object cluster. Width is measured by how many adjacent
// readings fall within 30 cm of the reference distance — wider clusters at
// high belief indicate a more confident object detection.
//
// Returns the best index, or -1 if no valid reading exists.
// =============================================================================
int getBestBeliefIndex() {
  float bestScore = -1;
  int   bestIndex = -1;

  for (int i = 0; i < NUM_ANGLES; i++) {
    if (distances[i] <= 0) continue;

    float refDist = distances[i];
    int   width   = 1;

    for (int j = i - 1; j >= 0; j--) {
      if (distances[j] > 0 && abs(distances[j] - refDist) < 30.0f) width++; else break;
    }
    for (int j = i + 1; j < NUM_ANGLES; j++) {
      if (distances[j] > 0 && abs(distances[j] - refDist) < 30.0f) width++; else break;
    }

    float score = belief[i] * width;
    Serial.print("i="); Serial.print(i);
    Serial.print(" belief="); Serial.print(belief[i]);
    Serial.print(" width="); Serial.print(width);
    Serial.print(" score="); Serial.println(score);

    if (score > bestScore) { bestScore = score; bestIndex = i; }
  }

  return bestIndex;
}

#undef CHECK_CMD
