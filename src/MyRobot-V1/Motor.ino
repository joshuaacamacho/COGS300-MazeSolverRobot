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
 * @param in1 GPIO pin connected to motor driver A input 1 (direction control)
 * @param in2 GPIO pin connected to motor driver A input 2 (direction control)
 * @param enA GPIO pin connected to motor driver A enable pin (motor on/off)
*  @param in3 GPIO pin connected to motor driver B input 1 (direction control)
 * @param in4 GPIO pin connected to motor driver B input 2 (direction control)
 * @param enB GPIO pin connected to motor driver B enable pin (motor on/off)
 */
void drive() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);

    analogWrite(enA, currentSpeed);
    analogWrite(enB, currentSpeed);
}

void stop() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);

    analogWrite(enA, 0);
    analogWrite(enB, 0);
}

// TODO: add your own driving functions here

void backwards() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);

    analogWrite(enA, currentSpeed);
    analogWrite(enB, currentSpeed);
}

void turnLeft() {
    // Left motor stopped
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(enA, 0);

    // Right motor forward
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    analogWrite(enB, currentSpeed);
}

void turnRight() {
    // Right motor stopped
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    analogWrite(enB, 0);

    // Left motor forward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, currentSpeed);
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
