void processCommand(char cmd) {

  cmd = tolower(cmd);

  switch (cmd) {

    case 'w':
      mode = 'm';
      drive(DRIVE_SPEED);
      break;

    case 's':
      mode = 'm';
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(enA, DRIVE_SPEED);
      analogWrite(enB, DRIVE_SPEED * LEFT_SCALE);
      break;

    case 'a':
      mode = 'm';
      turnLeft();
      break;

    case 'd':
      mode = 'm';
      turnRight();
      break;

    case ' ':
      // Hard stop — cancels any mode, keeps LEDs on for debugging
      stopMotors();
      objectLocked = false;
      Serial.println("STOP — motors stopped, mode preserved for debugging");
      // NOTE: mode is NOT reset so LEDs stay on
      break;

    case 'l':
      noLineCount   = 0;
      wallSeenCount = 0;
      mode = 'l';
      break;

    case 'f':
      noWallCount = 0;
      mode = 'f';
      break;

    case 'o':
      objectLocked       = false;
      currentFacingSteps = 0;
      for (int i = 0; i < NUM_ANGLES; i++)
        belief[i] = 1.0 / NUM_ANGLES;
      mode = 'o';
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