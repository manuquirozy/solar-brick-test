#include <Arduino.h>
//Program to check the WCS1800 current sensor with smoothing
//inlcudes the curve shift operation as well
//works on Nov 26th

//pin definiton
#define input_pin_cur A3                                          //analog input pin for the current sensor connected to Analog input signal pin number 3
#define output_control 7                                         //control pin for the switch, connected to the digital output pin number 10

//variables for current capturing
float cur_analog  = 0;                                            //variable to store the input analog signal
float cur_fin     = 0;                                            //variable to store the current sensor signal as the actual current as calculated
float shift_curve = 0;                                            //variable to introduce the shift of the curve

//variables for mapping function
int vol_max = 5.020;                                              //5V reference from the Arduino
int analog_max = 1023.0;                                          //max possible Analog, the minima is 0 and the maximum analog number is 1023

//Definition of the constants for reading
const int numReadings    = 40;                                     //Describes the number of readings can be increased to stabilize the smoothing function, so right now the array is 80
const unsigned long tsam = 1000;                                   //sampling time to take measurements, which at the moment is 1s

//variables for current smoothing
int readings_cur[numReadings] = {0};                              //Definition of the array for current
int readIndex_cur             = 0;                                //Index reading (cur)
long total_cur                = 0;                                //Total readings(cur)

//Timing variables
unsigned long tini = 0;                                            //Initial timing variable
unsigned long tact = 0;                                            //Actual timing variable
unsigned long trel = 0;                                            //Relative timing variable
unsigned long ttotal = 0;
unsigned long t_shift_curve = 10000;

void printValues(){                                               //begin of printing command
  Serial.print("ANA curpin:      " + String(cur_analog));         //command to print the analog signal to the debug screen
  Serial.print("   Current(A):      " + String(cur_fin,4) + "\n");//command to print the (mapped) current signals to the screen
}                                                                 //end of prinining command

//DEFINING THE SUBROUTINES FOR THE Smoothing FUNCTIONS
unsigned long smooth(int pin, long &total, int *readings, int &readIndex) {
  total = total - readings[readIndex];                            //subtract the last reading
  readings[readIndex] = analogRead(pin);                          //read from the sensor
  total = total + readings[readIndex];                            //add the reading to the total
  readIndex = readIndex + 1;                                      //advance to the next position in the array         
  if (readIndex >= numReadings) {                                 //begin of if input, if the end of the array is reached
    readIndex = 0;                                                //then wrap around to the beginning
  }                                                               //end of if input
  return total / numReadings;                                     //calculate the average
}

void setup() {                                                    //beginn of void set up
  pinMode (output_control, OUTPUT);                               //setting the variable output_control as an ouput pin
  digitalWrite(output_control, LOW);                              //physical output cleaning respectively turn off the switch
  Serial.begin(9600);                                             //begin serial communication with the computer here (via USB)
  tini = millis();                                                //introduce the intial timing
}                                                                 //end of void setup

void loop() {                                                                     //beginn of void loop
    ttotal = millis();                                                            //introduce a total constant running time
    if ( ttotal <= t_shift_curve) {                                               //begin of if - when the total time is smaller than the time t_shift_curve
      digitalWrite (output_control, HIGH);                                        //turn on the swith
      tact = millis();                                                            //introduce the actual timing
      cur_analog = smooth(input_pin_cur, total_cur, readings_cur, readIndex_cur); //calling the smoothing function for the current with parameters for the current
      trel = tact - tini;                                                         //calculate the relative timing with the help of the actual and initial timing
        if (trel >= tsam) {                                                       //begin of if - if the relative timing is smaller than the sampling timing, then do
          shift_curve = 0.0742 * cur_analog;                                      //the calculation for transfering the analog signal into the current signal
          Serial.print(cur_analog);                                               //constantly print the analog signal to the debug screen
          Serial.print("Shift curve CONSTANT:   ");                               //print "Shift Curve Constant" to the debug screen
          Serial.println(shift_curve);                                            //print the shift curve constant to the debug screen
          }                                                                       //end of if 
      }                                                                           //end of if
   else {                                                                         //start of else
    digitalWrite(output_control, HIGH);                                           //closes the ciruit
    tact = millis();                                                              //calculate the actual time
    cur_analog = smooth(input_pin_cur, total_cur, readings_cur, readIndex_cur);   //calling the smoothing function for the current
    trel = tact - tini;                                                           //calculates the relative timing with the help of the actual time and the initial time
      if (trel >= tsam) {                                                         //begin of if - if the relative timing is smaller than the sampling timing, then do
        cur_fin = 0.0742 * cur_analog -shift_curve +0.00;                         //the calculation for transfering the analog signal into the current signal
        printValues();                                                            //calling the printing function
        }                                                                         //end of if
    }                                                                             //end of else
}                                                                                 //end of void loop