void processCommand(char command) {
  Serial.print("Command: ");
  Serial.println(command);

  switch (command) {
    case 'w': drive(); break;
    case 's': backwards(); break;
    case 'a': turnLeft(); break;
    case 'd': turnRight(); break;
    case ' ': 
      endMovementTracking();
      stop();
      isAutoMode = false;
      break;
    case 'c': 
      runQuickCalibration();
      break;
    case 'p': 
      printCalibrationStatus();
      break;
    case 'j': speedUp(); break;
    case 'k': slowDown(); break;
    case 'f': 
      rightWallFollow(); 
      isAutoMode = true;
      break;
    default:
      Serial.println("Unknown command");
      break;
  }
}