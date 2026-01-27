/**
 * @file RobotController.pde
 * @brief Processing-based serial controller for robot navigation
 * 
 * This Processing sketch provides a graphical interface to control a robot
 * via serial communication. It sends keyboard commands to an Arduino
 * that controls the robot's motors.
 * 
 * System Architecture:
 *   Keyboard → Processing → Serial → Arduino → Motor Driver → Robot
 * 
 * Connection Requirements:
 * - Arduino connected via USB with matching baud rate (9600)
 * - Processing Serial library installed
 * - Robot firmware (MyRobot-V1.ino) loaded on Arduino
 * 
 * @author Your Name
 * @version 1.0
 * @date 2024
 */

import processing.serial.*;

/**
 * @brief Serial port object for communication with Arduino
 * 
 * This object manages the serial connection to the Arduino board.
 * It handles opening/closing the port and sending/receiving data.
 */
Serial myPort;

// ===== APPLICATION SETUP =====
/**
 * @brief Initializes the Processing application window and serial connection
 * 
 * This function is called once when the program starts. It:
 * 1. Creates the display window
 * 2. Opens a serial connection to the first available port
 * 3. Sets up text rendering properties
 * 
 * @note The serial port index [0] assumes the Arduino is the first
 *       serial device. If multiple serial devices are connected,
 *       this may need adjustment.
 * 
 * @warning If no serial port is available, this will throw an exception.
 */
void setup() {
  // Create application window (400px wide, 200px tall)
  size(400, 200);
  
  // Initialize serial communication
  // Serial.list()[0] gets the first available serial port
  // 9600 baud matches the Arduino sketch configuration
  myPort = new Serial(this, Serial.list()[0], 9600);
  
  // Configure text rendering
  textAlign(CENTER, CENTER);  // Center text both horizontally and vertically
  textSize(16);               // Set font size to 16 pixels
}

// ===== MAIN DRAW LOOP =====
/**
 * @brief Continuously redraws the application interface
 * 
 * This function is called approximately 60 times per second (Processing's
 * default frame rate). It:
 * 1. Clears the screen with a dark background
 * 2. Displays the control instructions
 * 
 * The interface is minimalist, showing only the available controls.
 * No real-time status feedback from the robot is displayed.
 */
void draw() {
  // Clear screen with dark gray background (RGB: 30,30,30)
  background(30);
  
  // Set text color to white
  fill(255);
  
  // Display control instructions centered in the window
  // \n creates line breaks for better readability
  text("W/A/S/D = move\nJ/K = speed up/down\nSPACE = stop", 
       width/2, height/2);
}

// ===== KEYBOARD INPUT HANDLER =====
/**
 * @brief Handles keyboard input and sends commands to the robot
 * 
 * This function is automatically called by Processing whenever any
 * key is pressed. It filters for valid robot commands and sends
 * them via serial to the Arduino.
 * 
 * Valid Commands:
 * - 'w': Move forward
 * - 'a': Turn left
 * - 's': Move backward  
 * - 'd': Turn right
 * - 'j': Increase speed
 * - 'k': Decrease speed
 * - ' ': Stop (spacebar)
 * 
 * @note The function uses println() for debugging, showing sent commands
 *       in the Processing console.
 * 
 * @param key The character of the key that was pressed
 * @see The corresponding Arduino sketch (MyRobot-V1.ino) for command handling
 */
void keyPressed() {
  // Check if the pressed key is a valid robot command
  if (key == 'w' || key == 'a' || key == 's' || key == 'd' ||
      key == 'j' || key == 'k' || key == ' ') {
    
    // Send the character to Arduino via serial
    myPort.write(key);
    
    // Log the command to the console for debugging
    println("Sent: " + key);
  }
  
  // Note: Non-command keys are silently ignored
}

// ===== ADDITIONAL NOTES =====
/*
 * Potential Enhancements:
 * 
 * 1. Status Feedback:
 *    - Add visual indicators for current speed/direction
 *    - Display connection status
 *    - Show last command sent
 * 
 * 2. Error Handling:
 *    - Try-catch for serial port initialization
 *    - Fallback port selection if [0] fails
 *    - Connection loss detection
 * 
 * 3. User Interface:
 *    - On-screen buttons for touch/mouse control
 *    - Speed slider instead of incremental buttons
 *    - Direction pad visualization
 * 
 * 4. Advanced Features:
 *    - Command history
 *    - Macro command sequences
 *    - Data logging from robot sensors
 */
