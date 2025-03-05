#include <Arduino.h>

#include <Stepper.h>

// change this to the number of steps on your motor
#define STEPS 200

//define all stepper pins
#define xmotor1Pin1 34
#define xmotor1Pin2 35

#define xmotor2Pin1 32
#define xmotor2Pin2 33

#define ymotorPin1 25
#define ymotorPin2 26

//initialise steppers
Stepper xStepper1(STEPS, xmotor1Pin1, xmotor1Pin2 );
Stepper xStepper2(STEPS, xmotor2Pin1, xmotor2Pin2 );

// the previous reading from the analog input


void setup() {
  // set the speed of the motor to 30 RPMs
  xStepper1.setSpeed(30);
}

void loop() {

  //step 200 steps
  xStepper1.step(200);
  delay(500);
  xStepper1.step(-200);
  delay(500);

}