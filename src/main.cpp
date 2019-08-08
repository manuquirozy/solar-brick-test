#include <Arduino.h>
//Program to check the WCS1800 current sensor with Finite State Machines

// pin definiton
#define input_pin_cur A3            //analog input pin for the current sensor
#define output_control 7            //the output control decides whether the circuit is open or close

// FSM machines definition
#define CIRCUITOPENED 1
#define CIRCUITCLOSED 2

// variables for current capturing
float cur_analog  = 0;
float cur_fin     = 0;
float shift_curve = 0;                                            //variable to introduce the shift of the curve

unsigned int state = CIRCUITOPENED;                               //current state variable declaration

//Timing vars
unsigned long tini = 0;
unsigned long tact = 0;
unsigned long trel = 0;
unsigned long TS   = 5000;

//time variable for the shiftcurve value
unsigned long ttotal = 0;                                         //total time counter
unsigned long t_shift_curve = 2500;                               //time for calculating the c in the mx+c linear approximiation of the analog-current mapping curve

void printValues() {
  Serial.println("ANA: " + String(cur_analog, 4) +  " Cur (A): " + String(cur_fin, 4) + "\n");
}

void setup() {
  pinMode (output_control, OUTPUT);                                //setting the variable output_control as an ouput pin
  digitalWrite(output_control, LOW);                               //physical output cleaning respectively turn off the switch
  Serial.begin(9600);                                              //begin serial communication with the computer here (via USB)
}

void loop() {
  ttotal = millis();                                               //introduce a total constant running time
  if ( ttotal <= t_shift_curve) {                                  //if the total time counter is smaller than the time for calculating the curve contant
    digitalWrite (output_control, HIGH);                           //opens the circuit
    cur_analog = analogRead(input_pin_cur);                        //analog singal is read and saved in the variable cur_analog
    shift_curve = 0.0742 * cur_analog;                             //calculates the c in the mx+c linear curve and saves it into the varialbe shift_curve
    Serial.print(cur_analog);                                      //prints the analog signal to the debug screen
    Serial.print("Shift curve CONSTANT:   ");                      //prints "Shift curve CONSTANT:" to the debug screen
    Serial.println(shift_curve);                                   //print the shift_curve signal to the debug scren
    //delay (100);
  }                                                                //end of if question
  else {                                                           //else loop
    tact = millis();
    switch (state) {
      case CIRCUITCLOSED:
        digitalWrite(output_control, HIGH);                              // Turn ON the switch
        cur_analog = analogRead(input_pin_cur);
        cur_fin = 0.0742 * cur_analog - shift_curve;                              //the calculation of the transfer between analog signal and current signal
        Serial.println("CLOSED");
        printValues();
        trel = tact - tini;
        if (trel >= TS) {
          state = CIRCUITOPENED;
          tini  = millis();
        }
        break;
      case CIRCUITOPENED:
        digitalWrite(output_control, LOW);        // Turn OFF the switch
        cur_analog = analogRead(input_pin_cur);
        cur_fin = 0.0742 * cur_analog - shift_curve;                              //the calculation of the transfer between analog signal and current signal
        Serial.println("OPENED");
        printValues();
        trel = tact - tini;
        if (trel >= TS) {
          state = CIRCUITCLOSED;
          tini  = millis();
        }
        break;
    }
  }
}