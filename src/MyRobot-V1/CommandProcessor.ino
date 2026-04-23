// =============================================================================
// CommandProcessor.ino
// Handles all incoming single-character serial commands. Called from loop()
// and from within autonomous mode loops so commands are responsive even during
// blocking manoeuvres.
//
// Commands:
//   w / a / s / d  — manual drive (forward / turn-left / reverse / turn-right)
//   l              — start line follow mode
//   f              — start wall follow mode
//   o              — start object detection mode
//   space          — emergency stop (clears on next mode command)
//   j / k          — increase / decrease currentSpeed by 25
// =============================================================================

extern bool emergencyStop;
extern char manualCommand;
extern char mode;
extern char lastLED;
extern int  currentSpeed;
extern int  noLineCount;
extern int  wallSeenCount;
extern int  noWallCount;
extern int  lastTurnDirection;
extern bool approachingLeftTurn;
extern bool approachingRightTurn;
extern bool objectLocked;
extern int  currentFacingSteps;
extern const int NUM_ANGLES;
extern float belief[];

void stopMotors();


// =============================================================================
// processCommand
// Dispatches a single character command. Case-insensitive. Resets all relevant
// state variables when switching modes to prevent stale state carrying over.
//
// Parameters:
//   cmd — the character command received over serial
// =============================================================================
void processCommand(char cmd) {
  cmd = tolower(cmd);

  switch (cmd) {

    // Manual drive commands — switch to manual mode and record direction
    case 'w':
    case 'a':
    case 's':
    case 'd':
      emergencyStop = false;
      manualCommand = cmd;
      mode    = 'm';
      lastLED = 'm';
      break;

    // Emergency stop — halts all motion, preserves LED state
    case ' ':
      emergencyStop = true;
      manualCommand = ' ';
      stopMotors();
      objectLocked = false;
      Serial.println("STOP");
      break;

    // Line follow mode
    case 'l':
      emergencyStop        = false;
      manualCommand        = ' ';
      noLineCount          = 0;
      wallSeenCount        = 0;
      lastTurnDirection    = 0;
      approachingLeftTurn  = false;
      approachingRightTurn = false;
      mode    = 'l';
      lastLED = 'l';
      break;

    // Wall follow mode — brief stop before engaging to let the robot settle
    case 'f':
      emergencyStop = false;
      manualCommand = ' ';
      noWallCount   = 0;
      stopMotors();
      delay(500);
      mode    = 'f';
      lastLED = 'f';
      break;

    // Object detection mode — reset all detection state and belief array
    case 'o':
      emergencyStop      = false;
      manualCommand      = ' ';
      objectLocked       = false;
      currentFacingSteps = 0;
      for (int i = 0; i < NUM_ANGLES; i++)
        belief[i] = 1.0f / NUM_ANGLES;
      mode    = 'o';
      lastLED = 'o';
      break;

    // Speed adjustment for manual mode
    case 'j':
      currentSpeed = min(currentSpeed + 25, 255);
      Serial.print("Speed: "); Serial.println(currentSpeed);
      break;

    case 'k':
      currentSpeed = max(currentSpeed - 25, 0);
      Serial.print("Speed: "); Serial.println(currentSpeed);
      break;
  }
}
