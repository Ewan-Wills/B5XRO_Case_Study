// #define STEP_X 2
// #define DIR_X 3
// #define STEP_Y 4
// #define DIR_Y 5

// #define STEPS_PER_MM 100  // Adjust based on motor & mechanism

// void moveStepper(int stepPin, int dirPin, int steps, bool direction) {
//     digitalWrite(dirPin, direction);  // Set direction
//     for (int i = 0; i < abs(steps); i++) {
//         digitalWrite(stepPin, HIGH);
//         delayMicroseconds(500);  // Adjust for speed
//         digitalWrite(stepPin, LOW);
//         delayMicroseconds(500);
//     }
// }

// void moveToXY(int x_target, int y_target, int x_current, int y_current) {
//     int stepsX = (x_target - x_current) * STEPS_PER_MM;
//     int stepsY = (y_target - y_current) * STEPS_PER_MM;

//     bool dirX = stepsX > 0;
//     bool dirY = stepsY > 0;

//     moveStepper(STEP_X, DIR_X, stepsX, dirX);
//     moveStepper(STEP_Y, DIR_Y, stepsY, dirY);
// }

// void setup() {
//     pinMode(STEP_X, OUTPUT);
//     pinMode(DIR_X, OUTPUT);
//     pinMode(STEP_Y, OUTPUT);
//     pinMode(DIR_Y, OUTPUT);
// }

// void loop() {
//     moveToXY(10, 10, 0, 0);  // Move from (0,0) to (10,10)
//     delay(2000);
//     moveToXY(0, 0, 10, 10);  // Move back
//     delay(2000);
// }
