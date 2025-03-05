#define STEP_PIN 18
#define DIR_PIN 19

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