#include <Arduino.h>
#include <ESP32Servo.h>

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
//right stepper driver
#define xStepPin 27
#define xDirPin 26

// Stepper pins for Y-axis (2 motors)
//middle stepper driver
#define yStepPin1 25
#define yDirPin1 33
//leftmost stepper driver
#define yStepPin2 12
#define yDirPin2 14

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
#define x 0x0
#define y 0x1

// motor = 1 for x and 2 for y
void moveStepper(uint8_t motor, int steps, int stepDelay = 2000)
{
  int thisDir;
  int thisStep;
  uint8_t direction;

  if (steps < 0)
  {
    direction = LOW;
    steps = steps * -1;
  }
  else
  {
    direction = HIGH;
  }
  if (motor == x)
  {
    digitalWrite(xDirPin, direction); // set direction
    for (int i = 0; i <= steps; i++)
    {

      digitalWrite(xStepPin, HIGH);
      delayMicroseconds(stepDelay); // Wait for the step delay
      digitalWrite(xStepPin, LOW);
      delayMicroseconds(stepDelay); // Wait for the next step
    }
  }
  if (motor == y)
  {
    digitalWrite(yDirPin1, direction); // set direction
    digitalWrite(yDirPin2, !direction); // set direction

    for (int i = 0; i <= steps; i++)
    {
      digitalWrite(yStepPin1, HIGH);
      digitalWrite(yStepPin2, HIGH);
      delayMicroseconds(stepDelay); // Wait for the step delay
      digitalWrite(yStepPin1, LOW);
      digitalWrite(yStepPin2, LOW);
      delayMicroseconds(stepDelay); // Wait for the next step
    }
  }
}
void setup()
{
  Serial.begin(115200);
  rackServo.attach(rackServoPin);
  screwServo.attach(screwServoPin);

  rackServo.write(rackTop);
  screw(screwOff);
  delay(100);

  pinMode(xStepPin, OUTPUT);
  pinMode(xDirPin, OUTPUT);
  digitalWrite(xDirPin, HIGH);

  pinMode(yStepPin1, OUTPUT);
  pinMode(yDirPin1, OUTPUT);
  digitalWrite(yDirPin1, HIGH);

  pinMode(yStepPin2, OUTPUT);
  pinMode(yDirPin2, OUTPUT);
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

  Serial.println("left 200 steps");
  moveStepper(x, 200);

  delay(1000);

  Serial.println("right 200 steps");
  moveStepper(x, -200);

  delay(1000);

  Serial.println("backwards 200 steps");
  moveStepper(y, 200);

  delay(1000);

  Serial.println("forewards 200 steps");
  moveStepper(y, -200);

  delay(1000);
}
