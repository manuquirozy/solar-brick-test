#include <Arduino.h>
//Definicion de pines
#define WCS1800 A2

//Definicion de constantes
const int numReadings = 40;
const unsigned long tsam = 1000;
float cur_fin = 0;

//Definicion de variables
float I = 0;  //Float I for current in volts
//Vars for smoothing
double InputC = 0;
int readingsC[numReadings] = {0};
int readIndexC = 0;
long totalC = 0;
//Timing vars
unsigned long tini = 0;
unsigned long tact = 0;
unsigned long trel = 0;

//Subrutinas
unsigned long smooth(int pin, long &total, int *readings, int &readIndex) {
  // subtract the last reading
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(pin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  return total / numReadings;
}

void setup() {
  //Comunicaciones
  Serial.begin(9600);
  tini = millis();
}


void loop() {
  tact = millis();
  InputC = smooth(WCS1800, totalC, readingsC, readIndexC);
  trel = tact - tini;
  if(trel >= tsam) {
    I = InputC * 5.0 / 1023.0;
    cur_fin = ((14.468 * I - 34.989)) * 1000;
    Serial.print("Current (A): ");
    Serial.println(cur_fin);
  }
}