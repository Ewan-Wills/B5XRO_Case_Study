#include <Arduino.h>

#include <Stepper.h>

// change this to the number of steps on your motor
#define STEPS 200

//define all stepper pins
#define xmotor1Pin1 12
#define xmotor1Pin2 14

// #define xmotor2Pin1 32
// #define xmotor2Pin2 33

// #define ymotorPin1 25
// #define ymotorPin2 26

// #include <AccelStepper.h>

// // Define some steppers and the pins the will use
// //AccelStepper stepper1; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

// AccelStepper stepper1(AccelStepper::FULL2WIRE, xmotor1Pin1, xmotor1Pin2);

// void setup()
// {  
//   Serial.begin(115200);
//     stepper1.setMaxSpeed(1000.0);
//     stepper1.setAcceleration(1000.0);
//     stepper1.moveTo(240);
    
   
// }

// void loop()
// {
//   Serial.println("hello");
//     // Change direction at the limits
//     if (stepper1.distanceToGo() == 0){
// 	    stepper1.moveTo(-stepper1.currentPosition());
//     }
//     stepper1.run();

// }

#define STEP_PIN 12
#define DIR_PIN 14

int stepDelay = 2000;  // Initial delay between steps (microseconds)
int minStepDelay = 500;  // Minimum delay (maximum speed)
int acceleration = 10;  // Acceleration factor (lower value = faster acceleration)
int totalSteps = 200;  // Total steps (1 full rotation for 1.8° step motor)
int stepCount = 0;  // Track the number of steps taken

void setup() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(DIR_PIN, HIGH);  // Set motor direction
}

void loop() {
    // Gradually accelerate
    while (stepCount < totalSteps) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(stepDelay);  // Wait for the step delay
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(stepDelay);  // Wait for the next step
        
        // Increase speed by reducing the delay
        if (stepDelay > minStepDelay) {
            stepDelay -= acceleration;  // Increase speed (decrease delay)
        }
        
        stepCount++;  // Increment step counter
    }
    
    delay(1000);  // Wait after completing a full rotation
    
    // Reverse direction for the next loop
    digitalWrite(DIR_PIN, !digitalRead(DIR_PIN));  // Reverse direction
    stepCount = 0;  // Reset step count
    stepDelay = 2000;  // Reset the step delay to initial value
}