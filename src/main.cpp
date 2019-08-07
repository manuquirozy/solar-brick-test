#include <Arduino.h>
//program to obtain the voltage measurements from 2 pins
//Defining a mapping function as well as a printing function
//Defining also a smoothing function
//Including finite state machines
//WORKS 20th of Nov
//Update 27th of Nov, also includes wait signal now! Works 27th of NOV with 24 % of pro stor space and 27% of dynamic memory

#define input_pin_vol_plus A0                                                     //Defining the analog pin number 5 to be for the plus cable of the attached source
#define input_pin_vol_minus A1                                                    //Defining the analog pin number 5 to be for the minus cable of the attached source
#define output_control 7                                                         //Defining the pin that operates the switch (relay or mosfet) it is  the digital output pin 10
#define CIRCUITCLOSED 1                                                           //Defining the FSM ONE, which is a short circuit
#define CIRCUITOPENED 2                                                           //Defining the FSM TWO, which is an open circuit

//current state variable declaration
unsigned int state = CIRCUITOPENED;                                               //Defines the inital state to start the whole program

//variable for storing voltages
int   vol_plus_analog;                                                            //Stores the analog signal of the plus cable
int   vol_minus_analog;                                                           //Stores the analog signal of the minus cable
int   difference_analog;                                                          //Stores the calculated difference of the analog signal
float vol_plus_fin;                                                               //Stores the analog signal of the plus calculated (mapped) voltage
float vol_minus_fin;                                                              //Stores the analog signal of the minus calculated (mapped) voltage
float difference_fin;                                                             //Stores the calculated difference of the voltage signals

//variables for mapping function
int vol_max = 5.0;                                                                //5V reference from the Arduino
int analog_max = 1023.0;                                                          //max possible Analog, the minima is 0 and the maximum analog number is 1023

//Definition of the contants for reading for both, voltage and current
const int numReadings    = 40;                                                    //Describes the number of readings can be increased to stabilize the smoothing function, so right now the array is 80
const unsigned long tsam = 1000;                                                  //sampling time to take measurements, which at the moment is 1s

//Timing variables
unsigned long tini = 0;                                                           //Initial timing variable
unsigned long tact = 0;                                                           //Actual timing variable
unsigned long trel = 0;                                                           //Relative timing variable
unsigned long TS   = 2000;                                                       //Time for one state

//Vars for smoothing for voltage (+)
int     readings_vol_plus[numReadings] = {0};                                     //Definition of the Array (vol, pos)
int     readIndex_vol_plus = 0;                                                   //Index reading (vol, pos)
long    total_vol_plus = 0;                                                       //Total readings (vol, pos)
//Vars for smoothing for voltage (-)
int     readings_vol_minus[numReadings] = {0};                                    //Definition of the Array (vol, neg)
int     readIndex_vol_minus = 0;                                                  //Index reading (vol, neg)
long    total_vol_minus = 0;                                                      //Total readings (vol, neg)

//Definition of the mapping function for mapping the voltage of the plus and minus pin
float flmap(float x, int maxVoltage, int maxAnalog) {                             //general formulation of the mapping function, expects a int value x, which is the analog signal and a maximum possible voltage as well as maximum possible analog signal
  return ((x * maxVoltage) / maxAnalog) + 0.01;                                   //returns the formula "INPUTSIGNAL x 5V / 1023 plus a shifting constant of 0.3
}

//Definition of the printing commands to print them all straight, printing of analog signals, difference as well as voltage signals and difference
void printValues() {
  Serial.println("ANA pluspin:      " + String(vol_plus_analog) + "     ANA minuspin:      " + String(vol_minus_analog) + "         ANA difference:    " + String(difference_analog));    //command to print the analog signal to the debug screen
  Serial.println("voltage_pluspin:  " + String(vol_plus_fin, 4) + "  voltage_minuspin:  " + String(vol_minus_fin, 4) + "    Vol difference:    " + String(difference_fin, 4) + "\n");     //command to print the (mapped) voltage signals to the screen
}

//DEFINING THE SUBROUTINES FOR THE Smoothing FUNCTIONS
unsigned long smooth(int pin, long &total, int *readings, int &readIndex) {                                           //beginn of smoothing function
  total = total - readings[readIndex];                                                                                //subtract the last reading
  readings[readIndex] = analogRead(pin);                                                                              //read from the sensor
  total = total + readings[readIndex];                                                                                //add the reading to the total
  readIndex = readIndex + 1;                                                                                          //advance to the next position in the array
  if (readIndex >= numReadings) {                                                                                     //if the Indexreading reaches the maximum amount of reading
    readIndex = 0;                                                                                                  //set the readings to zero and go back to the beginning
  }
  return total / numReadings;                                                                                         //calculate the average
}                                                                                                                     //end of smoothing function definition

void setup() {                                                                                                        //beginn of void setup
  pinMode (output_control, OUTPUT);                                                                                   //setting the variable output_control as an ouput pin
  digitalWrite(output_control, LOW);                                                                                  // Turn OFF the switch respectivley cleans the switch
  Serial.begin(9600);                                                                                                 //communication with 9600 bauds
  tini =  millis();                                                                                                   //refreshs the initial time
}                                                                                                                   //end of void setup

void loop() {                                                                                                         //beginning of void loop
  tact = millis();                                                                                              //defines the actual time in milli seconds
  switch (state) {                                                                                                    //beginning of the switch case definitions
    case CIRCUITCLOSED:                                                                                               //beginn of the definition for the FSM state CIRCUITOPENED
      digitalWrite(output_control, HIGH);                                                                                //opens the switch
      trel = tact - tini;                                                                                               //calculates the relative time with the help of the actual time and the initial time
         vol_plus_analog = smooth(input_pin_vol_plus, total_vol_plus, readings_vol_plus, readIndex_vol_plus);              //calls the smoothing function for the analog plus voltage with all necessary plus voltage values
      vol_minus_analog = smooth(input_pin_vol_minus, total_vol_minus, readings_vol_minus, readIndex_vol_minus);         //calls the smoothing function for the analog minus voltage with all necessary minus voltage values
      if (trel >= tsam) {                                                                                       //if the relative time is bigger than the sampeling time
        vol_plus_fin = flmap(vol_plus_analog, vol_max, analog_max);                                                   //command to convert the analog signal of the input plus voltage towards the plus voltage signal
        vol_minus_fin = flmap(vol_minus_analog, vol_max, analog_max);                                                 //command to convert the analog signal of the input minus voltage towards the minus voltage signal
        difference_analog = vol_plus_analog - vol_minus_analog;                                                       //calculates the difference of the analog singals
        difference_fin = vol_plus_fin - vol_minus_fin;                                                                //calculates the difference of the voltage signals
        Serial.println("CLOSED");                                                                                     //prints "CLOSED" to the debug screen
        printValues();                                                                                                //calls the printing command
      }
      trel = tact - tini;                                                                                             //calculates the relative time with the help of the actual and the inital time
      if (trel >= TS) {                                                                                                 //if the relative time is bigger than TS, the time for one state, 10s then
        state = CIRCUITOPENED;                                                                                          //switch to the state CIRCUITCLOSED
        tini  = millis();                                                                                               //refresh the time, because it is needed in the other case as well
      }
      break;                                                                                                          //end of the definition for the FSM state CIRCUITOPENED
    case CIRCUITOPENED:                                                                                             //beginning of the definition for the FSM state CIRCUITCLOSED
      digitalWrite(output_control, LOW);                                                                                //closes the switch
      trel = tact - tini;                                                                                               //calculates the relative time with the help of the actual time and the initial time
      vol_plus_analog = smooth(input_pin_vol_plus, total_vol_plus, readings_vol_plus, readIndex_vol_plus);              //calls the smoothing function for the analog plus voltage with all necessary plus voltage values
      vol_minus_analog = smooth(input_pin_vol_minus, total_vol_minus, readings_vol_minus, readIndex_vol_minus);         //calls the smoothing function for the analog minus voltage with all necessary minus voltage values
      if (trel >= tsam) {                                                                                             //if the relative time is bigger than the sampeling time
        vol_plus_fin = flmap(vol_plus_analog, vol_max, analog_max);                                                     //command to convert the analog signal of the input plus voltage towards the plus voltage signal
        vol_minus_fin = flmap(vol_minus_analog, vol_max, analog_max);                                                   //command to convert the analog signal of the input minus voltage towards the minus voltage signal
        difference_analog = vol_plus_analog - vol_minus_analog;                                                         //calculates the difference of the analog singals
        difference_fin = vol_plus_fin - vol_minus_fin;                                                                  //calculates the difference of the voltage signals
        Serial.println("OPENED");                                                                                       //prints "OPENED" to the debug screen
        printValues();                                                                                                  //calls the printing command
      }
      trel = tact - tini;                                                                                             //calculates the relative time with the help of the actual and the inital time
      if (trel >= TS) {                                                                                                //if the relative time is bigger than TS, the time for one stae, 10s then
        state = CIRCUITCLOSED;                                                                                         //switch to state CIRCUITOPENED
        tini  = millis();                                                                                              //refresh the time, because it is needed in the other case as well
      }
      break;                                                                                                          //end of the definition for the FSM state CIRCUITCLOSED
  }                                                                                                               //end of switch case definitions
}                                                                                                                    //end of void loop