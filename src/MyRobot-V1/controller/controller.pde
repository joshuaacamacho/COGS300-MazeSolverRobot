/**
 * @file WiFiRobotController.pde
 * @brief Processing-based WiFi controller for Arduino robot
 * @details This Processing sketch provides a graphical interface to control
 *          an Arduino UNO R4 WiFi robot over a WiFi network. It connects to
 *          the Arduino's TCP server and sends control commands when keys
 *          are pressed.
 * 
 * @section architecture System Architecture
 *                        [This GUI] --- WiFi/TCP ---> [Arduino Server] ---> Motors
 * 
 * @section controls Control Mapping
 *                    W Key - Move robot forward
 *                    S Key - Move robot backward  
 *                    A Key - Turn robot left
 *                    D Key - Turn robot right
 *                    J Key - Increase speed
 *                    K Key - Decrease speed
 *                    Space - Stop all motors
 * 
 * @section requirements Requirements
 *                       - Processing 3.5.4 or later
 *                       - Processing Network library (built-in)
 *                       - Arduino and Processing on same WiFi network
 * 
 * @section setup Setup Instructions
 *                1. Upload WiFiRobot.ino to Arduino UNO R4 WiFi
 *                2. Note Arduino's IP address from Serial Monitor
 *                3. Update arduinoIP variable below (line 49)
 *                4. Run this Processing sketch
 *                5. Press control keys to operate robot
 * 
 * @author Joshua Camacho
 * @version 1.0.0
 * @date 2026
 */

import processing.net.*;  // Required for TCP network communication

// ===== NETWORK CONFIGURATION ================================================
/**
 * @brief Client object for TCP communication with Arduino
 * @details Manages the connection to the Arduino's WiFi server,
 *          handles data transmission, and monitors connection status.
 */
Client client;

/**
 * @brief Arduino's IP address on the local network
 * @details Must be updated to match Arduino's actual IP address.
 *          Find this in the Arduino Serial Monitor after uploading.
 * @note Default is a placeholder - MUST BE CHANGED for operation
 */
String arduinoIP = "192.168.1.100";  ///< REPLACE with Arduino IP

/**
 * @brief TCP port number for communication
 * @details Must match the port defined in the Arduino sketch (8080)
 */
final int PORT = 8080;  ///< Must match Arduino server port

// ===== APPLICATION STATE VARIABLES ==========================================
/**
 * @brief Connection status flag
 * @details Tracks whether the Processing sketch is successfully
 *          connected to the Arduino server.
 * @value true - Connected and ready to send commands
 * @value false - Disconnected or connection failed
 */
boolean connected = false;

/**
 * @brief Last command sent to Arduino
 * @details Stores the most recently sent command for display purposes.
 *          Helps users verify their inputs are being processed.
 */
String lastCommand = "None";

/**
 * @brief Current robot speed (0-255)
 * @details Tracks the robot's speed locally for display. This value
 *          is estimated based on commands sent and may not exactly
 *          match the Arduino's actual speed if packets are lost.
 */
int currentSpeed = 150;

/**
 * @brief Connection attempt counter
 * @details Tracks how many times connection has been attempted,
 *          used for controlling reconnection frequency.
 */
int connectionAttempts = 0;

// ===== UI CONSTANTS =========================================================
/**
 * @brief Application window width in pixels
 */
final int WINDOW_WIDTH = 600;

/**
 * @brief Application window height in pixels  
 */
final int WINDOW_HEIGHT = 400;

/**
 * @brief Background color (dark gray)
 */
final color BACKGROUND_COLOR = color(30, 30, 30);

/**
 * @brief Text color (white)
 */
final color TEXT_COLOR = color(255, 255, 255);

/**
 * @brief Connection status color (green when connected)
 */
final color CONNECTED_COLOR = color(0, 255, 0);

/**
 * @brief Disconnection status color (red when disconnected)
 */
final color DISCONNECTED_COLOR = color(255, 0, 0);

// ===== SETUP FUNCTION =======================================================
/**
 * @brief Initializes the Processing application
 * @details Called once when the program starts. Sets up the display
 *          window, configures text rendering, and attempts to connect
 *          to the Arduino.
 * 
 * @sideeffects Creates application window, establishes network connection
 * @return void
 */
void setup() {
  // Create application window
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  surface.setTitle("WiFi Robot Controller v1.0");
  
  // Configure text rendering
  textAlign(CENTER, CENTER);  // Center text both horizontally and vertically
  textSize(16);               // Base font size
  
  // Display initialization message
  println("==========================================");
  println("WiFi Robot Controller - Processing Client");
  println("==========================================");
  println("Target Arduino: " + arduinoIP + ":" + PORT);
  
  // Attempt initial connection to Arduino
  connectToArduino();
}

// ===== MAIN DRAW LOOP =======================================================
/**
 * @brief Continuously updates the application display
 * @details Called approximately 60 times per second (Processing's default
 *          frame rate). Updates the GUI, checks connection status, and
 *          processes any incoming messages from the Arduino.
 * 
 * @sideeffects Updates screen display, manages reconnection attempts
 * @return void
 */
void draw() {
  // Clear screen with dark background
  background(BACKGROUND_COLOR);
  
  // ===== HEADER SECTION =====
  drawHeader();
  
  // ===== CONNECTION STATUS SECTION =====
  drawConnectionStatus();
  
  // ===== CONTROL INSTRUCTIONS SECTION =====
  drawControlInstructions();
  
  // ===== STATUS DISPLAY SECTION =====
  drawStatusDisplay();
  
  // ===== ARDUINO COMMUNICATION HANDLING =====
  handleArduinoCommunication();
  
  // ===== AUTOMATIC RECONNECTION =====
  manageReconnection();
}

// ===== DRAWING HELPER FUNCTIONS =============================================

/**
 * @brief Draws the application header/title
 * @private
 * @return void
 */
void drawHeader() {
  fill(TEXT_COLOR);
  textSize(24);
  text("WiFi Robot Controller", width/2, 40);
  textSize(16);
}

/**
 * @brief Draws the connection status indicator
 * @private
 * @return void
 */
void drawConnectionStatus() {
  if (connected) {
    fill(CONNECTED_COLOR);
    text("✓ CONNECTED to Arduino at " + arduinoIP, width/2, 80);
  } else {
    fill(DISCONNECTED_COLOR);
    text("✗ DISCONNECTED - Attempting to reconnect...", width/2, 80);
  }
}

/**
 * @brief Draws the control instructions/legend
 * @private
 * @return void
 */
void drawControlInstructions() {
  fill(TEXT_COLOR);
  textSize(18);
  text("CONTROL MAPPING", width/2, 120);
  textSize(16);
  
  // Column 1: Movement controls
  text("MOVEMENT", width/4, 160);
  text("W = Forward", width/4, 190);
  text("S = Backward", width/4, 220);
  text("A = Left Turn", width/4, 250);
  text("D = Right Turn", width/4, 280);
  
  // Column 2: Speed controls
  text("SPEED CONTROL", 3*width/4, 160);
  text("J = Speed Up", 3*width/4, 190);
  text("K = Speed Down", 3*width/4, 220);
  text("SPACE = Emergency Stop", 3*width/4, 250);
  
  // Visual separator line
  stroke(TEXT_COLOR);
  strokeWeight(1);
  line(width/2, 140, width/2, 300);
  noStroke();
}

/**
 * @brief Draws the current status information
 * @private
 * @return void
 */
void drawStatusDisplay() {
  fill(TEXT_COLOR);
  textSize(18);
  text("CURRENT STATUS", width/2, 320);
  textSize(16);
  
  text("Last Command: " + lastCommand, width/2, 350);
  text("Speed: " + currentSpeed + " / 255", width/2, 380);
}

// ===== NETWORK COMMUNICATION FUNCTIONS ======================================

/**
 * @brief Handles incoming messages from Arduino
 * @details Checks for available data from the Arduino and processes
 *          any received messages. Currently logs messages to console.
 * @private
 * @return void
 */
void handleArduinoCommunication() {
  if (client != null && client.available() > 0) {
    String message = client.readStringUntil('\n');
    if (message != null) {
      message = message.trim();
      println("[Arduino] " + message);
      
      // Parse acknowledgment messages
      if (message.startsWith("ACK:")) {
        String cmd = message.substring(4);
        println("Command acknowledged: " + cmd);
      }
      
      // Parse speed updates
      if (message.startsWith("Speed:")) {
        try {
          int reportedSpeed = Integer.parseInt(message.substring(6));
          println("Arduino reports speed: " + reportedSpeed);
        } catch (NumberFormatException e) {
          println("Error parsing speed from Arduino");
        }
      }
    }
  }
}

/**
 * @brief Manages automatic reconnection attempts
 * @details Attempts to reconnect to Arduino every 5 seconds if disconnected
 * @private
 * @return void
 */
void manageReconnection() {
  if (!connected && frameCount % 300 == 0) {  // Every 5 seconds (60fps * 5)
    println("Attempting reconnection...");
    connectToArduino();
  }
}

// ===== KEYBOARD INPUT HANDLER ===============================================
/**
 * @brief Processes keyboard input and sends commands to Arduino
 * @details Called automatically by Processing when any key is pressed.
 *          Filters for valid robot commands and sends them via TCP.
 *          Also updates local speed display for immediate feedback.
 * 
 * @param key The character of the key that was pressed
 * @return void
 */
void keyPressed() {
  // Ignore input if not connected
  if (!connected) {
    println("WARNING: Not connected to Arduino! Command ignored.");
    return;
  }
  
  // Convert key to lowercase for case-insensitive handling
  char lowerKey = Character.toLowerCase(key);
  
  // Check if key is a valid robot command
  if (lowerKey == 'w' || lowerKey == 'a' || lowerKey == 's' || lowerKey == 'd' ||
      lowerKey == 'j' || lowerKey == 'k' || key == ' ') {
    
    // Send command to Arduino
    client.write(lowerKey);
    lastCommand = formatCommandDisplay(lowerKey);
    
    // Log the command
    println("[Client] Sent command: '" + lowerKey + "'");
    
    // Update local speed estimate (for display only)
    updateLocalSpeedEstimate(lowerKey);
    
  } else {
    // Invalid key pressed
    println("Invalid key: '" + key + "'. Valid: w,a,s,d,j,k,space");
  }
}

/**
 * @brief Formats command character for display
 * @details Converts special characters (like space) to readable format
 * @param cmd The command character to format
 * @return Formatted string for display
 * @private
 */
String formatCommandDisplay(char cmd) {
  if (cmd == ' ') {
    return "[SPACE]";
  } else {
    return "'" + cmd + "'";
  }
}

/**
 * @brief Updates local speed estimate based on command
 * @details Maintains a local estimate of robot speed for display purposes.
 *          This is an estimate only - actual speed is controlled by Arduino.
 * @param cmd The command that was sent
 * @private
 * @return void
 */
void updateLocalSpeedEstimate(char cmd) {
  if (cmd == 'j') {
    currentSpeed = min(currentSpeed + 25, 255);
  } else if (cmd == 'k') {
    currentSpeed = max(currentSpeed - 25, 0);
  }
  // Movement commands don't change speed estimate
}

// ===== CONNECTION MANAGEMENT ================================================
/**
 * @brief Establishes TCP connection to Arduino WiFi server
 * @details Attempts to connect to the Arduino at the configured
 *          IP address and port. Updates connection status and
 *          provides feedback to the user.
 * 
 * @sideeffects Updates 'connected' flag, creates Client object
 * @return void
 */
void connectToArduino() {
  connectionAttempts++;
  println("Connection attempt #" + connectionAttempts);
  println("Target: " + arduinoIP + ":" + PORT);
  
  try {
    // Attempt to create TCP client connection
    client = new Client(this, arduinoIP, PORT);
    
    // Check if connection was successful
    if (client.active()) {
      connected = true;
      println("✓ Successfully connected to Arduino!");
      println("Ready to send commands.");
    } else {
      connected = false;
      println("✗ Connection failed: Client not active");
    }
  } catch (Exception e) {
    // Connection failed
    connected = false;
    println("✗ Connection failed: " + e.getMessage());
    println("Check:");
    println("  1. Arduino IP address (" + arduinoIP + ")");
    println("  2. Arduino is powered and connected to WiFi");
    println("  3. Both devices on same network");
    println("  4. No firewall blocking port " + PORT);
  }
}

// ===== APPLICATION CLEANUP ==================================================
/**
 * @brief Performs cleanup when application closes
 * @details Called automatically by Processing when the sketch window
 *          is closed. Ensures proper disconnection from Arduino.
 * 
 * @sideeffects Closes network connection, stops motors
 * @return void
 */
void dispose() {
  println("\n==========================================");
  println("Application shutting down...");
  
  // Send stop command to ensure robot stops
  if (connected && client != null) {
    println("Sending final stop command to Arduino");
    client.write(' ');
    delay(100);  // Brief delay to ensure transmission
  }
  
  // Close network connection
  if (client != null) {
    client.stop();
    println("Network connection closed");
  }
  
  println("WiFi Robot Controller stopped");
  println("==========================================");
}

/**
 * @brief Manual connection test function
 * @details Can be called to test connection without waiting for
 *          automatic reconnection. Useful for debugging.
 * @private
 * @return void
 */
void manualReconnect() {
  println("Manual reconnection requested...");
  connectToArduino();
}
