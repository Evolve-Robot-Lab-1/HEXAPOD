// Hexapod Movement Controller
// Serial commands: w=forward, x=backward, a=turn left, d=turn right
//                  s=stop, c=center, +=faster, -=slower
// Ready for BLE/WiFi/ESP32 — just replace serial input source

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50
#define NUM_SERVOS 18
#define NUM_LEGS   6
#define GROUP_SIZE 3

// Gait parameters
int coxaSwing  = 40;
int liftOffset = 25;
int stepDelay  = 10;
int phasePause = 30;

// Movement mode: 0=idle, 1=forward, 2=backward, 3=turn_left, 4=turn_right
int mode = 0;

// Calibrated centers (2026-02-27)
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

const int groupA[GROUP_SIZE] = {0, 2, 4};  // L1, L3, L5
const int groupB[GROUP_SIZE] = {1, 3, 5};  // L2, L4, L6

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

// ===================== Walk step =====================

void halfStep(const int swingGroup[], const int stanceGroup[]) {
  int targets[NUM_SERVOS];

  // 1. Lift swing group
  copyToTargets(targets);
  setLift(targets, swingGroup, true);
  smoothMoveAll(targets, stepDelay);
  delay(phasePause);

  // Check for mode change
  checkSerial();
  if (mode == 0) return;

  // 2. Move coxas based on mode
  copyToTargets(targets);
  switch (mode) {
    case 1:  // forward
      setCoxaSwing(targets, swingGroup, 1);
      setCoxaSwing(targets, stanceGroup, -1);
      break;
    case 2:  // backward
      setCoxaSwing(targets, swingGroup, -1);
      setCoxaSwing(targets, stanceGroup, 1);
      break;
    case 3:  // turn left — all coxas rotate same direction
      setCoxaSwing(targets, swingGroup, 1);
      setCoxaSwing(targets, stanceGroup, 1);
      break;
    case 4:  // turn right — all coxas rotate same direction
      setCoxaSwing(targets, swingGroup, -1);
      setCoxaSwing(targets, stanceGroup, -1);
      break;
  }
  smoothMoveAll(targets, stepDelay);
  delay(phasePause);

  // 3. Lower swing group
  copyToTargets(targets);
  setLift(targets, swingGroup, false);
  smoothMoveAll(targets, stepDelay);
  delay(phasePause);
}

// ===================== Serial =====================

void checkSerial() {
  if (!Serial.available()) return;
  char ch = Serial.read();
  switch (ch) {
    case 'w': mode = 1; Serial.println(">> Forward");  break;
    case 'x': mode = 2; Serial.println(">> Backward"); break;
    case 'a': mode = 3; Serial.println(">> Turn Left");  break;
    case 'd': mode = 4; Serial.println(">> Turn Right"); break;
    case 's':
      mode = 0;
      goToCenter();
      Serial.println(">> Stopped");
      break;
    case 'c':
      mode = 0;
      goToCenter();
      Serial.println(">> Centered");
      break;
    case '+':
      if (stepDelay > 3) stepDelay -= 2;
      Serial.print(">> Speed up, delay="); Serial.println(stepDelay);
      break;
    case '-':
      stepDelay += 2;
      Serial.print(">> Slow down, delay="); Serial.println(stepDelay);
      break;
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
  Serial.println("w=fwd x=back a=left d=right s=stop c=center +=fast -=slow");
}

void loop() {
  checkSerial();

  if (mode > 0) {
    halfStep(groupA, groupB);
    if (mode == 0) return;
    halfStep(groupB, groupA);
  } else {
    delay(50);
  }
}
