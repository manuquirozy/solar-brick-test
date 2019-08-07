#include <Arduino.h>
//Program to check the WCS1800 current sensor without smoothing
//without FSM
//BUT with the shift_curve variable
//Works 20th of NOV
//update 26th of NOV
//works! still on 27th of NOV
//updated on NOV 29th for averagin the shift constant

//pin definiton
#define input_pin_cur A3                                          //analog input pin for the current sensor connected to Analog input signal pin number 3
#define output_control 7                                          //control pin for the switch, connected to the digital output pin number 7

//variables for current capturing
float cur_analog  = 0;                                            //variable to store the input analog signal
float cur_fin     = 0;                                            //variable to store the current sensor signal as the actual current as calculated
float shift_curve = 0;                                            //variable to introduce the shift of the curve

//time variable for the shiftcurve value

float numerator = 0;                                              //counter variable for the shift_curve averaging evaluation
float placeholder = 0;                                            //counter variable for the shift_curve averaging evaluation
float numberOfReadings = 10000;

//Definition of printing command
void printValues() {                                               //start of printing command
  Serial.println("ANA: " + String(cur_analog, 4) + " Cur (A): " + String(cur_fin, 4) + "\n");
}                                                                  //end of printing command

float calculateoff (int pin, float &counter, float &spaceholder, int maxiterating) {
  for (int i = 0; i <= maxiterating; i++) {                        //condition of the for loop, for the counting variable smaller than the numberofReadings do
    spaceholder = 0.0742 * analogRead(pin);                        //the mathematical calculation for the shift_curve value, place holder in the function is called spaceholder
    counter = counter + spaceholder;                               //counts everything together                                    
  }
  return counter / maxiterating;
}

void setup() {                                                      //beginn of void set up
  pinMode (output_control, OUTPUT);                                 //setting the variable output_control as an ouput pin
  digitalWrite(output_control, LOW);                                //physical output cleaning respectively turn off the switch
  digitalWrite(output_control, HIGH);                               //turn on the circuit for shift curve measurement
  shift_curve = calculateoff (input_pin_cur, numerator, placeholder, numberOfReadings);
  Serial.begin(9600);                                               //begin serial communication with the computer here (via USB)
  Serial.println(shift_curve);                                      //prints the shift_curve value to the debug screen
  delay(1000);
}                                                                   //end of void setup

void loop() {                                                       //beginn of void loop
  digitalWrite(output_control, HIGH);                               //closes the ciruit
  cur_analog = analogRead(input_pin_cur);                           //reading the input_pin_cur and saving it into the variable cur_analog
  cur_fin = 0.0742 * cur_analog - shift_curve;                      //converting the analog signal into a current signal
  printValues();                                                    //calling the printing function
  delay(100);                                                       //delay for easy reading
}                                                                   //end of void loop