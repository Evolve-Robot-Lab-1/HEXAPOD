// Hexapod Sweep All 6 Legs - runs on Arduino, no serial dependency
// Sweeps each servo center-60 -> center+60 -> center, then next
// Re-centers all on any reset

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50

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

void slowMove(int s, int target) {
  if (target < 100) target = 100;
  if (target > 480) target = 480;
  int step = (target > curPos[s]) ? 1 : -1;
  while (curPos[s] != target) {
    curPos[s] += step;
    setServo(s, curPos[s]);
    delay(10);
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
  Serial.println("All centered. Sweep starting...");
  delay(2000);

  // Sweep leg by leg, re-init PCA and re-center all after each leg
  for (int leg = 0; leg < 6; leg++) {
    int base = leg * 3;
    for (int j = 0; j < 3; j++) {
      int s = base + j;
      int c = centers[s];
      slowMove(s, c - 30);
      delay(1500);
      slowMove(s, c + 30);
      delay(1500);
      slowMove(s, c);
      delay(1000);
    }
    // Re-init PCA and re-center all after each leg
    pca1.begin();
    pca1.setPWMFreq(SERVO_FREQ);
    delay(10);
    pca2.begin();
    pca2.setPWMFreq(SERVO_FREQ);
    delay(10);
    centerAll();
    delay(1000);
  }

  Serial.println("Sweep done. All at center.");
}

void loop() {
  // Stay centered, do nothing
}
