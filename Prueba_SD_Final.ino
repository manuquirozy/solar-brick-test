#include <LowPower.h>

//Program to check the ACS711 current sensor without smoothing and without FSM

//include libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>                                           //library necessary for the SD cards
#include <RTClib.h>

// pin definiton
#define input_pin_vol A0                             //Defining the analog pin number 5 to be for the plus cable of the attached source
#define input_pin_cur A3                                  //analog input pin for the current sensor
#define output_control 6                                  //the output control decides whether the circuit is open or close

// FSM machines definition
#define CIRCUITOPENED 1                                   //FSM State 1, when the circuit is opened
#define CIRCUITCLOSED 2                                   //FSM State 2, when the circuit is closed

//variables for the RTC
RTC_DS1307 RTC;                                           //defining the clock
File dataFile;                                            //creates the data file
DateTime now;

//variables for current capturing
unsigned int A, B, k;
String NombreArchivo = " ";
float cur_analog = 0; //variable to store the input analogue signal
float voltaje = 0;
float cur_fin_ = 0; //variable to store the current sensor signal as the actual current as calculated
float cur_fin = 0; //variable to store the current sensor signal as the actual current as calculated
float corriente = 0;
int   vol_analog;                                    //Stores the analog signal of the plus cable
float ResolucionArduino = 1023;
float MaxCorrienteSensor = 35; // WCS1800 Hall effect Current Sensor
float BitCeroV = 0; // 503.951 (AWS1800)
float I = 0;
float V = 0;

//variable for the SD card
const int chipSelect = 10;                                //Chip Select pin is tied to pin 8 on the SparkFun SD Card Shield

// current state variable declaration
unsigned int state = CIRCUITOPENED;

unsigned long Contador = 0;

//Definition of printing command
void printValues() {                                                          //begin of the print function declaration
  Serial.println("(C): " + String(I, 3) + "\n");
  Serial.println("V:    " + String(V, 3) + "\n");
}

void setup() { //begin of void set up
  Serial.begin(9600); //begin serial communication with the computer here (via USB)

  for (int i = 8 ; i < 13 ; i++) // Para establecer los pines del 8-11 como salidas, ahí están A,B, X, Y (Selectores y salidas comunes del multiplexor CD4052)
    pinMode(i , OUTPUT);
  for (int j = 0 ; j < 4 ; j++)
  {
    k = j; // Selecciona la puerta a pasar en el multiplexor CD4052
    A = k & B00000001 ;  digitalWrite ( 10 , A) ; //Solo quiero el ultimo bit para el selector A
    B = ( k >> 1 ) & B00000001 ; digitalWrite ( 4 , B) ; //Solo quiero el ultimo bit para el selector B
    analogWrite(8, 1023); // X, Salidas común // Desconecte los relés
    analogWrite(9, 1023); // Y, Salida
  }

  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));


  state = CIRCUITOPENED;
  pinMode(chipSelect, OUTPUT);
  pinMode (output_control, OUTPUT);
  digitalWrite(output_control, LOW);


  float CalibradoC = CalibradoCorriente(10000, 5); //obtenemos voltaje del sensor(10000 muestras)
  Serial.print("Bits_i para sensor: ");
  Serial.println(CalibradoC , 3);

  BitCeroV = CalibradoC;

  Serial.print("Iniciando SD ...");
  if (!SD.begin(chipSelect)) {                                                                                        //begin if SD card is not present then
    Serial.println("No se pudo inicializar");                                                                    //print that the SD card failed
    return;                                                                                                           //don't do anything more
  }                                                                                                                   //end of if case
  Serial.println("Inicializacion exitosa");

  for (int j = 0 ; j < 4 ; j++)
  {
    if (j == 0)
    {
      NombreArchivo = "datalog0.txt";
    }
    else if (j == 1)
    {
      NombreArchivo = "datalog1.txt";
    }
    else if (j == 2)
    {
      NombreArchivo = "datalog2.txt";
    }
    else if (j == 3)
    {
      NombreArchivo = "datalog3.txt";
    }

    //    NombreArchivo = String("datalog_") + String(j) +  String(".txt");
    dataFile = SD.open(NombreArchivo, FILE_WRITE);                                                              //opens or intializies the file, dependent if that file already exits or not
    if (dataFile) {
      //      Serial.print("Escribiendo SD: ");
      dataFile.print("Estatus" );
      dataFile.print("\t");
      dataFile.print("Corriente (A)" );
      dataFile.print("\t");
      dataFile.print("Diferencia de voltaje (V):");
      dataFile.print("\t");
      dataFile.print("Tiempo:" );
      dataFile.print("\t");
      dataFile.println("Fecha:" );  //
      dataFile.close();
    }
    else {
      Serial.println("Error al abrir el archivo");
    }
  }

} //end of void setup

void loop() { //begin of void loop

  ///////////////// Estrutura FOR para que se comunte a cada LS, usando un multiplexor CD4052////////////////////
  for (int j = 0 ; j < 4 ; j++)
  {
    Serial.print("Para igual a "); Serial.println(j);
    k = j; // Selecciona la puerta a pasar en el multiplexor CD4052
    A = k & B00000001 ;  digitalWrite ( 10 , A) ; //Solo quiero el ultimo bit para el selector A
    B = ( k >> 1 ) & B00000001 ; digitalWrite ( 4 , B) ; //Solo quiero el ultimo bit para el selector B
    analogWrite(8, 0); // X, Salidas común
    analogWrite(9, 0); // Y, Salida común

    // Cuando el relé está a 5V se deconectan (inicialmemnte en comunmente abierto), con 0V se conectan y el led rojo se activa

    /////////////////////// Defino el nombre del archivo ///////////////////////////

    if (j == 0)
    {
      NombreArchivo = "datalog0.txt";
    }
    else if (j == 1)
    {
      NombreArchivo = "datalog1.txt";
    }
    else if (j == 2)
    {
      NombreArchivo = "datalog2.txt";
    }
    else if (j == 3)
    {
      NombreArchivo = "datalog3.txt";
    }
    //////////////////////////////////// NO CORTO///////////////////////////////////

    delay(1000);
    digitalWrite(output_control, LOW);
    delay(500);

    Serial.println("\nNo Corto\n");
    I = get_corriente(200);
    V = get_voltaje (200);

    printValues();
    DateTime now = RTC.now();
    delay(200);
    Serial.println(String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  ");
    Serial.println(String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC));
    dataFile = SD.open(NombreArchivo, FILE_WRITE);
    if (dataFile) {
      //      Serial.print("Escribiendo SD: ");
      DateTime now = RTC.now();
      dataFile.print("NCC"); // No Corto Circuito
      dataFile.print("\t");
      dataFile.print(String(I, 3)); // "Corriente (A):"
      dataFile.print("\t");
      dataFile.print(String(V, 3)); // "Diferencia de voltaje (V):"
      dataFile.print("\t");
      dataFile.print(String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  "); //"Tiempo:"
      dataFile.print("\t");
      dataFile.println(String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC)); // "Fecha:"
      dataFile.close();
    }
    else {
      Serial.println("Error al abrir el archivo");
    }

    //////////////////////////////////// CORTO ////////////////////////////////////
    //    k = j; // Selecciona la puerta a pasar en el multiplexor CD4052
    //    A = k & B00000001 ;  digitalWrite ( 10 , A) ; //Solo quiero el ultimo bit para el selector A
    //    B = ( k >> 1 ) & B00000001 ; digitalWrite ( 4 , B) ; //Solo quiero el ultimo bit para el selector B
    //    analogWrite(8, 0); // X, Salidas común
    //    analogWrite(9, 0); // Y, Salida común

    delay(1000);
    digitalWrite(output_control, HIGH);
    delay(500);

    Serial.println("\nCorto\n");
    I = get_corriente(200);
    V = get_voltaje (200);

    printValues();
    //DateTime now = RTC.now();
    delay(200);
    Serial.println(String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  ");
    Serial.println(String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC));
    dataFile = SD.open(NombreArchivo, FILE_WRITE);
    if (dataFile) {
      //      Serial.print("Escribiendo SD: ");
      DateTime now = RTC.now();
      dataFile.print("CC"); // Corto Circuito
      dataFile.print("\t");
      dataFile.print(String(I, 3)); // "Corriente (A):"
      dataFile.print("\t");
      dataFile.print(String(V, 3)); // "Diferencia de voltaje (V):"
      dataFile.print("\t");
      dataFile.print(String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) + "  "); // "Tiempo:"
      dataFile.print("\t");
      dataFile.println(String(now.year(), DEC) + "/" + String(now.month(), DEC)  + "/" + String(now.day(), DEC));  // "Fecha:"
      dataFile.close();
    }
    else {
      Serial.println("Error al abrir el archivo");
    }

    digitalWrite(output_control, LOW);
    delay(1000);
    k = j; // Selecciona la puerta a pasar en el multiplexor CD4052
    A = k & B00000001 ;  digitalWrite ( 10 , A) ; //Solo quiero el ultimo bit para el selector A
    B = ( k >> 1 ) & B00000001 ; digitalWrite ( 4 , B) ; //Solo quiero el ultimo bit para el selector B
    analogWrite(8, 1023); // X, Salidas común
    analogWrite(9, 1023); // Y, Salida común

    // Que el muestreo esté determinado por un N segundos
    //  tini =  millis();
    //  while (1)
    //  {
    //    tact =  millis();
    //    tres = tact - tini; // tres es la direferencia de tiempo entre lo que ha transcurrido y un actual (0?)
    //    if (tres >= 10000) // Muestreo de 5min
    //    {
    //      break;
    //    }
    //  }


    if (j == 3) { // Si llega al último ladrillo, que entre en modo bajo consumo
      Serial.println("Entramos a MBC");
      digitalWrite(output_control, LOW);
      break;
    }
  }

  delay(100);
  ////////////////7 A fuera del FOR que pasa midiendo por cada ladrillo V e I //////////////////
  for (int i = 0 ;  i  <  0.5 * 15 ; i++) // Ese 0.2 minutos *15 (60/4) iteraciones
  {
    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); // Se duerme el Arduino por 5 minutos
  }
  delay(500);
}

float get_corriente(int n_muestras)
{
  float corriente = 0;

  for (int i = 0; i < n_muestras; i++)
  {
    cur_analog = analogRead(input_pin_cur);
    corriente = corriente + ((cur_analog * 0.00489 - BitCeroV * 0.00489) / 0.727554); //Ecuación  para obtener la corriente 0.07214 (AWS1800) 0.13957 (ACS711 ( BitCeroV * 0.00489 = 2.60)
  }
  corriente = corriente / n_muestras;

  //  if (corriente < 0) {
  //    corriente = 0;
  //  }

  return (corriente);
}

float get_voltaje(int n_muestras)
{
  float voltaje = 0;

  for (int i = 0; i < n_muestras; i++)
  {
    vol_analog = analogRead(input_pin_vol);
    //    vol_minus_analog = analogRead(input_pin_vol_minus);
    //    difference_analog = vol_plus_analog - vol_minus_analog;

    voltaje = voltaje + (vol_analog) * 0.00489;
  }
  voltaje = voltaje / n_muestras;
  return (voltaje);
}

float CalibradoCorriente(int n_muestras, int n_medidas)
{
  float CorrienteCalib = 0;
  float CorrienteCalib_ = 0;
  for (int j = 0; j < n_medidas; j++ ) {
    Serial.println("Medida:" + String(j));

    for (int i = 0; i < n_muestras; i++)
    {
      CorrienteCalib = CorrienteCalib + analogRead(A3); // * (5.0 / 1023.0);
    }
    CorrienteCalib = CorrienteCalib / n_muestras;

    CorrienteCalib_ = CorrienteCalib_ + CorrienteCalib;
  }

  CorrienteCalib_ = CorrienteCalib_ / n_medidas;
  return (CorrienteCalib_);
}
