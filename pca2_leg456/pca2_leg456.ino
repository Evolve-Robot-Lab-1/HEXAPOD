// PCA2 (0x41) - Leg 4-6 center test
// Leg 4 on ch0-2, Leg 5 on ch3-5, Leg 6 on ch6-8
// Commands: sN = select channel, NNN = move to pulse (100-480)

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca2 = Adafruit_PWMServoDriver(0x41);

#define SERVO_FREQ 50

#define CH0_CENTER 300  // L4 Coxa
#define CH1_CENTER 300  // L4 Femur
#define CH2_CENTER 270  // L4 Tibia
#define CH3_CENTER 260  // L5 Coxa
#define CH4_CENTER 230  // L5 Femur
#define CH5_CENTER 320  // L5 Tibia
#define CH6_CENTER 280  // L6 Coxa
#define CH7_CENTER 250  // L6 Femur
#define CH8_CENTER 300  // L6 Tibia

int chPos[16];
int activeCh = 0;

void slowMove(int ch, int target) {
  if (target < 100) target = 100;
  if (target > 480) target = 480;
  int step = (target > chPos[ch]) ? 1 : -1;
  while (chPos[ch] != target) {
    chPos[ch] += step;
    pca2.setPWM(ch, 0, chPos[ch]);
    delay(3);
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  pca2.begin();
  pca2.setPWMFreq(SERVO_FREQ);
  delay(10);

  chPos[0] = CH0_CENTER; pca2.setPWM(0, 0, CH0_CENTER);
  chPos[1] = CH1_CENTER; pca2.setPWM(1, 0, CH1_CENTER);
  chPos[2] = CH2_CENTER; pca2.setPWM(2, 0, CH2_CENTER);
  chPos[3] = CH3_CENTER; pca2.setPWM(3, 0, CH3_CENTER);
  chPos[4] = CH4_CENTER; pca2.setPWM(4, 0, CH4_CENTER);
  chPos[5] = CH5_CENTER; pca2.setPWM(5, 0, CH5_CENTER);

  chPos[6] = CH6_CENTER; pca2.setPWM(6, 0, CH6_CENTER);
  chPos[7] = CH7_CENTER; pca2.setPWM(7, 0, CH7_CENTER);
  chPos[8] = CH8_CENTER; pca2.setPWM(8, 0, CH8_CENTER);

  for (int i = 9; i < 16; i++) chPos[i] = 280;

  Serial.println("PCA2 0x41 - L4 centered. sN=select ch, NNN=move");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("s")) {
      activeCh = input.substring(1).toInt();
      Serial.print("ch"); Serial.print(activeCh);
      Serial.print(" pos="); Serial.println(chPos[activeCh]);
    } else {
      int val = input.toInt();
      if (val >= 100 && val <= 480) {
        Serial.print("ch"); Serial.print(activeCh);
        Serial.print(" -> "); Serial.println(val);
        slowMove(activeCh, val);
      }
    }
  }
}
