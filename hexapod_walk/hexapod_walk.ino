// Hexapod Movement Controller + Dance + Wave
// w=fwd x=back a=left d=right s=stop c=center +=fast -=slow
// 1=dance 2=wave walk 3=body sway

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50
#define NUM_SERVOS 18
#define NUM_LEGS   6
#define GROUP_SIZE 3

int coxaSwing  = 40;
int liftOffset = 25;
int stepDelay  = 10;
int phasePause = 30;

// mode: 0=idle 1=fwd 2=back 3=left 4=right 5=dance 6=wave 7=sway
int mode = 0;

int centers[NUM_SERVOS] = {
  280, 280, 330,  // L1
  300, 320, 300,  // L2
  330, 310, 290,  // L3
  300, 300, 270,  // L4
  260, 230, 320,  // L5
  280, 230, 300   // L6
};

int curPos[NUM_SERVOS];

const int coxas[NUM_LEGS]  = {0, 3, 6, 9, 12, 15};
const int femurs[NUM_LEGS] = {1, 4, 7, 10, 13, 16};
const int tibias[NUM_LEGS] = {2, 5, 8, 11, 14, 17};

const int coxaDir[NUM_LEGS]  = {1, 1, 1, -1, -1, -1};
const int femurDir[NUM_LEGS] = {1, -1, -1, -1, 1, 1};
const int tibiaDir[NUM_LEGS] = {-1, 1, 1, 1, -1, -1};

const int groupA[GROUP_SIZE] = {0, 2, 4};
const int groupB[GROUP_SIZE] = {1, 3, 5};

// ===================== Low-level =====================

void setServo(int s, int val) {
  val = constrain(val, 100, 480);
  if (s < 9) pca1.setPWM(s, 0, val);
  else pca2.setPWM(s - 9, 0, val);
}

void smoothMoveAll(int targets[], int sd) {
  int maxDist = 0;
  for (int i = 0; i < NUM_SERVOS; i++) {
    int d = abs(targets[i] - curPos[i]);
    if (d > maxDist) maxDist = d;
  }
  if (maxDist == 0) return;
  int steps = (maxDist + 2) / 3;
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
    delay(sd);
  }
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

void goToCenter() {
  int targets[NUM_SERVOS];
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  smoothMoveAll(targets, stepDelay);
}

void copyToTargets(int targets[]) {
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = curPos[i];
}

// ===================== Gait helpers =====================

void setLift(int targets[], const int group[], bool lift) {
  for (int i = 0; i < GROUP_SIZE; i++) {
    int leg = group[i];
    if (lift) {
      targets[femurs[leg]] = centers[femurs[leg]] + (liftOffset * -femurDir[leg]);
      targets[tibias[leg]] = centers[tibias[leg]] + (liftOffset * -tibiaDir[leg]);
    } else {
      targets[femurs[leg]] = centers[femurs[leg]];
      targets[tibias[leg]] = centers[tibias[leg]];
    }
  }
}

void setCoxaSwing(int targets[], const int group[], int swingDir) {
  for (int i = 0; i < GROUP_SIZE; i++) {
    int leg = group[i];
    targets[coxas[leg]] = centers[coxas[leg]] + (coxaSwing * swingDir * coxaDir[leg]);
  }
}

// ===================== Tripod walk =====================

void halfStep(const int swingGroup[], const int stanceGroup[]) {
  int targets[NUM_SERVOS];
  copyToTargets(targets);
  setLift(targets, swingGroup, true);
  smoothMoveAll(targets, stepDelay);
  delay(phasePause);
  checkSerial();
  if (mode == 0 || mode > 4) return;

  copyToTargets(targets);
  switch (mode) {
    case 1: setCoxaSwing(targets, swingGroup, 1); setCoxaSwing(targets, stanceGroup, -1); break;
    case 2: setCoxaSwing(targets, swingGroup, -1); setCoxaSwing(targets, stanceGroup, 1); break;
    case 3: setCoxaSwing(targets, swingGroup, 1); setCoxaSwing(targets, stanceGroup, 1); break;
    case 4: setCoxaSwing(targets, swingGroup, -1); setCoxaSwing(targets, stanceGroup, -1); break;
  }
  smoothMoveAll(targets, stepDelay);
  delay(phasePause);

  copyToTargets(targets);
  setLift(targets, swingGroup, false);
  smoothMoveAll(targets, stepDelay);
  delay(phasePause);
}

// ===================== Dance =====================

void doBounce() {
  int targets[NUM_SERVOS];
  // Down
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < NUM_LEGS; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (30 * femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (30 * tibiaDir[leg]);
  }
  smoothMoveAll(targets, 6);
  delay(150);
  // Up
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < NUM_LEGS; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (20 * -femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (20 * -tibiaDir[leg]);
  }
  smoothMoveAll(targets, 6);
  delay(150);
}

void doTwist() {
  int targets[NUM_SERVOS];
  // Twist right
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < NUM_LEGS; leg++)
    targets[coxas[leg]] = centers[coxas[leg]] + (35 * coxaDir[leg]);
  smoothMoveAll(targets, 6);
  delay(200);
  // Twist left
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < NUM_LEGS; leg++)
    targets[coxas[leg]] = centers[coxas[leg]] + (35 * -coxaDir[leg]);
  smoothMoveAll(targets, 6);
  delay(200);
}

void doLean(int side) {
  // side: 1=right(L1-3 down, L4-6 up), -1=left
  int targets[NUM_SERVOS];
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < 3; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (25 * side * femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (25 * side * tibiaDir[leg]);
  }
  for (int leg = 3; leg < 6; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (25 * -side * femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (25 * -side * tibiaDir[leg]);
  }
  smoothMoveAll(targets, 8);
  delay(250);
}

void doGroupBounce() {
  int targets[NUM_SERVOS];
  // Group A up, B down
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int i = 0; i < GROUP_SIZE; i++) {
    int la = groupA[i]; int lb = groupB[i];
    targets[femurs[la]] = centers[femurs[la]] + (25 * -femurDir[la]);
    targets[tibias[la]] = centers[tibias[la]] + (25 * -tibiaDir[la]);
    targets[femurs[lb]] = centers[femurs[lb]] + (20 * femurDir[lb]);
    targets[tibias[lb]] = centers[tibias[lb]] + (20 * tibiaDir[lb]);
  }
  smoothMoveAll(targets, 6);
  delay(200);
  // Swap: A down, B up
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int i = 0; i < GROUP_SIZE; i++) {
    int la = groupA[i]; int lb = groupB[i];
    targets[femurs[la]] = centers[femurs[la]] + (20 * femurDir[la]);
    targets[tibias[la]] = centers[tibias[la]] + (20 * tibiaDir[la]);
    targets[femurs[lb]] = centers[femurs[lb]] + (25 * -femurDir[lb]);
    targets[tibias[lb]] = centers[tibias[lb]] + (25 * -tibiaDir[lb]);
  }
  smoothMoveAll(targets, 6);
  delay(200);
}

void danceLoop() {
  Serial.println("~ Bounce");
  doBounce(); checkSerial(); if (mode != 5) return;
  doBounce(); checkSerial(); if (mode != 5) return;

  Serial.println("~ Twist");
  doTwist(); checkSerial(); if (mode != 5) return;
  doTwist(); checkSerial(); if (mode != 5) return;

  Serial.println("~ Lean");
  doLean(1); checkSerial(); if (mode != 5) return;
  doLean(-1); checkSerial(); if (mode != 5) return;
  doLean(1); checkSerial(); if (mode != 5) return;
  doLean(-1); checkSerial(); if (mode != 5) return;

  Serial.println("~ Group bounce");
  doGroupBounce(); checkSerial(); if (mode != 5) return;
  doGroupBounce(); checkSerial(); if (mode != 5) return;
  doGroupBounce(); checkSerial(); if (mode != 5) return;

  Serial.println("~ Twist+Bounce");
  // Twist right + bounce
  int targets[NUM_SERVOS];
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < NUM_LEGS; leg++) {
    targets[coxas[leg]] = centers[coxas[leg]] + (30 * coxaDir[leg]);
    targets[femurs[leg]] = centers[femurs[leg]] + (25 * -femurDir[leg]);
  }
  smoothMoveAll(targets, 6);
  delay(150);
  checkSerial(); if (mode != 5) return;
  // Twist left + down
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < NUM_LEGS; leg++) {
    targets[coxas[leg]] = centers[coxas[leg]] + (30 * -coxaDir[leg]);
    targets[femurs[leg]] = centers[femurs[leg]] + (20 * femurDir[leg]);
  }
  smoothMoveAll(targets, 6);
  delay(150);
  checkSerial(); if (mode != 5) return;

  goToCenter();
  delay(300);
}

// ===================== Wave gait =====================

void waveLeg(int leg) {
  int targets[NUM_SERVOS];
  // Lift
  copyToTargets(targets);
  targets[femurs[leg]] = centers[femurs[leg]] + (liftOffset * -femurDir[leg]);
  targets[tibias[leg]] = centers[tibias[leg]] + (liftOffset * -tibiaDir[leg]);
  smoothMoveAll(targets, stepDelay);
  delay(20);
  // Swing forward
  copyToTargets(targets);
  targets[coxas[leg]] = centers[coxas[leg]] + (coxaSwing * coxaDir[leg]);
  smoothMoveAll(targets, stepDelay);
  delay(20);
  // Lower
  copyToTargets(targets);
  targets[femurs[leg]] = centers[femurs[leg]];
  targets[tibias[leg]] = centers[tibias[leg]];
  smoothMoveAll(targets, stepDelay);
  delay(20);
}

void waveStep() {
  // Wave: one leg at a time, order L1 L3 L5 L2 L4 L6
  int order[] = {0, 2, 4, 1, 3, 5};
  // Push all coxas back slightly
  int targets[NUM_SERVOS];
  copyToTargets(targets);
  for (int leg = 0; leg < NUM_LEGS; leg++)
    targets[coxas[leg]] = centers[coxas[leg]] + (8 * -coxaDir[leg]);
  smoothMoveAll(targets, stepDelay);

  for (int i = 0; i < 6; i++) {
    checkSerial();
    if (mode != 6) return;
    waveLeg(order[i]);
  }
}

// ===================== Body sway =====================

void swayLoop() {
  doLean(1);
  checkSerial(); if (mode != 7) return;
  doLean(-1);
  checkSerial(); if (mode != 7) return;
  // Forward/back lean
  int targets[NUM_SERVOS];
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  // Front legs down, back legs up
  for (int leg = 0; leg < 2; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (20 * femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (20 * tibiaDir[leg]);
  }
  for (int leg = 4; leg < 6; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (20 * -femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (20 * -tibiaDir[leg]);
  }
  smoothMoveAll(targets, 8);
  delay(300);
  checkSerial(); if (mode != 7) return;
  // Reverse
  for (int i = 0; i < NUM_SERVOS; i++) targets[i] = centers[i];
  for (int leg = 0; leg < 2; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (20 * -femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (20 * -tibiaDir[leg]);
  }
  for (int leg = 4; leg < 6; leg++) {
    targets[femurs[leg]] = centers[femurs[leg]] + (20 * femurDir[leg]);
    targets[tibias[leg]] = centers[tibias[leg]] + (20 * tibiaDir[leg]);
  }
  smoothMoveAll(targets, 8);
  delay(300);
  checkSerial(); if (mode != 7) return;
  goToCenter();
  delay(200);
}

// ===================== Serial =====================

void checkSerial() {
  if (!Serial.available()) return;
  char ch = Serial.read();
  switch (ch) {
    case 'w': mode = 1; Serial.println(">> Forward");     break;
    case 'x': mode = 2; Serial.println(">> Backward");    break;
    case 'a': mode = 3; Serial.println(">> Turn Left");   break;
    case 'd': mode = 4; Serial.println(">> Turn Right");  break;
    case '1': mode = 5; Serial.println(">> Dance!");      break;
    case '2': mode = 6; Serial.println(">> Wave Walk");   break;
    case '3': mode = 7; Serial.println(">> Body Sway");   break;
    case 's':
      mode = 0; goToCenter();
      Serial.println(">> Stopped");  break;
    case 'c':
      mode = 0; goToCenter();
      Serial.println(">> Centered"); break;
    case '+':
      if (stepDelay > 3) stepDelay -= 2;
      Serial.print(">> Speed: "); Serial.println(stepDelay); break;
    case '-':
      stepDelay += 2;
      Serial.print(">> Speed: "); Serial.println(stepDelay); break;
    default: break;
  }
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
  Serial.println("Hexapod ready.");
  Serial.println("w/x/a/d=move s=stop c=center +-=speed");
  Serial.println("1=dance 2=wave 3=sway");
}

void loop() {
  checkSerial();
  if (mode >= 1 && mode <= 4) {
    halfStep(groupA, groupB);
    if (mode == 0 || mode > 4) return;
    halfStep(groupB, groupA);
  } else if (mode == 5) {
    danceLoop();
  } else if (mode == 6) {
    waveStep();
  } else if (mode == 7) {
    swayLoop();
  } else {
    delay(50);
  }
}
