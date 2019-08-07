#include <Arduino.h>
//Program to check the WCS1800 current sensor without smoothing
//without FSM
//BUT with the shift_curve variable
//Works 20th of NOV
//update 26th of NOV
//works! still on 27th of NOV

//pin definiton
#define input_pin_cur A3                                          //analog input pin for the current sensor connected to Analog input signal pin number 3
#define output_control 7                                         //control pin for the switch, connected to the digital output pin number 7

//variables for current capturing
float cur_analog  = 0;                                            //variable to store the input analog signal
float cur_fin     = 0;                                            //variable to store the current sensor signal as the actual current as calculated
float shift_curve = 0;                                            //variable to introduce the shift of the curve

//time variable for the shiftcurve value
unsigned long ttotal = 0;                                         //total time counter
unsigned long t_shift_curve = 2500;                               //time for calculating the c in the mx+c linear approximiation of the analog-current mapping curve

//Definition of printing command
void printValues() {                                               //start of printing command
  Serial.println("ANA: " + String(cur_analog, 4) + " Cur (A): " + String(cur_fin,4) + "\n");
}                                                                  //end of printing command

void setup() {                                                     //beginn of void set up
  pinMode (output_control, OUTPUT);                                //setting the variable output_control as an ouput pin
  digitalWrite(output_control, LOW);                               //physical output cleaning respectively turn off the switch
  Serial.begin(9600);                                              //begin serial communication with the computer here (via USB)
}                                                                  //end of void setup

void loop() {                                                      //beginn of void loop
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
   digitalWrite(output_control, HIGH);                             //closes the ciruit                                                       
   cur_analog = analogRead(input_pin_cur);                         //reading the input_pin_cur and saving it into the variable cur_analog
   cur_fin = 0.0742 * cur_analog - shift_curve;                    //converting the analog signal into a current signal
      printValues();                                               //calling the printing function
      //delay(500);
  }                                                                //end of else loop
}                                                                  //end of void loop