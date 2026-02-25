// Hexapod Sit-to-Stand Test
// Centers = standing. Femur+Tibia offset = sit.
// Runs: center(stand) -> sit -> stand -> repeat

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50
#define SIT_OFFSET 25  // how far femur/tibia move for sit

int centers[18] = {
  280, 290, 330,  // L1: coxa, femur, tibia
  300, 330, 310,  // L2
  330, 310, 290,  // L3
  300, 300, 270,  // L4
  260, 230, 320,  // L5
  280, 250, 300   // L6
};

int curPos[18];

void setServo(int s, int val) {
  if (s < 9) pca1.setPWM(s, 0, val);
  else pca2.setPWM(s - 9, 0, val);
}

// Move all 18 servos smoothly toward their targets
void smoothMoveAll(int targets[18], int stepDelay) {
  bool moving = true;
  while (moving) {
    moving = false;
    for (int i = 0; i < 18; i++) {
      if (curPos[i] != targets[i]) {
        moving = true;
        if (curPos[i] < targets[i]) curPos[i]++;
        else curPos[i]--;
        setServo(i, curPos[i]);
      }
    }
    delay(stepDelay);
  }
}

void centerAll() {
  for (int i = 0; i < 18; i++) {
    curPos[i] = centers[i];
    setServo(i, centers[i]);
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  pca1.begin();
  pca1.setPWMFreq(SERVO_FREQ);
  delay(10);
  pca2.begin();
  pca2.setPWMFreq(SERVO_FREQ);
  delay(10);

  centerAll();
  Serial.println("Standing. Starting sit-stand in 3s...");
  delay(3000);
}

void loop() {
  // SIT: move femurs and tibias
  int sitTargets[18];
  for (int i = 0; i < 18; i++) sitTargets[i] = centers[i];

  // Femur sit directions: L1=+, L2=-, L3=-, L4=-, L5=+, L6=+
  // Tibia sit directions: L1=-, L2=+, L3=+, L4=+, L5=-, L6=-
  int femurs[] = {1, 4, 7, 10, 13, 16};
  int tibias[] = {2, 5, 8, 11, 14, 17};
  int femurDir[] = {1, -1, -1, -1, 1, 1};
  int tibiaDir[] = {-1, 1, 1, 1, -1, -1};

  for (int i = 0; i < 6; i++) {
    sitTargets[femurs[i]] = centers[femurs[i]] + (SIT_OFFSET * femurDir[i]);
    sitTargets[tibias[i]] = centers[tibias[i]] + (SIT_OFFSET * tibiaDir[i]);
  }

  Serial.println("Sitting...");
  smoothMoveAll(sitTargets, 15);
  delay(2000);

  // STAND: back to center
  Serial.println("Standing...");
  int standTargets[18];
  for (int i = 0; i < 18; i++) standTargets[i] = centers[i];
  smoothMoveAll(standTargets, 15);
  delay(2000);
}
