#include <Arduino.h>
#include <Stepper.h>
#include <ESP32Servo.h>
// change this to the number of steps on your motor
#define STEPS 200

//define all stepper pins
#define xmotor1Pin1 12
#define xmotor1Pin2 14

#define rackServoPin 19
#define screwServoPin 18
Servo rackServo;
Servo screwServo;

//values for end effector
#define rackBottom 180
#define rackTop 0

#define screwOff 90
#define screwIn 180
#define screwOut 0
//speed at which the rack will go up/down. Lower means faster
#define rackSpeed 100

int stepDelay = 2000;  // Initial delay between steps (microseconds)
int minStepDelay = 500;  // Minimum delay (maximum speed)
int acceleration = 10;  // Acceleration factor (lower value = faster acceleration)
int totalSteps = 200;  // Total steps (1 full rotation for 1.8° step motor)
int stepCount = 0;  // Track the number of steps taken

void screw(int value){
  screwServo.write(value);
}
void rackDown (int speed){
  if (speed==-1){
    rackServo.write(rackBottom);
  }else{
    for (int i = rackTop; i<=rackBottom; i++){
      rackServo.write(i);
      delay(speed);
    }
  }
}
void rackUp(int speed){
  if (speed==-1){
    rackServo.write(rackTop);
  }else{
    for (int i = rackBottom; i>=rackTop; i--){
      rackServo.write(i);
      delay(speed);
  }
  }
}
void pickUp(){
  rackDown(-1);
  delay(100);
  screw(screwOut);
  delay(500);
  rackUp(rackSpeed);
  screw(screwOff);
}

void putDown(){
  rackUp(-1);
  delay(100);
  screw(screwIn);
  rackDown(rackSpeed);
  delay(100);
  screw(screwOff);
  rackUp(-1);
}
void setup() {
    Serial.begin(115200);
    rackServo.attach(rackServoPin);
    screwServo.attach(screwServoPin);

    pinMode(xmotor1Pin1, OUTPUT);
    pinMode(xmotor1Pin2, OUTPUT);
    digitalWrite(xmotor1Pin2, HIGH);  // Set motor direction
    rackServo.write(rackTop);
    screw(screwOff);
    delay(100);
}
void loop() {
    
    
    // Serial.println("Picking Up");
    // pickUp();
    
    // Serial.println("Waiting");
    // delay(5000);

    // Serial.println("Placing bolt");
    // putDown();
    
    // Serial.println("Waiting");
    // delay(5000);

    }