/**
 * @file Motor.ino
 * @brief Simple H-bridge motor control helpers.
 *
 * Provides a minimal interface for driving a DC motor using
 * digital GPIO pins (e.g. Arduino-style platforms).
 *
 * The motor direction is controlled via two input pins,
 * while a separate enable pin turns the motor on or off.
 */

/**
 * @brief Drives both motors forward at current speed
 * 
 * Sets Motor A and Motor B to rotate forward direction
 * and applies PWM speed to both enable pins.
 * 
 * Motor direction truth table:
 * - in1=LOW, in2=HIGH: Forward
 * - in3=LOW, in4=HIGH: Forward
 */
void drive() {
  // Set Motor A forward direction
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  
  // Set Motor B forward direction
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  
  // Apply PWM speed to both motors
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
  
  Serial.println("Moving FORWARD");
}

/**
 * @brief Drives both motors backward at current speed
 * 
 * Sets Motor A and Motor B to rotate backward direction
 * and applies PWM speed to both enable pins.
 * 
 * Motor direction truth table:
 * - in1=HIGH, in2=LOW: Backward
 * - in3=HIGH, in4=LOW: Backward
 */
void backwards() {
  // Set Motor A backward direction
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  
  // Set Motor B backward direction
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  
  // Apply PWM speed to both motors
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
  
  Serial.println("Moving BACKWARD");
}

/**
 * @brief Stops both motors immediately
 * 
 * Sets all direction pins to LOW and PWM outputs to 0,
 * effectively braking the motors.
 * 
 * Note: Some H-bridge configurations may require different
 * pin states for active braking vs. coasting.
 */
void stop() {
  // Set all direction pins to LOW
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
  // Set PWM outputs to 0 (no power)
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  
  Serial.println("STOPPED");
}

/**
 * @brief Executes a left pivot turn
 * 
 * Stops the left motor (Motor A) and drives the right
 * motor (Motor B) forward, causing the robot to pivot
 * around the left wheel.
 * 
 * For a smoother turn, consider reducing the speed of
 * one motor instead of stopping it completely.
 */
void turnLeft() {
  // Stop left motor (Motor A)
  analogWrite(enA, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  
  // Drive right motor (Motor B) forward
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, currentSpeed);
  
  Serial.println("Turning LEFT (pivot)");
}

/**
 * @brief Executes a right pivot turn
 * 
 * Stops the right motor (Motor B) and drives the left
 * motor (Motor A) forward, causing the robot to pivot
 * around the right wheel.
 * 
 * For a smoother turn, consider reducing the speed of
 * one motor instead of stopping it completely.
 */
void turnRight() {
  // Stop right motor (Motor B)
  analogWrite(enB, 0);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
  // Drive left motor (Motor A) forward
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, currentSpeed);
  
  Serial.println("Turning RIGHT (pivot)");
}

/**
 * @brief Increases the motor speed by SPEED_INCREMENT
 * 
 * Increments currentSpeed by SPEED_INCREMENT, up to MAX_SPEED.
 * Immediately applies the new speed to both motors if they
 * are currently enabled.
 * 
 * @note Speed changes take effect immediately and persist
 * until changed again or power is cycled.
 */
void speedUp() {
  // Increase speed, but not above MAX_SPEED
  currentSpeed = min(currentSpeed + SPEED_INCREMENT, MAX_SPEED);
  
  // Apply new speed to both motors
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
  
  Serial.print("Speed INCREASED to: ");
  Serial.println(currentSpeed);
}

/**
 * @brief Decreases the motor speed by SPEED_INCREMENT
 * 
 * Decrements currentSpeed by SPEED_INCREMENT, down to 0.
 * Immediately applies the new speed to both motors if they
 * are currently enabled.
 * 
 * @note Speed changes take effect immediately and persist
 * until changed again or power is cycled.
 */
void slowDown() {
  // Decrease speed, but not below 0
  currentSpeed = max(currentSpeed - SPEED_INCREMENT, 0);
  
  // Apply new speed to both motors
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);
  
  Serial.print("Speed DECREASED to: ");
  Serial.println(currentSpeed);

  //If speed reaches 0, actually stop the motors
  if (currentSpeed == 0) {
    stop();
}