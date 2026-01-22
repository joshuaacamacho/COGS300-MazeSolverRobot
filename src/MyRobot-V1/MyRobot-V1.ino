// MyRobot-V1.ino
//final working version
// This file must be named the same as your sketch folder
int enA = 9;   // Enable pin for Motor A — must be a PWM-capable pin (left wheel)
int in1 = 8;   // Direction control pin 1 for Motor A
int in2 = 7;   // Direction control pin 2 for Motor A

int enB = 5;    // Enable pin for Motor B — must be a PWM-capable pin (right wheel)
int in3 = 2;   // Direction control pin 1 for Motor B
int in4 = 4;   // Direction control pin 2 for Motor B 

int currentSpeed = 150;  // Default medium speed (0-255)
const int SPEED_INCREMENT = 25;// MyRobot-V1.ino
// This file must be named the same as your sketch folder

// ===== PIN DEFINITIONS =====
int enA = 9;   // Enable pin for Motor A (PWM) - left wheel
int in1 = 8;   // Direction pin 1 for Motor spA
int in2 = 7;   // Direction pin 2 for Motor A

int enB = 5;   // Enable pin for Motor B (PWM) - right wheel
int in3 = 2;   // Direction pin 1 for Motor B
int in4 = 4;   // Direction pin 2 for Motor B

// ===== SPEED SETTINGS =====
int currentSpeed = 150;        // 0–255
const int SPEED_INCREMENT = 25;
const int MAX_SPEED = 255;

// ===== SETUP =====
void setup() {
  Serial.begin(9600);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  Serial.println("Robot ready");
}

// ===== LOOP =====
void loop() {
  if (Serial.available()) {
    char command = Serial.read();

    switch (command) {
      case 'w': drive();     break;
      case 's': backwards();break;
      case 'a': turnLeft(); break;
      case 'd': turnRight();break;
      case 'j': speedUp();  break;
      case 'k': slowDown(); break;
      case ' ': stopMotors(); break;
    }
  }
}

// ===== MOTOR FUNCTIONS =====

void drive() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);

  Serial.println("Forward");
}

void backwards() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);

  Serial.println("Backward");
}

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  analogWrite(enA, 0);
  analogWrite(enB, 0);

  Serial.println("Stop");
}

void turnLeft() {
  analogWrite(enA, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, currentSpeed);

  Serial.println("Left");
}

void turnRight() {
  analogWrite(enB, 0);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, currentSpeed);

  Serial.println("Right");
}

void speedUp() {
  currentSpeed = min(currentSpeed + SPEED_INCREMENT, MAX_SPEED);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);

  Serial.print("Speed: ");
  Serial.println(currentSpeed);
}

void slowDown() {
  currentSpeed = max(currentSpeed - SPEED_INCREMENT, 0);
  analogWrite(enA, currentSpeed);
  analogWrite(enB, currentSpeed);

  Serial.print("Speed: ");
  Serial.println(currentSpeed);
}

const int MAX_SPEED = 255;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);

    pinMode(enA, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);

    pinMode(enB, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        
        switch(command) {
            case 'w':  // Forward
                drive();
                break;
            case 's':  // Backward
                backwards();
                break;
            case 'a':  // Turn left
                turnLeft();
                break;
            case 'd':  // Turn right
                turnRight();
                break;
            case 'j':  // Speed up
                speedUp();
                break;
            case 'k':  // Slow down
                slowDown();
                break;
            case ' ':  // Stop
                stop();
                break;
        }
    }
}

