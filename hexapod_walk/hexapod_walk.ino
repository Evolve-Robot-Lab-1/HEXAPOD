// Hexapod Tripod Walk
// Phase 1: Simple tripod gait using pulse offsets (no IK)
// Board: Arduino Mega 2560
//
// Tripod groups:
//   Group A: L1 (front-right), L3 (back-right), L5 (mid-left)
//   Group B: L2 (mid-right),   L4 (back-left),  L6 (front-left)
//
// Walk cycle per half-step:
//   1. Lift swing group (femur+tibia up)
//   2. Swing forward + push back (coxa, simultaneous)
//   3. Lower swing group
//   4. Swap groups, repeat
//
// Serial commands: 's'=stop, 'g'=go, 'c'=center

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50
#define NUM_SERVOS 18
#define NUM_LEGS   6
#define GROUP_SIZE 3

// --- Gait parameters (tune these) ---
#define COXA_SWING  40   // coxa forward/back swing amount
#define LIFT_OFFSET 25   // femur+tibia lift height
#define STEP_DELAY  10   // ms per step (with step size 3)
#define PHASE_PAUSE 30   // ms pause between phases

// --- Center positions (standing neutral) ---
int centers[NUM_SERVOS] = {
  280, 280, 330,  // L1: coxa, femur, tibia
  300, 310, 310,  // L2
  330, 310, 290,  // L3
  300, 320, 270,  // L4
  260, 210, 320,  // L5
  280, 250, 300   // L6
};

int curPos[NUM_SERVOS];

// --- Servo indices by joint type (per leg L1-L6) ---
const int coxas[NUM_LEGS]  = {0, 3, 6, 9, 12, 15};
const int femurs[NUM_LEGS] = {1, 4, 7, 10, 13, 16};
const int tibias[NUM_LEGS] = {2, 5, 8, 11, 14, 17};

// --- Direction arrays (per leg L1-L6) ---
const int coxaDir[NUM_LEGS]  = {1, 1, 1, -1, -1, -1};  // R:+, L:-
const int femurDir[NUM_LEGS] = {1, -1, -1, -1, 1, 1};
const int tibiaDir[NUM_LEGS] = {1, -1, -1, -1, 1, 1};

// --- Tripod groups (leg indices 0-5 = L1-L6) ---
const int groupA[GROUP_SIZE] = {0, 2, 4};  // L1, L3, L5
const int groupB[GROUP_SIZE] = {1, 3, 5};  // L2, L4, L6

bool walking = false;  // start idle after stand

// ===================== Low-level =====================

void setServo(int s, int val) {
  val = constrain(val, 100, 480);
  if (s < 9) pca1.setPWM(s, 0, val);
  else pca2.setPWM(s - 9, 0, val);
}

void smoothMoveAll(int targets[], int stepDelay) {
  // Find max distance so all servos finish together
  int maxDist = 0;
  for (int i = 0; i < NUM_SERVOS; i++) {
    int d = abs(targets[i] - curPos[i]);
    if (d > maxDist) maxDist = d;
  }
  if (maxDist == 0) return;

  // Move proportionally — all finish at the same time
  int steps = (maxDist + 2) / 3;  // total ticks (step size ~3 for largest)
  int startPos[NUM_SERVOS];
  for (int i = 0; i < NUM_SERVOS; i++) startPos[i] = curPos[i];

  for (int s = 1; s <= steps; s++) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      int target = startPos[i] + (long)(targets[i] - startPos[i]) * s / steps;
      if (target != curPos[i]) {
        curPos[i] = target;
        setServo(i, curPos[i]);
      }
    }
    delay(stepDelay);
  }
  // Ensure exact target
  for (int i = 0; i < NUM_SERVOS; i++) {
    if (curPos[i] != targets[i]) {
      curPos[i] = targets[i];
      setServo(i, curPos[i]);
    }
  }
}

void centerAll() {
  for (int leg = 0; leg < NUM_LEGS; leg++) {
    int base = leg * 3;
    for (int j = 0; j < 3; j++) {
      curPos[base + j] = centers[base + j];
      setServo(base + j, centers[base + j]);
    }
    delay(100);
  }
}

void copyToTargets(int targets[]) {
  for (int i = 0; i < NUM_SERVOS; i++) {
    targets[i] = curPos[i];
  }
}

// ===================== Gait helpers =====================

void setLift(int targets[], const int group[], bool lift) {
  for (int i = 0; i < GROUP_SIZE; i++) {
    int leg = group[i];
    if (lift) {
      targets[femurs[leg]] = centers[femurs[leg]] + (LIFT_OFFSET * -femurDir[leg]);
      targets[tibias[leg]] = centers[tibias[leg]] + (LIFT_OFFSET * -tibiaDir[leg]);
    } else {
      targets[femurs[leg]] = centers[femurs[leg]];
      targets[tibias[leg]] = centers[tibias[leg]];
    }
  }
}

void setCoxaSwing(int targets[], const int group[], int swingDir) {
  for (int i = 0; i < GROUP_SIZE; i++) {
    int leg = group[i];
    targets[coxas[leg]] = centers[coxas[leg]] + (COXA_SWING * swingDir * coxaDir[leg]);
  }
}

// ===================== Walk step =====================

void halfStep(const int swingGroup[], const int stanceGroup[], const char* label) {
  int targets[NUM_SERVOS];

  Serial.print(label);
  Serial.println(": Lift");
  copyToTargets(targets);
  setLift(targets, swingGroup, true);
  smoothMoveAll(targets, STEP_DELAY);
  delay(PHASE_PAUSE);

  if (Serial.available() && Serial.peek() == 's') return;

  Serial.print(label);
  Serial.println(": Swing+Push");
  copyToTargets(targets);
  setCoxaSwing(targets, swingGroup, 1);
  setCoxaSwing(targets, stanceGroup, -1);
  smoothMoveAll(targets, STEP_DELAY);
  delay(PHASE_PAUSE);

  Serial.print(label);
  Serial.println(": Lower");
  copyToTargets(targets);
  setLift(targets, swingGroup, false);
  smoothMoveAll(targets, STEP_DELAY);
  delay(PHASE_PAUSE);
}

// ===================== Main =====================

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
  Serial.println("Standing idle. 'g'=walk, 's'=stop, 'c'=center");
}

void loop() {
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'g') {
      walking = true;
      Serial.println("Walking...");
    } else if (ch == 's') {
      walking = false;
      Serial.println("Stopped.");
    } else if (ch == 'c') {
      walking = false;
      int targets[NUM_SERVOS];
      for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
      smoothMoveAll(targets, STEP_DELAY);
      Serial.println("Centered.");
    }
  }

  if (walking) {
    halfStep(groupA, groupB, "A");
    if (!walking) return;
    halfStep(groupB, groupA, "B");
  } else {
    delay(100);
  }
}
