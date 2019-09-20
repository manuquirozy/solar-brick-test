//Program to check the ACS711 current sensor without smoothing and without FSM

//include libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>                                           //library necessary for the SD cards
#include <RTClib.h>

// pin definiton
#define input_pin_vol_plus A0                             //Defining the analog pin number 5 to be for the plus cable of the attached source
#define input_pin_vol_minus A1                            //Defining the analog pin number 5 to be for the minus cable of the attached source
#define input_pin_cur A3                                  //analog input pin for the current sensor
#define output_control 7                                  //the output control decides whether the circuit is open or close

// FSM machines definition
#define CIRCUITOPENED 1                                   //FSM State 1, when the circuit is opened
#define CIRCUITCLOSED 2                                   //FSM State 2, when the circuit is closed

//variables for the RTC
RTC_DS1307 RTC;                                           //defining the clock
File dataFile;                                            //creates the data file
DateTime now;

//variables for current capturing
float cur_analog = 0; //variable to store the input analogue signal
float voltaje = 0;
float cur_fin_ = 0; //variable to store the current sensor signal as the actual current as calculated
float cur_fin = 0; //variable to store the current sensor signal as the actual current as calculated
float corriente = 0;
//variable for storing voltages
int   vol_plus_analog;                                    //Stores the analog signal of the plus cable
int   vol_minus_analog;                                   //Stores the analog signal of the minus cable
int   difference_analog;
float vol_plus_fin;                                       //Stores the analog signal of the plus calculated (mapped) voltage
float vol_minus_fin;                                      //Stores the analog signal of the minus calculated (mapped) voltage
float difference_fin;
float ResolucionArduino = 1023;
float MaxCorrienteSensor = 35; // WCS1800 Hall effect Current Sensor
float BitCeroV = 523.5; // 503.951 (AWS1800)
float I = 0;
float V = 0;

//variable for the SD card
const int chipSelect = 10;                                //Chip Select pin is tied to pin 8 on the SparkFun SD Card Shield
const unsigned long tsam = 1000;

// current state variable declaration
unsigned int state = CIRCUITOPENED;

//Timing vars
unsigned long tini = 0;                                   //introducing a variable for the intial time
unsigned long tact = 0;                                   //introducing a variable for the actual time
unsigned long trel = 0;                                   //introducing a variable for the relative time
unsigned long TS  = 5000;                                 //timing variable for one status of the FSM

//Definition of printing command
void printValues() {                                                          //begin of the print function declaration
  Serial.println("ANA: " + String(cur_analog, 3) + "   (A): " + String(I, 3) + "\n");
  Serial.println("V:    " + String(V, 3) + "\n");
}

void setup() { //begin of void set up
  Serial.begin(9600); //begin serial communication with the computer here (via USB)
  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


  state = CIRCUITOPENED;
  pinMode(chipSelect, OUTPUT);
  pinMode (output_control, OUTPUT);
  digitalWrite(output_control, LOW);



  Serial.print("Initializing SD card...");
  tini =  millis();
  if (!SD.begin(chipSelect)) {                                                                                        //begin if SD card is not present then
    Serial.println("Card failed, or not present");                                                                    //print that the SD card failed
    return;                                                                                                           //don't do anything more
  }                                                                                                                   //end of if case
  Serial.println("card initialized.");

  File dataFile = SD.open("datalog.txt", FILE_WRITE);                                                              //opens or intializies the file, dependent if that file already exits or not
  if (dataFile) {
    dataFile.print("Estatus" );
    dataFile.print("\t");
    dataFile.print("Corriente (A)" );
    dataFile.print("\t");
    dataFile.print("Diferencia de voltaje (V):");
    dataFile.print("\t");
    dataFile.print("Tiempo:" );
    dataFile.print("\t");
    dataFile.print("Fecha:" );  //
    dataFile.close();                                                                                           //properly close the data file
  }                                                                                                             //end of data file input
  else {                                                                                                        //begin of if data file does not exist then
    Serial.println("error printing to data file");                                                              //print an error line to the debug screen
  }

} //end of void setup

void loop() { //begin of void loop
  tact = millis();
  switch (state) {
    case CIRCUITCLOSED:

      digitalWrite (output_control, HIGH);
//      delay(10000);
      trel = tact - tini;
      if (trel >= tsam) {
        Serial.println("\nCC\n");
        I = get_corriente(200);
        V = get_voltaje (200);

        printValues();
        DateTime now = RTC.now();
//        Serial.println("Tiempo:                    " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  ");
//        Serial.println("Fecha:                    " + String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC));

        delay(500);

        File dataFile = SD.open("datalog.txt", FILE_WRITE);                                                              //opens or intializies the file, dependent if that file already exits or not
        if (dataFile) {                                                                                               //begin if the data file exists then
          DateTime now = RTC.now();
          dataFile.print("Corto Circuito");
          dataFile.print("\t");
          dataFile.print(String(I, 3)); // "Corriente (A):"
          dataFile.print("\t");
          dataFile.print(String(V, 3)); // "Diferencia de voltaje (V):"
          dataFile.print("\t");
          dataFile.print(String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  "); // "Tiempo:"
          dataFile.print("\t");
          dataFile.print(String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC));  // "Fecha:"
          dataFile.close();                                                                                           //properly close the data file
        }                                                                                                             //end of data file input
        else {                                                                                                        //begin of if data file does not exist then
          Serial.println("error printing to data file");                                                              //print an error line to the debug screen
        }
      }
      if (trel >= TS) {                                                                                                  //begin of if clause, for the transition questions
        state = CIRCUITOPENED;                                                                                            //switch to state CIRCUITOPENED when the if-statement is fulfilled
        tini  = millis();                                                                                                 //refresh the intial timing
      }                                                                                                                  //end of if clause, for the transition questions
      break;

    case CIRCUITOPENED:

      digitalWrite(output_control, LOW);
//      delay(10000);
      trel = tact - tini;
      if (trel >= tsam) {
        Serial.println("\nNCC\n");
        I = get_corriente(200);
        V = get_voltaje (200);

        printValues();
        DateTime now = RTC.now();
//        Serial.println("Tiempo:                    " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  ");
//        Serial.println("Fecha:                    " + String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC));

        delay(500);

        File dataFile = SD.open("datalog.txt", FILE_WRITE);                                                           //opens or intializies the file, dependent if that file already exits or not
        if (dataFile) {                                                                                               //begin if the data file exists then
          DateTime now = RTC.now();
          dataFile.print("No Corto Circuito");
          dataFile.println("\t");
          dataFile.print(String(I, 3)); // "Corriente (A):"
          dataFile.println("\t");
          dataFile.println(String(V, 3)); // "Diferencia de voltaje (V):"
          dataFile.println("\t");
          dataFile.println(String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  "); //"Tiempo:"
          dataFile.println("\t");
          dataFile.println(String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC)); // "Fecha:"
          dataFile.close();                                                                                           //properly close the data file
        }                                                                                                             //end of data file input
        else {                                                                                                        //begin of if data file does not exist then
          Serial.println("error printing to data file");                                                              //print an error line to the debug screen
        }
      }
      if (trel >= TS) {                                                                                                  //begin of if clause, for the transition questions
        state = CIRCUITCLOSED;                                                                                            //switch to state CIRCUITOPENED when the if-statement is fulfilled
        tini  = millis();                                                                                                 //refresh the intial timing
      }                                                                                                                  //end of if clause, for the transition questions
      break;
  }
}

float get_corriente(int n_muestras)
{
  float corriente = 0;

  for (int i = 0; i < n_muestras; i++)
  {
    cur_analog = analogRead(input_pin_cur);
    corriente = corriente + ((cur_analog * 0.00489 - BitCeroV * 0.00489) / 0.07214) ; //Ecuación  para obtener la corriente 0.07214 (AWS1800) 0.13957 (ACS711
  }
  corriente = corriente / n_muestras;
  return (corriente);
}

float get_voltaje(int n_muestras)
{
  float voltaje = 0;

  for (int i = 0; i < n_muestras; i++)
  {
    vol_plus_analog = analogRead(input_pin_vol_plus);
    //    vol_minus_analog = analogRead(input_pin_vol_minus);
    //    difference_analog = vol_plus_analog - vol_minus_analog;

    voltaje = voltaje + (vol_plus_analog) * 0.00489;
  }
  voltaje = voltaje / n_muestras;
  return (voltaje);
}













//  cur_analog = analogRead(input_pin_cur); //reading the input_pin_cur and saving it into the variable cur_analog  (5 * cur_analog) / 1023; //
//  Vcur_analog = (5.0 * cur_analog) / 1023.0; //
//  //  cur_fin = (Vcur_analog - 2.5) / 0.0631;
//  //  cur_fin = (70 / ResolucionArduino) * cur_analog - MaxCorrienteSensor;//
//  cur_fin =  (cur_analog * 0.00489 - BitCeroV * 0.00489) / 0.076; // Es 0.0644 (El de verde BitCeroV = 504)
//
//  Serial.println(Vcur_analog);
//  delay(200);
//  if (cur_fin < 0) {
//    Serial.println("cur_fin < 0");
//    Serial.println(cur_fin);
//    cur_fin = 0;
//  }
//  printValues();













