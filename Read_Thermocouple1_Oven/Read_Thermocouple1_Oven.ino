// This example demonstrates continuous conversion mode using the
// DRDY pin to check for conversion completion.

#include <Adafruit_MAX31856.h>
#include <string.h>

#define DRDY_PIN 9

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);

void setup() {
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

void loop() {
  // The DRDY output goes low when a new conversion result is available
  int count = 0;
  while (digitalRead(DRDY_PIN)) {
    if (count++ > 200) {
      count = 0;
      Serial.print(".");
    }
  }

  float temp = maxthermo.readThermocoupleTemperature();
  String c = "C = ";
  c.concat( temp );
  
  String f = "F = ";
  f.concat( ( temp * (9.0f/5.0f) ) + 32.0f );

  char cCopy[c.length()+1];
  c.toCharArray(cCopy, c.length());
  cCopy[c.length()] = '\0';
  
  Serial.println( cCopy );

  char fCopy[f.length()];
  f.toCharArray(fCopy, f.length());
  fCopy[f.length()] = '\0';

  Serial.println( fCopy ); 
  
  delay( 1500 );
}
