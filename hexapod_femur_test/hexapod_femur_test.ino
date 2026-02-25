// Hexapod Femur-only sit-stand test
// Only femurs move, tibias and coxas stay at center

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50
#define SIT_OFFSET 40

int centers[18] = {
  280, 290, 330,  // L1
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
  Serial.println("Centered. Femur test in 3s...");
  delay(3000);
}

void loop() {
  int targets[18];
  for (int i = 0; i < 18; i++) targets[i] = centers[i];

  // Femur sit directions: L1=+, L2=-, L3=-, L4=-, L5=+, L6=+
  int femurs[] = {1, 4, 7, 10, 13, 16};
  int femurDir[] = {1, -1, -1, -1, 1, 1};

  for (int i = 0; i < 6; i++) {
    targets[femurs[i]] = centers[femurs[i]] + (SIT_OFFSET * femurDir[i]);
  }

  Serial.println("Femurs down...");
  smoothMoveAll(targets, 8);
  delay(2000);

  Serial.println("Femurs center...");
  int stand[18];
  for (int i = 0; i < 18; i++) stand[i] = centers[i];
  smoothMoveAll(stand, 8);
  delay(2000);
}
