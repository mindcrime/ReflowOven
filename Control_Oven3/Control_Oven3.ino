#include <Adafruit_MAX31856.h>
#include <string.h>


#define DRDY_PIN 5

int OVEN_ELEMENT_PIN = 8;
int DOOR_ACTUATOR_IN_PIN = 7;
int DOOR_ACTUATOR_OUT_PIN = 6;

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);


boolean profileStopped = false;
long startMillis = 0;
boolean ovenOn = false;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(OVEN_ELEMENT_PIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("MAX31856 thermocouple test");

  pinMode(DRDY_PIN, INPUT);

  if (!maxthermo.begin()) {
    Serial.println("Could not initialize thermocouple.");
    while (1) delay(10);
  }

  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);

  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType() ) {
    case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
    case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
    case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
    case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
    case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
    case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
    case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
    case MAX31856_VMODE_G32: Serial.println("Voltage x8 Gain mode"); break;
    default: Serial.println("Unknown"); break;
  }

  maxthermo.setConversionMode(MAX31856_CONTINUOUS);
}

// the loop function runs over and over again forever
void loop() {

  // The DRDY output goes low when a new conversion result is available
  int count = 0;
  while (digitalRead(DRDY_PIN)) {
    if (count++ > 200) {
      count = 0;
      // Serial.print(".");
    }
  }

  float temp = maxthermo.readThermocoupleTemperature();

  if( startMillis == 0 )
  {
    startMillis = millis();  
    Serial.println( "Turning OVEN on" );
    digitalWrite(OVEN_ELEMENT_PIN, HIGH);   // turn the OVEN on (HIGH is the voltage level)
    ovenOn = true;
  }
  
  
  delay(2000);
  
  long elapsedMillis = millis() - startMillis;
  Serial.print( elapsedMillis/1000 ); Serial.print( "," ); Serial.println( temp );  

  if( elapsedMillis > 360000 && ovenOn )
  {
    Serial.println( "Turning OVEN off" );
    digitalWrite(OVEN_ELEMENT_PIN, LOW);   // turn the OVEN off (LOW is the voltage level)
    ovenOn = false;
  }
  
}
