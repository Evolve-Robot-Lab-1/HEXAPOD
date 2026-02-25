// Hexapod All 6 Legs - Dual PCA9685
// PCA1 (0x40): Legs 1-3, PCA2 (0x41): Legs 4-6
// Commands: sN = select servo 0-17, NNN = move to pulse (100-480)
// Servo mapping: 0-8 = PCA1 ch0-8, 9-17 = PCA2 ch0-8

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50

// PCA1 (0x40) - Legs 1-3
#define L1_COXA   280  // s0
#define L1_FEMUR  290  // s1
#define L1_TIBIA  330  // s2
#define L2_COXA   300  // s3
#define L2_FEMUR  330  // s4
#define L2_TIBIA  310  // s5
#define L3_COXA   330  // s6
#define L3_FEMUR  310  // s7
#define L3_TIBIA  290  // s8

// PCA2 (0x41) - Legs 4-6
#define L4_COXA   300  // s9
#define L4_FEMUR  300  // s10
#define L4_TIBIA  270  // s11
#define L5_COXA   260  // s12
#define L5_FEMUR  230  // s13
#define L5_TIBIA  320  // s14
#define L6_COXA   280  // s15
#define L6_FEMUR  250  // s16
#define L6_TIBIA  300  // s17

int centers[18] = {
  L1_COXA, L1_FEMUR, L1_TIBIA,
  L2_COXA, L2_FEMUR, L2_TIBIA,
  L3_COXA, L3_FEMUR, L3_TIBIA,
  L4_COXA, L4_FEMUR, L4_TIBIA,
  L5_COXA, L5_FEMUR, L5_TIBIA,
  L6_COXA, L6_FEMUR, L6_TIBIA
};

int chPos[18];
int activeCh = 0;

void setServo(int servo, int val) {
  if (servo < 9) {
    pca1.setPWM(servo, 0, val);
  } else {
    pca2.setPWM(servo - 9, 0, val);
  }
}

void slowMove(int servo, int target) {
  if (target < 100) target = 100;
  if (target > 480) target = 480;
  int step = (target > chPos[servo]) ? 1 : -1;
  while (chPos[servo] != target) {
    chPos[servo] += step;
    setServo(servo, chPos[servo]);
    delay(3);
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

  // Center all 18 servos
  for (int i = 0; i < 18; i++) {
    chPos[i] = centers[i];
    setServo(i, centers[i]);
  }

  Serial.println("Hexapod 6 legs ready");
  Serial.println("s0-s17 = select, 100-480 = move");
  Serial.println("L1:s0-2 L2:s3-5 L3:s6-8");
  Serial.println("L4:s9-11 L5:s12-14 L6:s15-17");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("s")) {
      activeCh = input.substring(1).toInt();
      if (activeCh >= 0 && activeCh < 18) {
        Serial.print("s"); Serial.print(activeCh);
        Serial.print(" pos="); Serial.println(chPos[activeCh]);
      }
    } else {
      int val = input.toInt();
      if (val >= 100 && val <= 480) {
        Serial.print("s"); Serial.print(activeCh);
        Serial.print(" -> "); Serial.println(val);
        slowMove(activeCh, val);
      }
    }
  }
}
