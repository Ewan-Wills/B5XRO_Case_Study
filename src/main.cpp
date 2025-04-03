#include <Arduino.h>
#include <ESP32Servo.h>

#define rackServoPin 19
#define screwServoPin 18
Servo rackServo;
Servo screwServo;

// values for end effector
#define rackBottom 140
#define rackMiddle 90
#define rackTop 0

#define screwOff 90
#define screwIn 180
#define screwOut 0
// speed at which the rack will go up/down. Lower means faster
#define rackSpeedIn 85
#define rackSpeedOut 150
// Stepper pins for X-axis
// right stepper driver
#define xStepPin 27
#define xDirPin 26

// Stepper pins for Y-axis (2 motors)
// middle stepper driver
#define yStepPin1 25
#define yDirPin1 33
// leftmost stepper driver
#define yStepPin2 12
#define yDirPin2 14

// defualt delay between steps in microseconds
#define startSpeed 6000
#define acceleration 25 // how much the speed will be incremented in the for loop
#define maxSpeed 1500

#define xMax 230
#define yMax 150
#define x 0x0
#define y 0x1

// let 0,0 be bottom left
// initial position is centre marked by pen on the axis's
int xPosition = 90;
int yPosition = 70;

void screw(int value)
{
  screwServo.write(value);
}
void rack(int speed, int from, int to)
{
  if (speed == -1)
  {
    rackServo.write(to);
  }
  else
  {
    if (from < to)
    {
      for (int i = from; i <= to; i++)
      {
        rackServo.write(i);
        delay(speed);
      }
    }
    if (from > to)
    {
      for (int i = from; i >= to; i--)
      {
        rackServo.write(i);
        delay(speed);
      }
    }
  }
  delay(100);
}

void pickUp()
{
  screw(screwOut);
  rack(-1, rackTop, rackBottom); // go to bottom instantly
  delay(1000);
  rack(rackSpeedOut, rackBottom, rackMiddle); // halfway to bottom slowly
  delay(100);
  screw(screwOff);
  rack(-1, rackMiddle, rackTop); // go up rest of wayh instantly
}

void putDown()
{
  screw(screwIn);
  rack(-1, rackTop, rackMiddle);             // bottom to halfway instantly
  rack(rackSpeedIn, rackMiddle, rackBottom); // halfway to bottom slowly
  // delay(250);                      // screw a little longer
  screw(screwOff);
  rack(-1, rackBottom, rackTop); // bottom to top instantly
  delay(100);
  rack(-1, rackBottom, rackTop); // bottom to top instantly // try again because sometimes the magnet too strong
}

// use start speed, max speed and acceleration constants to get the delay for a step (in microseconds)
int getDelay(int currentStep, int steps, int currentDelay)
{
  // if three quatres of journey decrease delay by the acceleration
  // 0.75 is the ratio of accelerating to deccelerating (ie we want to accelerate for three quatres then deccelerate for one quatre)
  if ((double)currentStep < ((double)steps * 0.75))
  {
    if (currentDelay > maxSpeed)
    {
      currentDelay = currentDelay - acceleration;
    }
  }
  else
  { // on the second half the journey increase delay between steps by the acceleratio
    if (currentDelay < startSpeed)
    {
      currentDelay = currentDelay + acceleration;
    }
  }
  // Serial.print("i: ");
  // Serial.print(currentStep);
  // Serial.print(" steps: ");
  // Serial.print(steps);
  // Serial.print(" delay: ");
  // Serial.println(currentDelay);

  return currentDelay;
}
// motor = 1 for x and 2 for y
void moveSteps(uint8_t motor, int steps)
{
  int thisDir;
  int thisStep;
  uint8_t direction;
  int stepDelay;
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
    digitalWrite(xDirPin, !direction); // set direction
    stepDelay = startSpeed;
    for (int i = 0; i <= steps; i++)
    {
      stepDelay = getDelay(i, steps, stepDelay);

      digitalWrite(xStepPin, HIGH);
      delayMicroseconds(stepDelay); // Wait for the step delay
      digitalWrite(xStepPin, LOW);
      delayMicroseconds(stepDelay); // Wait for the next step
    }
  }
  if (motor == y)
  {
    digitalWrite(yDirPin1, direction);  // set direction
    digitalWrite(yDirPin2, !direction); // set direction
    stepDelay = startSpeed;
    for (int i = 0; i <= steps; i++)
    {
      stepDelay = getDelay(i, steps, stepDelay);
      digitalWrite(yStepPin1, HIGH);
      digitalWrite(yStepPin2, HIGH);
      delayMicroseconds(stepDelay); // Wait for the step delay
      digitalWrite(yStepPin1, LOW);
      digitalWrite(yStepPin2, LOW);
      delayMicroseconds(stepDelay); // Wait for the next step
    }
  }
}

void moveMillimeters(uint8_t motor, int mm, int stepDelay = 2000)
{

  // 500 steps on x axis moved the end effector 93mm, 500 on y axis moved the end effector 96mm (at 2000 speed)
  int steps = mm * 5.2631;
  moveSteps(motor, steps);
}

void gotoXY(int xDestination, int yDestination)
{
  Serial.print("Current x: ");
  Serial.print(xPosition);
  Serial.print("  New x:");
  Serial.print(xDestination);
  Serial.print("  Current y:");
  Serial.print(yPosition);
  Serial.print("  New y:");
  Serial.println(yDestination);

  if ((xDestination >= 0) && (xDestination <= xMax))
  {
    // the x destination is to the left of the current position
    if (xDestination < xPosition)
    {
      moveMillimeters(x, -1 * (xPosition - xDestination));
    }

    // the x destination is to the right of the current position
    if (xDestination > xPosition)
    {
      moveMillimeters(x, xDestination - xPosition);
    }
    xPosition = xDestination;
  }
  if ((yDestination >= 0) && (yDestination <= yMax))
  {
    // the y destination is below the current position
    if (yDestination < yPosition)
    {
      moveMillimeters(y, -1 * (yPosition - yDestination));
    }
    // the y destination is above the current position
    if (yDestination > yPosition)
    {
      moveMillimeters(y, yDestination - yPosition);
    }
    yPosition = yDestination;
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
  int loopDelay = 50;
  // load 1
  Serial.println("load 1");
  gotoXY(182, 43);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);

  // hole 1
  Serial.println("hole 1");
  gotoXY(42, 44);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // //load 6
  Serial.println("load 6");
  gotoXY(183, 98);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);

  // //hole 6
  Serial.println("hole 6");
  gotoXY(41, 99);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // load 2
  Serial.println("load 2");
  gotoXY(183, 54);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // hole 2
  Serial.println("hole 2");
  gotoXY(88, 16);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // load 5
  Serial.println("load 5");
  gotoXY(183, 87);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // hole 5
  Serial.println("hole 5");
  gotoXY(88, 127);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // load 3
  Serial.println("load 3");
  gotoXY(183, 65);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // hole 3
  Serial.println("hole 3");
  gotoXY(135, 43);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // load 4
  Serial.println("load 4");
  gotoXY(182, 76);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // hole 4
  Serial.println("hole 4");
  gotoXY(136, 99);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // load 3
  Serial.println("pickup @ hole 3");
  gotoXY(135, 43);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // hole 3
  Serial.println("putdown @ load 3");
  gotoXY(183, 65);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // load 4
  Serial.println("pickup at hole 4");
  gotoXY(136, 99);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // hole 4
  Serial.println("place at load 4");
  gotoXY(182, 76);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // load 2
  Serial.println("pickup @ hole 2");
  gotoXY(88, 16);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // hole 2
  Serial.println("putdown @ load 2");
  gotoXY(183, 54);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // hole 5
  Serial.println(" pickup at hole 5");
  gotoXY(88, 127);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // load 5
  Serial.println("place at load 5");
  gotoXY(183, 87);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  Serial.println("pickup from hole 1");
  gotoXY(42, 44);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);

  // load 1
  Serial.println("put down in load 1");
  gotoXY(182, 43);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  // //load 6
  Serial.println("pickup at hole 6");
  gotoXY(41, 99);
  delay(loopDelay);
  pickUp();
  delay(loopDelay);
  // //hole 6
  Serial.println("putdown at load 6");
  gotoXY(183, 98);
  delay(loopDelay);
  putDown();
  delay(loopDelay);

  delay(5000);
}
