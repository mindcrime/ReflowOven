/********************************/
// include the library code
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
/*********************************************************/

int DATA_AVAILABLE = 2;
int OUTPUT_ENABLE = 3;
int DATA_BUSA = 7;
int DATA_BUSB = 6;
int DATA_BUSC = 5;   // This appears to be correct from a cursory visual examination. 01-29-2022
int DATA_BUSD = 4;

volatile char inputString[3];
volatile boolean inputAvailable = false;
volatile int inputPos = 0;

long profileStartTime = -1;
int activeProfile = -1;
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
  lcd.print(" Hello, EEVBlog!    "); // write this string on the top row
  lcd.setCursor ( 0, 1 );            // go to the 2nd row
  lcd.print("BUY SOME TEST GEAR");   // pad string with spaces for centering
  lcd.setCursor ( 0, 2 );            // go to the third row
  lcd.print("NOW, OR ELSE!!!"); // pad with spaces for centering
  lcd.setCursor ( 0, 3 );            // go to the fourth row
  lcd.print("    @mindcrime   ");

  Serial.println( "Hello World message written to LCD" );
  
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
          // abort
          break;
       default:
          // set activeProfile to the supplied value, if activeProfile is currently empty
          // otherwise, ignore     
          break;
      }
  
  }

  // check if a profile is already running. If not, do nothing but wait a little while
  
  /* 
    long profileStartTime = -1;
    int activeProfile = -1;
    boolean profileRunning = false;
    boolean abortRun = false;
  */

  // if a profile IS running, check the desired temperature at the current time-step and then check the
  // temperature from the thermocouple. Take any required action to account for a delta between the two values.
  // also, update the LCD with the current oven temperature and the elapsed time since the profile started.
  
  
  
  //   delay( 250 );
}
/************************************************************/
