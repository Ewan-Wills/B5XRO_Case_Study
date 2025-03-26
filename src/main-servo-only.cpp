#include <Arduino.h>
#include <Stepper.h>
#include <ESP32Servo.h>
// change this to the number of steps on your motor
#define STEPS 200

//define all stepper pins
#define xmotor1Pin1 12
#define xmotor1Pin2 14

#define servo1Pin 18
#define servo2Pin 19
Servo servo1;
Servo servo2;



int stepDelay = 2000;  // Initial delay between steps (microseconds)
int minStepDelay = 500;  // Minimum delay (maximum speed)
int acceleration = 10;  // Acceleration factor (lower value = faster acceleration)
int totalSteps = 200;  // Total steps (1 full rotation for 1.8° step motor)
int stepCount = 0;  // Track the number of steps taken

void setup() {
    Serial.begin(115200);
    servo1.attach(servo1Pin);
    servo2.attach(servo2Pin);

    pinMode(xmotor1Pin1, OUTPUT);
    pinMode(xmotor1Pin2, OUTPUT);
    digitalWrite(xmotor1Pin2, HIGH);  // Set motor direction
}

void loop() {
    // Gradually accelerate
    // while (stepCount < totalSteps) {
    //     digitalWrite(STEP_PIN, HIGH);
    //     delayMicroseconds(stepDelay);  // Wait for the step delay
    //     digitalWrite(STEP_PIN, LOW);
    //     delayMicroseconds(stepDelay);  // Wait for the next step
        
    //     // Increase speed by reducing the delay
    //     if (stepDelay > minStepDelay) {
    //         stepDelay -= acceleration;  // Increase speed (decrease delay)
    //     }
        
    //     stepCount++;  // Increment step counter
    // }
    
    // delay(1000);  // Wait after completing a full rotation
    
    // // Reverse direction for the next loop
    // digitalWrite(DIR_PIN, !digitalRead(DIR_PIN));  // Reverse direction
    // stepCount = 0;  // Reset step count
    // stepDelay = 2000;  // Reset the step delay to initial value

    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        servo2.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
      }
    
      for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        servo2.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
      }
    }