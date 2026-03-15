void processCommand(char cmd) {

  cmd = tolower(cmd);

  switch (cmd) {

    case 'w':
      emergencyStop = false;
      manualCommand = 'w';
      mode    = 'm';
      lastLED = 'm';
      break;

    case 's':
      emergencyStop = false;
      manualCommand = 's';
      mode    = 'm';
      lastLED = 'm';
      break;

    case 'a':
      emergencyStop = false;
      manualCommand = 'a';
      mode    = 'm';
      lastLED = 'm';
      break;

    case 'd':
      emergencyStop = false;
      manualCommand = 'd';
      mode    = 'm';
      lastLED = 'm';
      break;

    case ' ':
      emergencyStop = true;
      manualCommand = ' ';
      stopMotors();
      objectLocked = false;
      Serial.println("STOP — motors stopped, LED preserved");
      break;

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

    case 'f':
      emergencyStop = false;
      manualCommand = ' ';
      noWallCount   = 0;
      stopMotors();
      delay(500);
      mode    = 'f';
      lastLED = 'f';
      break;

    case 'o':
      emergencyStop      = false;
      manualCommand      = ' ';
      objectLocked       = false;
      currentFacingSteps = 0;
      for (int i = 0; i < NUM_ANGLES; i++)
        belief[i] = 1.0 / NUM_ANGLES;
      mode    = 'o';
      lastLED = 'o';
      break;

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