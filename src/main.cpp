#include <Arduino.h>
#include <ESP32Servo.h>
#include <AccelStepper.h>

#define rackServoPin 19
#define screwServoPin 18
Servo rackServo;
Servo screwServo;

// values for end effector
#define rackBottom 180
#define rackTop 0

#define screwOff 90
#define screwIn 180
#define screwOut 0
// speed at which the rack will go up/down. Lower means faster
#define rackSpeed 100

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

void screw(int value)
{
  screwServo.write(value);
}
void rackDown(int speed)
{
  if (speed == -1)
  {
    rackServo.write(rackBottom);
  }
  else
  {
    for (int i = rackTop; i <= rackBottom; i++)
    {
      rackServo.write(i);
      delay(speed);
    }
  }
}
void rackUp(int speed)
{
  if (speed == -1)
  {
    rackServo.write(rackTop);
  }
  else
  {
    for (int i = rackBottom; i >= rackTop; i--)
    {
      rackServo.write(i);
      delay(speed);
    }
  }
}
void pickUp()
{
  rackDown(-1);
  delay(100);
  screw(screwOut);
  delay(500);
  rackUp(rackSpeed);
  screw(screwOff);
}

void putDown()
{
  rackUp(-1);
  delay(100);
  screw(screwIn);
  rackDown(rackSpeed);
  delay(100);
  screw(screwOff);
  rackUp(-1);
}
void setup()
{
  Serial.begin(115200);
  rackServo.attach(rackServoPin);
  screwServo.attach(screwServoPin);

  rackServo.write(rackTop);
  screw(screwOff);
  delay(100);

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
void loop()
{
  // Serial.println("Picking Up");
  // pickUp();

  // Serial.println("Waiting");
  // delay(5000);

  // Serial.println("Placing bolt");
  // putDown();

  // Serial.println("Waiting");
  // delay(5000);

  // Stepper synchronized motion example
  xStepper.moveTo(400);  // Move X-axis 400 steps
  yStepper1.moveTo(400); // Move Y-axis motor 1 400 steps
  yStepper2.moveTo(400); // Move Y-axis motor 2 400 steps

  while (xStepper.distanceToGo() > 0 || yStepper1.distanceToGo() > 0 || yStepper2.distanceToGo() > 0)
  {
    xStepper.run();
    yStepper1.run();
    yStepper2.run();
  }

  delay(1000); // Pause after movement

  // Reverse direction and move back
  xStepper.moveTo(-400);
  yStepper1.moveTo(-400);
  yStepper2.moveTo(-400);

  while (xStepper.distanceToGo() > 0 || yStepper1.distanceToGo() > 0 || yStepper2.distanceToGo() > 0)
  {
    xStepper.run();
    yStepper1.run();
    yStepper2.run();
  }

  delay(1000); // Pause after movement
}
