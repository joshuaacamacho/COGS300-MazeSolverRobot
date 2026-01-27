/**
 * @file MyRobot-V1.ino
 * @brief Arduino-based robot controller with serial command interface
 * 
 * This sketch controls a two-wheeled robot using an H-bridge motor driver.
 * It accepts serial commands for movement and speed control.
 * 
 * Hardware Requirements:
 * - Arduino board (Uno, Nano, Mega, etc.)
 * - Dual H-bridge motor driver (L298N, L293D, etc.)
 * - Two DC motors with wheels
 * - Power supply for motors
 * 
 * Serial Commands:
 * - 'w': Move forward
 * - 's': Move backward
 * - 'a': Turn left (pivot)
 * - 'd': Turn right (pivot)
 * - 'j': Increase speed
 * - 'k': Decrease speed
 * - ' ': Stop motors
 * 
 * Motor Connections:
 * - Motor A (Left): enA=9, in1=8, in2=7
 * - Motor B (Right): enB=5, in3=2, in4=4
 * 
 * @author Your Name
 * @version 1.0
 * @date 2024
 */

#include <Arduino.h>

// ===== PIN DEFINITIONS =====
/** @brief Enable pin for Motor A (PWM capable) - controls left wheel speed */
int enA = 9;
/** @brief Direction control pin 1 for Motor A */
int in1 = 8;
/** @brief Direction control pin 2 for Motor A */
int in2 = 7;

/** @brief Enable pin for Motor B (PWM capable) - controls right wheel speed */
int enB = 5;
/** @brief Direction control pin 1 for Motor B */
int in3 = 2;
/** @brief Direction control pin 2 for Motor B */
int in4 = 4;

// ===== SPEED SETTINGS =====
/** 
 * @brief Current motor speed (0-255)
 * @note 150 is approximately 59% of max speed
 */
int currentSpeed = 150;
/** @brief Amount to increase/decrease speed with each adjustment */
const int SPEED_INCREMENT = 25;
/** @brief Maximum allowed speed value (PWM maximum) */
const int MAX_SPEED = 255;

// ===== FUNCTION DECLARATIONS =====
void drive();
void backwards();
void stop();
void turnLeft();
void turnRight();
void speedUp();
void slowDown();

// ===== SETUP =====
/**
 * @brief Initializes the robot hardware and serial communication
 * 
 * This function runs once when the Arduino starts or resets.
 * It configures all motor control pins as outputs and starts
 * serial communication at 9600 baud.
 */
void setup() {
  // Initialize serial communication for command input
  Serial.begin(9600);
  
  // Configure Motor A control pins
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  
  // Configure Motor B control pins
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  // Ensure motors are stopped initially
  stop();
  
  Serial.println("Robot ready - Send commands:");
  Serial.println("  w: Forward");
  Serial.println("  s: Backward");
  Serial.println("  a: Turn left");
  Serial.println("  d: Turn right");
  Serial.println("  j: Speed up");
  Serial.println("  k: Slow down");
  Serial.println("  space: Stop");
  Serial.println("==============================");
}

// ===== MAIN LOOP =====
/**
 * @brief Main program loop - checks for and processes serial commands
 * 
 * Continuously monitors the serial port for incoming commands.
 * When a valid command is received, it calls the corresponding
 * motor control function.
 * 
 * Command processing is blocking - the robot will continue
 * moving in the last commanded direction until a new command
 * is received or stop() is called.
 */
void loop() {
  // Check if serial data is available
  if (Serial.available()) {
    // Read the incoming character
    char command = Serial.read();
    
    // Process the command
    switch(command) {
      case 'w':  // Move forward
        drive();
        break;
      case 's':  // Move backward
        backwards();
        break;
      case 'a':  // Turn left (pivot turn)
        turnLeft();
        break;
      case 'd':  // Turn right (pivot turn)
        turnRight();
        break;
      case 'j':  // Increase speed
        speedUp();
        break;
      case 'k':  // Decrease speed
        slowDown();
        break;
      case ' ':  // Stop all motors
        stop();
        break;
      default:   // Invalid command
        Serial.print("Unknown command: '");
        Serial.print(command);
        Serial.println("'");
        Serial.println("Valid commands: w, s, a, d, j, k, space");
        break;
    }
  }
}

