#include <Arduino.h>
#include <ESP32Servo.h>
#include <AccelStepper.h>

// Servo pins
#define servo1Pin 18
#define servo2Pin 19
Servo servo1;
Servo servo2;

// Stepper pins for X-axis
#define xStepPin 12
#define xDirPin 14

// Stepper pins for Y-axis (2 motors)
#define yStepPin1 25
#define yDirPin1 26
#define yStepPin2 27
#define yDirPin2 33

// Constants for acceleration and speed
#define maxSpeed 1000 // Max speed (steps per second)
#define accel 500     // Acceleration (steps per second^2)

// Create stepper objects
AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper1(AccelStepper::DRIVER, yStepPin1, yDirPin1);
AccelStepper yStepper2(AccelStepper::DRIVER, yStepPin2, yDirPin2);

void setup() {
  Serial.begin(115200);

  // Attach servos
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  // Initialize stepper motor pins
  xStepper.setMaxSpeed(maxSpeed);
  xStepper.setAcceleration(accel);

  yStepper1.setMaxSpeed(maxSpeed);
  yStepper1.setAcceleration(accel);

  yStepper2.setMaxSpeed(maxSpeed);
  yStepper2.setAcceleration(accel);

  // Set stepper directions (initial direction)
  digitalWrite(xDirPin, HIGH);
  digitalWrite(yDirPin1, HIGH);
  digitalWrite(yDirPin2, HIGH);
}

void loop() {
  // Servo movement example
  for (int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    servo1.write(posDegrees);
    servo2.write(posDegrees);
    delay(20);
  }

  for (int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    servo1.write(posDegrees);
    servo2.write(posDegrees);
    delay(20);
  }

  // Stepper synchronized motion example
  xStepper.moveTo(400); // Move X-axis 400 steps
  yStepper1.moveTo(400); // Move Y-axis motor 1 400 steps
  yStepper2.moveTo(400); // Move Y-axis motor 2 400 steps

  while (xStepper.distanceToGo() > 0 || yStepper1.distanceToGo() > 0 || yStepper2.distanceToGo() > 0) {
    xStepper.run();
    yStepper1.run();
    yStepper2.run();
  }

  delay(1000); // Pause after movement

  // Reverse direction and move back
  xStepper.moveTo(-400);
  yStepper1.moveTo(-400);
  yStepper2.moveTo(-400);

  while (xStepper.distanceToGo() > 0 || yStepper1.distanceToGo() > 0 || yStepper2.distanceToGo() > 0) {
    xStepper.run();
    yStepper1.run();
    yStepper2.run();
  }

  delay(1000); // Pause after movement
}