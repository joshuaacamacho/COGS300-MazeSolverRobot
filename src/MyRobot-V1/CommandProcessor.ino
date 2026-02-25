void processCommand(char cmd) {

  cmd = tolower(cmd);

  switch (cmd) {

    case 'w':
      manualControl = true;
      isAutoMode = false;
      drive();
      break;

    case 's':
      manualControl = true;
      isAutoMode = false;
      backwards();
      break;

    case 'a':
      manualControl = true;
      isAutoMode = false;
      turnLeft();
      break;

    case 'd':
      manualControl = true;
      isAutoMode = false;
      turnRight();
      break;

    case ' ':
      stopMotors();
      break;

    case 'l':   // Line follow
      manualControl = false;
      isAutoMode = false;
      break;

    case 'f':   // Auto wall follow
      isAutoMode = true;
      manualControl = false;
      break;

    case 'j':
      currentSpeed = min(currentSpeed + 25, 255);
      break;

    case 'k':
      currentSpeed = max(currentSpeed - 25, 0);
      break;
  }
}