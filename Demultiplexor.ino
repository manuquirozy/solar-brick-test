//const int Enable = 11; // Se lleva tierra

void setup()
{
  Serial.begin(9600);
  for (int i = 8 ; i < 12 ; i++)
    pinMode(i , OUTPUT);
}

void loop()
{
  for (int j = 0; j < 5; j++)
  {

    unsigned int A, B, k;
    k = 1;//j;    // Selecciona la puerta a pasar
    A = k & B00000001 ;  digitalWrite ( 10 , 0) ; //Solo quiero el ultimo bit
    B = ( k >> 1 ) & B00000001 ; digitalWrite ( 11 , 0) ;
    analogWrite(8,0); // X
    analogWrite(9, 0); // Y
    delay (2000);
    
  }
}

  // Cuando el relé está a 5V se deconectan (inicialmemnte en comunmente abierto), con 0V se conectan y el led rojo se activa
