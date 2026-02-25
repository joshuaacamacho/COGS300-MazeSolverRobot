/**
 * Wired Robot Controller (USB Serial)
 * Processing → Arduino via COM port
 */

import processing.serial.*;

Serial myPort;

// ===== SETTINGS =====
final int BAUD_RATE = 9600;   
final int PORT_INDEX = 0;     

// ===== STATE =====
String lastCommand = "None";
int currentSpeed = 150;

// ===== GUI SIZE =====
final int WIDTH = 600;
final int HEIGHT = 400;

void settings() {
  size(WIDTH, HEIGHT);
}

void setup() {
  surface.setTitle("Wired Robot Controller (USB)");
  textAlign(CENTER, CENTER);
  textSize(16);

  println("Available Ports:");
  printArray(Serial.list());

  myPort = new Serial(this, Serial.list()[PORT_INDEX], BAUD_RATE);
}

void draw() {
  background(30);
  
  drawHeader();
  drawControls();
  drawStatus();
}

// ===== DRAW FUNCTIONS =====
void drawHeader() {
  fill(255);
  textSize(24);
  text("USB Robot Controller", width/2, 40);
  textSize(16);
}

void drawControls() {
  fill(255);
  textSize(18);
  text("CONTROL MAPPING", width/2, 100);
  textSize(16);
  
  text("W = Forward", width/4, 150);
  text("S = Backward", width/4, 180);
  text("A = Left", width/4, 210);
  text("D = Right", width/4, 240);
  text("F = Ultrasonic Auto", width/4, 270);
  text("L = Line Follow (IR)", width/4, 300);

  text("SPACE = Stop", 3*width/4, 150);
  text("J = Speed Up", 3*width/4, 180);
  text("K = Speed Down", 3*width/4, 210);

  stroke(255);
  line(width/2, 120, width/2, 320);
  noStroke();
}

void drawStatus() {
  fill(255);
  textSize(18);
  text("CURRENT STATUS", width/2, 340);
  textSize(16);
  text("Last Command: " + lastCommand, width/2, 370);
  text("Speed: " + currentSpeed + " / 255", width/2, 395);
}

// ===== KEY INPUT =====
void keyPressed() {

  char c = Character.toLowerCase(key);
  
  boolean valid = (c == 'w' || c == 'a' || c == 's' || c == 'd' ||
                   c == 'j' || c == 'k' || c == 'f' || c == 'l' ||
                   key == ' ');
  
  if (valid) {
    myPort.write(c);
    lastCommand = formatCmd(c);
    println("[Sent] " + lastCommand);

    if (c == 'j') currentSpeed = min(currentSpeed + 25, 255);
    if (c == 'k') currentSpeed = max(currentSpeed - 25, 0);
  }
}

String formatCmd(char c) {
  return (c == ' ') ? "[SPACE]" : "'" + c + "'";
}

void dispose() {
  println("Closing controller...");
  if (myPort != null) {
    myPort.write(' ');
    delay(50);
    myPort.stop();
  }
  println("Controller closed.");
}