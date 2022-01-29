#include <Adafruit_MAX31856.h>
#include <string.h>


// this is no longer accurate. Need to visually inspect and find out where DRDY really goes on the Arduino side...
#define DRDY_PIN 9

int OVEN_ELEMENT_PIN = 8;

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);

boolean reflowStarted = false;
boolean coolDownStarted = false;
boolean profileStarted = false;
boolean profileStopped = false;
long startMillis = 0;
boolean ovenOn = false;
float temp = 0;
float lastTemp = 0;


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
void loop() 
{
  if( profileStopped )
  {
    delay(5000);
    return;  
  }
  
  // The DRDY output goes low when a new conversion result is available
  int count = 0;
  while (digitalRead(DRDY_PIN)) {
    if (count++ > 200) {
      count = 0;
      // Serial.print(".");
    }
  }

  lastTemp = temp;
  temp = maxthermo.readThermocoupleTemperature();
  
  if( temp > 230.0 )
  {
    // ABORT, ABORT!!!!
    Serial.print( "TEMP: " ); Serial.print( temp ); Serial.println( " - MAX TEMP EXCEEDED. ABORT, ABORT!!!" );  
    digitalWrite(OVEN_ELEMENT_PIN, LOW);   // turn the oven OFF
    delay( 240000 );
    return;
  }

  if( !profileStarted && !profileStopped )
  {
    profileStarted = true;
    // turn oven on  
    startMillis = millis();  
    
  }
  else if( profileStarted )
  {

    long elapsedMillis = millis() - startMillis;
    Serial.print( elapsedMillis/1000 ); Serial.print( "," ); Serial.println( temp ); 
    
    if( temp < 148.0 && (elapsedMillis / 1000) < 150 )
    {
      // PREHEAT: come up to 150 degrees, max of 2.5 minutes (150 seconds)
      if( !ovenOn )
      {
        // we're in the preheat phase, leave the oven ON 
        Serial.println( "PREHEAT: Turning oven ON" );
        digitalWrite(OVEN_ELEMENT_PIN, HIGH);   // turn the oven ON
        ovenOn = true;
      }
       
    }
    else if( temp > 148 || (elapsedMillis / 1000) > 150 )
    {

        if( temp < 178 && (elapsedMillis / 1000) < 240 && !reflowStarted )
        {

            // SOAK: come up to 178, max of 4 minutes (combined with PREHEAT)
            if( ovenOn )
            {
              Serial.println( "SOAK: Turning oven OFF" );
              digitalWrite(OVEN_ELEMENT_PIN, LOW);   // turn the oven OFF
              // turn the oven completely OFF for this phase. We'll see how close that gets us relying on thermal interia  
              ovenOn = false;
            }

            if( temp <= lastTemp ) // if we've let the temperature drop at all
            {
              // give it a little bump
              Serial.println( "SOAK: Turning oven ON" );
              digitalWrite(OVEN_ELEMENT_PIN, HIGH);   // turn the oven ON
              delay( 5000 ); // a little 5 second blast of heat
              Serial.println( "SOAK: Turning oven OFF" );
              digitalWrite(OVEN_ELEMENT_PIN, LOW);   // turn the oven OFF  
            }
        }
        else if( temp > 178 || (elapsedMillis / 1000 ) > 240  )
        {
      
            if ( temp < 220 && (elapsedMillis / 1000) < 315 && !coolDownStarted )
            {
              // REFLOW: come up to 220, max of 75 seconds      
              if( !ovenOn )
              {
                Serial.println( "REFLOW: Turning oven ON" );
                digitalWrite(OVEN_ELEMENT_PIN, HIGH);   // turn the oven ON
                // turn the oven ON for this phase
                ovenOn = true;
                reflowStarted = true;
              } 
      
            } 
            else if( temp > 220 || (elapsedMillis / 1000) > 315 )
            {
              if( (elapsedMillis / 1000) < 435 )
              {
                  // to prevent the oven from going back into REFLOW once COOLDOWN has started
                  coolDownStarted = true;
                  
                  // COOLDOWN: turn off heat, come down to room temp
                  if( ovenOn )
                  {
                    Serial.println( "COOLDOWN: Turning oven OFF" );
                    digitalWrite(OVEN_ELEMENT_PIN, LOW);   // turn the oven OFF        
                    // turn the oven OFF for this phase
                    // and, once we implement the door control, open the door
                    // also, set off the alert buzzer here  
                    ovenOn = false;
                  }
              }
              else if( (elapsedMillis / 1000) > 435 )
              {
                // sanity check that the oven is off
                Serial.println( "FINALIZER: Turning oven OFF" );
                digitalWrite(OVEN_ELEMENT_PIN, LOW);   // turn the oven OFF 

                profileStopped = true; // set to TRUE so we don't do anything else until receiving an
                                       // explicit "start profile" command


                // set other values back to their default state to get ready for the next run
                profileStarted = false;
                reflowStarted = false;
                ovenOn = false;
                coolDownStarted = false;  
              }          
           } 
        }     
     }  
  }
  
  delay( 2000 );
}
