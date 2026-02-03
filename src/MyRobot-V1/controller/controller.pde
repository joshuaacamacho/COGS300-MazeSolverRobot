/**
 * @file WiFiRobotControllerV2.pde
 * @brief Stable WiFi robot controller for Arduino
 * @details This Processing sketch connects to an Arduino WiFi server
 *          and sends single-character motor commands with non-blocking
 *          TCP handling. Includes GUI for control feedback.
 *
 * Controls:
 *   W = Forward
 *   S = Backward
 *   A = Left
 *   D = Right
 *   J = Speed Up
 *   K = Speed Down
 *   Space = Stop
 *
 * Author: Joshua Camacho
 * Version: 2.0.0
 * Date: 2026
 */

import processing.net.*;

// ===== NETWORK SETTINGS =====
String arduinoIP = "192.168.137.3"; // CHANGE to your Arduino IP
final int PORT = 8080;
Client client;
boolean connected = false;
int reconnectTimer = 0;  // frames until next reconnect attempt

// ===== APPLICATION STATE =====
String lastCommand = "None";
int currentSpeed = 150;

// ===== GUI SETTINGS =====
final int WIDTH = 600;
final int HEIGHT = 400;

void settings() {
  size(WIDTH, HEIGHT);
}

void setup() {
  surface.setTitle("WiFi Robot Controller v2.0");
  textAlign(CENTER, CENTER);
  textSize(16);
  connectToArduino();
}

void draw() {
  background(30);
  
  drawHeader();
  drawConnectionStatus();
  drawControls();
  drawStatus();
  
  handleArduinoMessages();
  
  // attempt reconnection every 5 seconds
  if (!connected && frameCount - reconnectTimer > 300) {
    reconnectTimer = frameCount;
    connectToArduino();
  }
}

// ===== DRAW HELPERS =====
void drawHeader() {
  fill(255);
  textSize(24);
  text("WiFi Robot Controller v2.0", width/2, 40);
  textSize(16);
}

void drawConnectionStatus() {
  if (connected) {
    fill(0, 255, 0);
    text("✓ CONNECTED to Arduino at " + arduinoIP, width/2, 80);
  } else {
    fill(255, 0, 0);
    text("✗ DISCONNECTED - attempting reconnect...", width/2, 80);
  }
}

void drawControls() {
  fill(255);
  textSize(18);
  text("CONTROL MAPPING", width/2, 120);
  textSize(16);
  
  // Left column
  text("MOVEMENT", width/4, 160);
  text("W = Forward", width/4, 190);
  text("S = Backward", width/4, 220);
  text("A = Left Turn", width/4, 250);
  text("D = Right Turn", width/4, 280);
  
  // Right column
  text("SPEED", 3*width/4, 160);
  text("J = Speed Up", 3*width/4, 190);
  text("K = Speed Down", 3*width/4, 220);
  text("SPACE = Stop", 3*width/4, 250);
  
  stroke(255);
  line(width/2, 140, width/2, 300);
  noStroke();
}

void drawStatus() {
  fill(255);
  textSize(18);
  text("CURRENT STATUS", width/2, 320);
  textSize(16);
  text("Last Command: " + lastCommand, width/2, 350);
  text("Speed: " + currentSpeed + " / 255", width/2, 380);
}

// ===== NETWORK FUNCTIONS =====
void connectToArduino() {
  println("Attempting connection to Arduino...");
  client = new Client(this, arduinoIP, PORT);
  
  if (client.active()) {
    connected = true;
    println("✓ Successfully connected!");
  } else {
    connected = false;
    println("✗ Connection failed. Check IP, network, Arduino power.");
  }
}

void handleArduinoMessages() {
  if (client != null && connected) {
    while (client.available() > 0) {
      String msg = client.readStringUntil('\n');
      if (msg != null) {
        msg = msg.trim();
        println("[Arduino] " + msg);
        
        // optional: parse ACK or speed
        if (msg.startsWith("OK:")) {
          String cmd = msg.substring(3);
          println("Command acknowledged: " + cmd);
        }
        if (msg.startsWith("Speed:")) {
          try {
            currentSpeed = Integer.parseInt(msg.substring(6));
          } catch (Exception e) {
            println("Error parsing speed from Arduino");
          }
        }
      }
    }
    
    if (!client.active()) {
      println("Connection lost.");
      connected = false;
    }
  }
}

// ===== INPUT HANDLING =====
void keyPressed() {
  if (!connected) {
    println("Not connected: command ignored");
    return;
  }
  
  char c = Character.toLowerCase(key);
  boolean valid = (c == 'w' || c == 'a' || c == 's' || c == 'd' ||
                   c == 'j' || c == 'k' || key == ' ');
  
  if (valid) {
    client.write(c);  // send single char
    lastCommand = formatCmd(c);
    println("[Client] Sent command: " + lastCommand);
    
    // update local speed display
    if (c == 'j') currentSpeed = min(currentSpeed + 25, 255);
    if (c == 'k') currentSpeed = max(currentSpeed - 25, 0);
  }
}

String formatCmd(char c) {
  return (c == ' ') ? "[SPACE]" : "'" + c + "'";
}

// ===== CLEANUP =====
void dispose() {
  println("Closing controller...");
  if (client != null && connected) {
    client.write(' '); // stop robot
    delay(50);
    client.stop();
  }
  println("Controller stopped.");
}
