/*
 * Arduino Spider Robot Test Program
 * 
 * This sketch tests the basic functionality of the spider robot
 * including servo movement, leg coordination, and gait patterns.
 * 
 * Author: [Your Name]
 * Date: [Current Date]
 * Version: 1.0
 */

#include <Servo.h>

// Servo pin definitions
#define SERVO_COUNT 8
int servoPins[SERVO_COUNT] = {2, 3, 4, 5, 6, 7, 8, 9};

// Servo objects
Servo servos[SERVO_COUNT];

// Test parameters
#define TEST_DELAY 1000
#define SERVO_MIN 0
#define SERVO_MAX 180

void setup() {
  Serial.begin(9600);
  Serial.println("Spider Robot Test Program");
  Serial.println("==========================");
  
  // Initialize servos
  initializeServos();
  
  // Run startup test
  runStartupTest();
}

void loop() {
  // Main test loop
  Serial.println("Running test sequence...");
  
  // Test individual servos
  testIndividualServos();
  
  // Test coordinated movement
  testCoordinatedMovement();
  
  // Test gait patterns
  testGaitPatterns();
  
  delay(TEST_DELAY * 2);
}

void initializeServos() {
  Serial.println("Initializing servos...");
  
  for (int i = 0; i < SERVO_COUNT; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90); // Center position
    delay(100);
  }
  
  Serial.println("Servos initialized");
}

void runStartupTest() {
  Serial.println("Running startup test...");
  
  // Move all servos to center position
  for (int i = 0; i < SERVO_COUNT; i++) {
    servos[i].write(90);
  }
  
  delay(TEST_DELAY);
  
  // Test sweep on first servo
  Serial.println("Testing servo sweep...");
  servos[0].write(0);
  delay(500);
  servos[0].write(180);
  delay(500);
  servos[0].write(90);
  
  Serial.println("Startup test complete");
}

void testIndividualServos() {
  Serial.println("Testing individual servos...");
  
  for (int i = 0; i < SERVO_COUNT; i++) {
    Serial.print("Testing servo ");
    Serial.println(i + 1);
    
    // Test sweep
    servos[i].write(SERVO_MIN);
    delay(500);
    servos[i].write(SERVO_MAX);
    delay(500);
    servos[i].write(90);
    delay(200);
  }
  
  Serial.println("Individual servo test complete");
}

void testCoordinatedMovement() {
  Serial.println("Testing coordinated movement...");
  
  // Test alternating movement
  for (int pos = 0; pos <= 180; pos += 10) {
    for (int i = 0; i < SERVO_COUNT; i += 2) {
      servos[i].write(pos);
      servos[i + 1].write(180 - pos);
    }
    delay(50);
  }
  
  // Return to center
  for (int i = 0; i < SERVO_COUNT; i++) {
    servos[i].write(90);
  }
  
  Serial.println("Coordinated movement test complete");
}

void testGaitPatterns() {
  Serial.println("Testing gait patterns...");
  
  // Simple walking pattern
  for (int step = 0; step < 4; step++) {
    Serial.print("Walking step ");
    Serial.println(step + 1);
    
    // Lift legs
    for (int i = 0; i < 4; i++) {
      servos[i * 2].write(120);
    }
    delay(200);
    
    // Move forward
    for (int i = 0; i < 4; i++) {
      servos[i * 2 + 1].write(60);
    }
    delay(200);
    
    // Place legs down
    for (int i = 0; i < 4; i++) {
      servos[i * 2].write(90);
    }
    delay(200);
    
    // Reset
    for (int i = 0; i < SERVO_COUNT; i++) {
      servos[i].write(90);
    }
    delay(300);
  }
  
  Serial.println("Gait pattern test complete");
}

// Utility function for debugging
void printServoPositions() {
  Serial.println("Current servo positions:");
  for (int i = 0; i < SERVO_COUNT; i++) {
    Serial.print("Servo ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(servos[i].read());
  }
}