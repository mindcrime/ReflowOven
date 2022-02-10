/********************************/
// includes
#include <Wire.h>
#include <Adafruit_MAX31856.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
/*********************************************************/

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// pins for getting data from the touchpad 
int DATA_AVAILABLE = 2;
int OUTPUT_ENABLE = 3;
int DATA_BUSA = 7;
int DATA_BUSB = 6;
int DATA_BUSC = 5;   // This appears to be correct from a cursory visual examination. 01-29-2022
int DATA_BUSD = 4;

// setup the pins used for talking to the thermocouple board
int DRDY_PIN = 9;
// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);

// the pin used for turning the oven element on and off
int OVEN_ELEMENT_PIN = 8;



// variables for getting data from the keypad input device
volatile char inputString[3];
volatile boolean inputAvailable = false;
volatile int inputPos = 0;


// variables for tracking the state of the running profile
boolean reflowStarted = false;
boolean coolDownStarted = false;
boolean profileStarted = false;
boolean profileStopped = false;

long startMillis = 0;
boolean ovenOn = false;
float temp = 0;
float lastTemp = 0;

int activeProfile = -1;


// another, redundant, set of variables for managing the running profiles. Delete all or
// most of these when merging in the gold "temperature control" code from the other project.

long profileStartTime = -1;
// int activeProfile = -1;
boolean profileRunning = false;
boolean abortRun = false;


// store profile data here. TODO: work out what the representation is going to be
// NOTE: a sample profile to model things after
// https://www.nxp.com/docs/en/supporting-information/Reflow_Soldering_Profile.pdf

//     RAMP / PREHEAT: target-temp: 150C, max-time: 120s
//    SOAK:           target-temp: 183C, max-time: 150s
//    REFLOW:         target-temp: 235C, max-time: 30s
//    COOL:           target-temp: room temperature

typedef struct ReflowProfile
{
  char profileName[64];
  int RAMP_TARGET;
  int RAMP_SECONDS;
  int SOAK_TARGET;
  int SOAK_SECONDS;
  int REFLOW_TARGET;
  int REFLOW_SECONDS;  
};


ReflowProfile profiles[2] = { {"Dummy Profile", 70, 30, 90, 30, 110, 20},
                              {"NXP Profile", 150, 120, 183, 150, 235, 30} 
                           };

void handleDataFromKeypad()
{
  Serial.println( "Received interrupt on DA line from keypad!" );  

  byte valA = digitalRead(DATA_BUSA);  
  byte valB = digitalRead(DATA_BUSB);  
  byte valC = digitalRead(DATA_BUSC);  
  byte valD = digitalRead(DATA_BUSD);  

  char emit[80];
  sprintf( emit, "Received valA: %d", valA );
  // Serial.println( emit );
  memset(emit, 0, sizeof(emit)/sizeof(emit[0]) );
  
  sprintf( emit, "Received valB: %d", valB );
  // Serial.println( emit );
  memset(emit, 0, sizeof(emit)/sizeof(emit[0]) );

  sprintf( emit, "Received valC: %d", valC );
  // Serial.println( emit );
  memset(emit, 0, sizeof(emit)/sizeof(emit[0]) );

  sprintf( emit, "Received valD: %d", valD );
  // Serial.println( emit );
  memset(emit, 0, sizeof(emit)/sizeof(emit[0]) ); 

  int received = 0;
  bitWrite(received, 0, bitRead(valA, 0) );
  bitWrite(received, 1, bitRead(valB, 0) );
  bitWrite(received, 2, bitRead(valC, 0) );
  bitWrite(received, 3, bitRead(valD, 0) ); 

  sprintf( emit, "Received value: %d", received );
  Serial.println( emit );

  if( inputPos > 1 && received != 14 )
  {
    // too many input characters with no terminator.
    // clear the input buffer  
    inputPos = 0;
    memset( (char*)inputString, 0, sizeof(inputString)/sizeof(inputString[0]));  

    // and set a "bad input" flag?
    Serial.println( "too many input characters with no terminator" );
    
  }
   
  switch( received )
  {
    case 0:
      inputString[inputPos] = '1';
      inputPos++;
      break;
    case 1:
      inputString[inputPos] = '2';
      inputPos++;
      break;
    case 2:
      inputString[inputPos] = '3';
      inputPos++;
      break;
    case 4:
      inputString[inputPos] = '4';
      inputPos++;
      break;
    case 5:
      inputString[inputPos] = '5';
      inputPos++;
      break;
    case 6:
      inputString[inputPos] = '6';
      inputPos++;
      break;
    case 8:
      inputString[inputPos] = '7';
      inputPos++;
      break;
    case 9:
      inputString[inputPos] = '8';
      inputPos++;
      break;
    case 10:
      inputString[inputPos] = '9';
      inputPos++;
      break;
    case 12:
      inputString[inputPos] = '*';
      inputPos++;
      break;
    case 13:
      inputString[inputPos] = '0';
      inputPos++;
      break;
    case 14:
      // inputString[inputPos] = '#';
      inputAvailable = true;
      inputString[inputPos] = '\0';
      inputPos = 0;
      Serial.println( "Found # character, finalizing input" );
      break;
    default:
      // just skip for now
      Serial.println( "Rut, roh!!" );
      break;
          
  }  
  
}

void setup()
{

  Serial.begin(9600);

  delay( 250 );
    
  attachInterrupt(digitalPinToInterrupt(DATA_AVAILABLE), handleDataFromKeypad, RISING );
  
  lcd.init();  //initialize the lcd

  Serial.println( "LCD initialized" );
  
  lcd.backlight();  //open the backlight 

  Serial.println( "Backlight enabled" );
  
  lcd.setCursor ( 0, 0 );            // go to the top left corner
  lcd.print(" Hello, Reflow User    "); // write this string on the top row
  lcd.setCursor ( 0, 1 );            // go to the 2nd row
  lcd.print("Enter control code now");   // pad string with spaces for centering
  lcd.setCursor ( 0, 2 );            // go to the third row
  lcd.print("* to terminate"); // pad with spaces for centering
  lcd.setCursor ( 0, 3 );            // go to the fourth row
  lcd.print("Code 99 to terminate active run");

  Serial.println( "Hello World message written to LCD" );

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

/*********************************************************/
void loop() 
{
  if( inputAvailable )
  {
      inputAvailable = false;
      Serial.println( "inputAvailable == TRUE!!!!" );
      char output[20];
      sprintf( output, "Profile: %s", inputString );
      Serial.println( output );
      
      memset( (char*)inputString, 0, sizeof(inputString)/sizeof(inputString[0]));  
      
      lcd.clear();
      Serial.println( "main loop() detected available user input" );
      
      lcd.setCursor ( 0, 0 );
      lcd.print( output );


      // check if a profile is running. If not, set the active profile to the provided profile number
      // unless the number is 99, which is the ABORT command. In that case, set the flag to kill the
      // running profile and turn the oven off. 

      // if a profile is already running, just ignore this, you can't start a new profile running while another
      // one is already running.

      int command = atoi( output );

      switch( command )
      {
        case 99:
          // TODO: set abort flag
          profileStopped = true;
          break;
       default:
          // set activeProfile to the supplied value, if activeProfile is currently empty
          // otherwise, ignore
    	  if( activeProfile == -1 )
    	  {
    		  activeProfile = command;
    	  }
    	  else
    	  {
    		  // NOP
    	  }
          break;
      }
  }

  // TODO: right now the profile details are hard-coded into the code below. Need to
  // modify this to use the profile parameters from the Profile data structure, for the
  // profile selected by the user.


  // TODO LATER: implement actual PID (or at least PI) control algorithm


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
