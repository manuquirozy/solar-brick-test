#include <Arduino.h>

//Simple program for obtaining the voltage measurement from 2 pins
//Defining a mapping function, as well as a printing function
// WORKS 20th of November
#define input_pin_vol_plus A5                           //Defining the analog pin number 5 to be for the plus cable of the attached source
#define input_pin_vol_minus A4                          //Defining the analog pin number 5 to be for the minus cable of the attached source

//variable for storing voltages
int   vol_plus_analog;                                  //Stores the analog signal of the plus cable
int   vol_minus_analog;                                 //Stores the analog signal of the minus cable
int   difference_analog;                                //Stores the calculated difference of the analog signal
float vol_plus_fin;                                     //Stores the analog signal of the plus calculated (mapped) voltage
float vol_minus_fin;                                    //Stores the analog signal of the minus calculated (mapped) voltage
float difference_fin;                                   //Stores the calculated difference of the voltage signals 

//Definition of the printing commands to print them all straight, printing of analog signals, difference as well as voltage signals and difference
void printValues(){
  Serial.println("ANA pluspin:      " + String(vol_plus_analog) + "       ANA minuspin:           " + String(vol_minus_analog) + "    ANA difference:    " + String(difference_analog));    //command to print the analog signal to the debug screen
  Serial.println("voltage_pluspin:  " + String(vol_plus_fin,4) + "  voltage_minuspin:  " + String(vol_minus_fin,4)+ "    Vol difference:    " + String(difference_fin,4) + "\n");         //command to print the (mapped) voltage signals to the screen
}

//Setting up of the program
void setup() {
Serial.begin(9600);                                                                                               //communication with 9600 bauds
}

//Setting up the loop for reading, calculating and printing
void loop() {
  vol_plus_analog = analogRead(input_pin_vol_plus);                                                               //command to read the pin which is connected to the plus cable
  vol_minus_analog = analogRead(input_pin_vol_minus);                                                             //command to read the pin which is connected to the minus cable
  vol_plus_fin = vol_plus_analog * 5.020 / 1023.0;                                                     //command to convert the analog signal of the input plus voltage towards the plus voltage signal
  vol_minus_fin = vol_minus_analog * 5.020 / 1023.0;                                                 //command to convert the analog signal of the input minus voltage towards the minus voltage signal
  difference_analog=vol_plus_analog-vol_minus_analog;                                                             //calculates the difference of the analog singals
  difference_fin = vol_plus_fin - vol_minus_fin;                                                                  //calculates the difference of the voltage signals
  printValues();                                                                                                  //calls the printing command
  delay (1000);                                                                                                    //delay for easier reading
}