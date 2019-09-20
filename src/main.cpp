#include <Arduino.h>

#define input_pin_cur A3

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(analogRead(input_pin_cur));
  delay(500);
}