
int OVEN_PIN = 8;


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin OVEN_PIN as an output.
  pinMode(OVEN_PIN, OUTPUT);
}

boolean terminate = false;

// the loop function runs over and over again forever
void loop() {

  if( !terminate )
  {
  
    digitalWrite(OVEN_PIN, HIGH);   // turn the OVEN on (HIGH is the voltage level)
    delay(7000);                       // wait for a few seconds
    digitalWrite(OVEN_PIN, LOW);    // turn the OVEN off by making the voltage LOW
    terminate = true;
  }
  else
  {
    delay(360000);    
  }
  
  
  
  // wait for a while
  delay( 30000 );
}
