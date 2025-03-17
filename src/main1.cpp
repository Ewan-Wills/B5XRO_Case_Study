// #include <AxoStepper.h>

// #define STEP_PIN 18
// #define DIR_PIN 19

// AxoStepper stepper(STEP_PIN, DIR_PIN);

// void setup() {
//     stepper.setSpeed(200); // Steps per second
//     stepper.setAcceleration(200); // Steps per second²
// }

// void loop() {
//     stepper.move(200); // Move 200 steps forward (1 full rotation if 1.8° step motor)
//     stepper.wait(); // Wait until movement completes

//     delay(1000); // Pause

//     stepper.move(-200); // Move back 200 steps
//     stepper.wait();

//     delay(1000);
// }