/**
 * @file Motor.ino
 * @brief Simple H-bridge motor control helpers.
 *
 * Provides a minimal interface for driving a DC motor using
 * digital GPIO pins (e.g. Arduino-style platforms).
 *
 * The motor direction is controlled via two input pins,
 * while a separate enable pin turns the motor on or off.
 *
 * @author Paul Bucci
 * @date 2026
 */


/**
 * @brief Drives a DC motor in a fixed direction using an H-bridge.
 *
 * @param in1 GPIO pin connected to motor driver input 1 (direction control)
 * @param in2 GPIO pin connected to motor driver input 2 (direction control)
 * @param enA GPIO pin connected to motor driver enable pin (motor on/off)
 */
void drive() {
    digitalWrite(in1, LOW);   // Direction control: IN1
    digitalWrite(in2, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, HIGH);  // Enable motor A driver

    digitalWrite(in3, LOW);   // Direction control: IN1
    digitalWrite(in4, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, HIGH);  // Enable motor B driver
}

void stop() {
    digitalWrite(in1, LOW);   // Direction control: IN1
    digitalWrite(in2, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, LOW);   // Disable motor A driver

    digitalWrite(in3, LOW);   // Direction control: IN1
    digitalWrite(in4, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, LOW);   // Disable motor B driver
}

// TODO: add your own driving functions here

void backwards() {
    digitalWrite(in1, HIGH);   // Direction control: IN1
    digitalWrite(in2, LOW);    // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, HIGH);  // Enable motor A driver

    digitalWrite(in3, HIGH);   // Direction control: IN1
    digitalWrite(in4, LOW);    // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, HIGH);   // Enable motor B driver
}

void turnLeft() {
    digitalWrite(in1, LOW);   // Direction control: IN1
    digitalWrite(in2, LOW);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, LOW);  // Enable motor A driver

    digitalWrite(in3, LOW);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(in4, HIGH);   // Direction control: IN1
    digitalWrite(enB, HIGH);   // Disable motor B driver
}

void turnRight() {
    digitalWrite(in1, LOW);   // Direction control: IN1
    digitalWrite(in2, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, HIGH);   // Disable motor A driver

    digitalWrite(in3, LOW);   // Direction control: IN1
    digitalWrite(in4, LOW);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, LOW);  // Enable motor B driver
}

void speedUp() {
    // Increase speed by increment, but don't exceed max
    if (currentSpeed + SPEED_INCREMENT <= MAX_SPEED) {
        currentSpeed += SPEED_INCREMENT;
    } else {
        currentSpeed = MAX_SPEED;  // Cap at max speed
    }
    
    // Apply the new speed to both motors
    analogWrite(enA, currentSpeed);
    analogWrite(enB, currentSpeed);
    
    // Optional: Print current speed for debugging
    Serial.print("Speed increased to: ");
    Serial.println(currentSpeed);
}

void slowDown() {
    // Decrease speed by increment, but don't go below 0
    if (currentSpeed - SPEED_INCREMENT >= 0) {
        currentSpeed -= SPEED_INCREMENT;
    } else {
        currentSpeed = 0;  // Don't go below 0
    }
    
    // Apply the new speed to both motors
    analogWrite(enA, currentSpeed);
    analogWrite(enB, currentSpeed);
    
    // Optional: Print current speed for debugging
    Serial.print("Speed decreased to: ");
    Serial.println(currentSpeed);
}
