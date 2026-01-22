// MyRobot-V1.ino
// This file must be named the same as your sketch folder
int enA = 9;   // Enable pin for Motor A — must be a PWM-capable pin (left wheel)
int in1 = 8;   // Direction control pin 1 for Motor A
int in2 = 7;   // Direction control pin 2 for Motor A

int enB = 5;    // Enable pin for Motor B — must be a PWM-capable pin (right wheel)
int in3 = 2;   // Direction control pin 1 for Motor B
int in4 = 4;   // Direction control pin 2 for Motor B 

int currentSpeed = 150;  // Default medium speed (0-255)
const int SPEED_INCREMENT = 25;
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

