/*
 * Blink Test for Arduino Uno
 * Turns an LED on and off for one second intervals.
 * Uses the built-in LED on pin 13.
 */

#define LED_PIN 13

void setup() {
  // Initialize the digital pin as an output
  pinMode(LED_PIN, OUTPUT);
  
  // Start serial communication for debugging
  Serial.begin(9600);
  Serial.println("Blink test started");
}

void loop() {
  // Turn LED on
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ON");
  delay(1000);
  
  // Turn LED off
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF");
  delay(1000);
}