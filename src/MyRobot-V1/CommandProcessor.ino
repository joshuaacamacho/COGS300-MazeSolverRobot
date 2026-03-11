void processCommand(char cmd) {

  cmd = tolower(cmd);

  switch (cmd) {

    case 'w':
      emergencyStop = false;
      mode    = 'm';
      lastLED = 'm';
      drive(DRIVE_SPEED);
      break;

    case 's':
      emergencyStop = false;
      mode    = 'm';
      lastLED = 'm';
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(enA, DRIVE_SPEED);
      analogWrite(enB, DRIVE_SPEED * LEFT_SCALE);
      break;

    case 'a':
      emergencyStop = false;
      mode    = 'm';
      lastLED = 'm';
      turnLeft();
      break;

    case 'd':
      emergencyStop = false;
      mode    = 'm';
      lastLED = 'm';
      turnRight();
      break;

    case ' ':
      // Emergency stop — keeps LED on last active mode
      emergencyStop = true;
      stopMotors();
      objectLocked = false;
      Serial.println("STOP — motors stopped, LED preserved");
      break;

    case 'l':
      emergencyStop = false;
      noLineCount   = 0;
      wallSeenCount = 0;
      mode    = 'l';
      lastLED = 'l';
      break;

    case 'f':
      emergencyStop = false;
      noWallCount = 0;
      stopMotors();
      delay(500);
      mode    = 'f';
      lastLED = 'f';
      break;

    case 'o':
      emergencyStop      = false;
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