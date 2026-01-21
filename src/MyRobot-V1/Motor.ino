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
void drive(int in1, int in2, int enA) {
    digitalWrite(in1, LOW);   // Direction control: IN1
    digitalWrite(in2, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, HIGH);  // Enable motor A driver

    digitalWrite(in4, LOW);   // Direction control: IN1
    digitalWrite(in3, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, HIGH);  // Enable motor B driver
}

void stop(int in1, int in2, int enA) {
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

    digitalWrite(in4, HIGH);   // Direction control: IN1
    digitalWrite(in3, LOW);    // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, HIGH);   // Enable motor B driver
}

void turnLeft() {
    digitalWrite(in1, LOW);   // Direction control: IN1
    digitalWrite(in2, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, HIGH);  // Enable motor A driver

    digitalWrite(in4, LOW);   // Direction control: IN1
    digitalWrite(in3, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, LOW);   // Disable motor B driver
}

void turnRight() {
    digitalWrite(in1, LOW);   // Direction control: IN1
    digitalWrite(in2, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enA, LOW);   // Disable motor A driver

    digitalWrite(in4, LOW);   // Direction control: IN1
    digitalWrite(in3, HIGH);  // Direction control: IN2 (sets rotation direction)
    digitalWrite(enB, HIGH);  // Enable motor B driver
}

void speedUp() {}

void slowDown() {}
