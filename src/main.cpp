#include <Arduino.h>

//Program to check the WCS1800 current sensor with Finite State Machines
//includes smoothing
//inlcudes curve shift variable

// pin definiton
#define input_pin_cur A3                                  //analog input pin for the current sensor
#define output_control 7                                 //the output control decides whether the circuit is open or close

// FSM machines definition
#define CIRCUITOPENED 1                                   //FSM State 1, when the circuit is opened
#define CIRCUITCLOSED 2                                   //FSM State 2, when the circuit is closed

// variables for current capturing
float cur_analog  = 0;                                    //variable to store the current as analog signal
float cur_fin     = 0;                                    //variable to store the current as current signal
float shift_curve = 0;                                    //constant for the calculations

//Definition of the contants for reading for both, voltage and current and curve constant
const int maxsamples     = 50;
const int numReadings    = 40;                            //Describes the number of readings can be increased to stabilize the smoothing function, so right now the array is 80
const unsigned long tsam = 1000;                          //Sample time for taking readings

// variables for current smoothing
int     readings_cur[numReadings] = {0};                  //introducing the array for the measurements of the current smoothing
int     readIndex_cur = 0;                                //reading the index of the current smoothing operation
long    total_cur = 0;                                    //accumulating the total current for the smoothing operation 

// variables for shift curve smoothing
float samples [maxsamples] = {0};
float numerator            = 0;

// current state variable declaration
unsigned int state = CIRCUITOPENED;                       //this variable saves the current status which FSM is activated at the moment, the intial one is the circuit opened

//Timing vars
unsigned long tini = 0;                                   //introducing a variable for the intial time
unsigned long tact = 0;                                   //introducing a variable for the actual time
unsigned long trel = 0;                                   //introducing a variable for the relative time
unsigned long ttotal = 0;                                 //introducing a variable for the total time
unsigned long t_shift_curve = 5000;                      //introducing a variable for the intial start of the program to calculate the variable shift_curve
unsigned long TS  = 5000;                                //timing variable for one status of the FSM


// DEFINING THE SUBROUTINES FOR THE SMOOTHING FUNCTIONS
unsigned long smooth(int pin, long &total, int *readings, int &readIndex) {   //begin of the smooth function declaration
  total = total - readings[readIndex];                                        //subtract the last reading
  readings[readIndex] = analogRead(pin);                                      //read from the sensor
  total = total + readings[readIndex];                                        //add the reading to the total
  readIndex = readIndex + 1;                                                  //advance to the next position in the array         
 if (readIndex >= numReadings) {                                              //begin of the if clause -> if it is at the end of the array then wrap around to the beginning
      readIndex = 0;                                                          //set the index to zero
    }                                                                         //end of the if clause
  return total / numReadings;                                                 //calculate the average
}                                                                             //end of the smooth function declaration

void printValues() {                                                          //begin of the print function declaration                                                                                                                    
  Serial.println("ANA: " + String(cur_analog, 4) + " Cur (mA): " + String(cur_fin,4) + "\n");
}                                                                             //end of the print fucntion declaration

void setup() {                                                                //begin of the void setup
  pinMode (output_control, OUTPUT);                                           //setting the variable output_control as an ouput pin
  digitalWrite(output_control, LOW);                                          //Turn OFF the switch respectivley physical output cleaning
  Serial.begin(9600);                                                         //begin serial communication with the computer here (via USB)
  tini = millis();                                                            //introduce the initial time
}                                                                             //end of the void setup

void loop() {                                                                 //start of void loop
   ttotal = millis();                                                         //introduce a total constant running time
  if ( ttotal <= t_shift_curve) {                                             //if the total time is less then the total time do
    digitalWrite (output_control, HIGH);                                      //closes the ciruit
    tact = millis();                                                          //introducing the actual time
    cur_analog = smooth(input_pin_cur, total_cur, readings_cur, readIndex_cur);  //calling the smoothing function for the current
    trel = tact - tini;                                                       //calculates the relative time from the actual and the intial time
     if (trel >= tsam) {                                                      //if the relative time is equal or higher than the sampling time 
    shift_curve = 0.0742 * cur_analog;                                        //calculate the shift_curve variable
    Serial.print(cur_analog);                                                 //prints the analog signal to the debug screen                                                  
    Serial.print("Shift curve CONSTANT:   ");                                 //prints the "Shift curve CONSTANT" to the debug screen
    Serial.println(shift_curve);                                              //prints the value of the constant to the debug screen
     }                                                                        //end of the if clause
  }                                                                           //end of the if clause
  else {                                                                      //when the constant shift_curve is calculated switch to the second part of the void loop which is a FSM code                       
   tact = millis();                                                           //calculate the actual execution time (if the FSM requires timing)
   switch(state) {                                                            //start of FSM switch state
   case CIRCUITCLOSED:                                                        //declaration of the first FSM state, which is the closed circuit
   digitalWrite(output_control, HIGH);                                        //Turn ON the switch
   cur_analog = smooth(input_pin_cur, total_cur, readings_cur, readIndex_cur);//calling the smoothing function for the analog signal of the current
   trel = tact-tini;                                                          //calulates the relative time with the help of the actual and the intial time
   if(trel>=tsam) {                                                           //begin of if the relative time is equal or higher then the sampling time, then do
    cur_fin = 0.0742 * cur_analog - shift_curve;                              //the calculation of the transfer between analog signal and current signal
    Serial.println("CLOSED");                                                 //Prints "Closed" to the debug screen
    printValues();                                                            //calls the printing function
    }                                                                         //end of if clause
   if (trel >= TS) {                                                          //begin of if clause, for the transition questions
    state = CIRCUITOPENED;                                                    //switch to state CIRCUITOPENED when the if-statement is fulfilled
    tini  = millis();                                                         //refresh the intial timing
   }                                                                          //end of if clause, for the transition questions
   break;                                                                     //end of FSM 1, reference by the "break" command
   case CIRCUITOPENED:                                                        //declaration of the second FSM case, which is an opened circuit
   digitalWrite(output_control, LOW);                                         //Turn OFF the switch
   cur_analog = smooth(input_pin_cur, total_cur, readings_cur, readIndex_cur);//calling the smoothing function for the analog signal of the current
   trel = tact-tini;                                                          //calculates / refreshes the relative time witht the help of the actual time and the intial time
   if(trel>=tsam) {                                                           //if the initial time is bigger then the sampling time then do
     cur_fin = 0.0742 * cur_analog - shift_curve;                             //the calculation of the transfer betweent analog signal and current signal
     Serial.println("OPEN");                                                  //prints "open" to the debug screen
     printValues();                                                           //calls the printing function command  
   }                                                                          //end of the if clause
   if (trel >= TS) {                                                          //begin of the if clause for the transition questions
    state = CIRCUITCLOSED;                                                    //when the if clause if fulfilled change (back) to the circuit closed FSM
    tini  = millis();                                                         //refresh the time
   }                                                                          //end of transition questions
   break;                                                                     //end of decclaration for the FSM 2 the opened circuit FSM
   }                                                                          //end of switch state FSM
  }                                                                           //end of the else clause
}                                                                             //end of the void loop