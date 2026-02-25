#include <Wire.h>

void setup() {
  Serial.begin(9600);
  delay(2000);
  Wire.begin();
  Serial.println("Scanning ALL addresses 0x01-0x7F...");

  byte count = 0;
  for (byte addr = 1; addr < 128; addr++) {
    Wire.beginTransmission(addr);
    byte err = Wire.endTransmission();
    Serial.print("0x");
    if (addr < 16) Serial.print("0");
    Serial.print(addr, HEX);
    Serial.print(": ");
    if (err == 0) {
      Serial.println("FOUND!");
      count++;
    } else {
      Serial.println(err);
    }
  }
  Serial.print("Total: ");
  Serial.println(count);
  Serial.println("DONE");
}

void loop() {}
