Code for a reflow oven controller based on an Arduino Nano 33 IOT device.
The hardware associated with this project assumes:

1. a SolidStateRelay (SSR) to toggle power to the oven heating elements
2. an AdaFruit MAX385655 thermocouple board with a type K thermocouple to read the temperature in the oven
3. an I2C LCD display for user prompts and other output messages
4. user input using a 74HC922 keypad decoder IC and a 16 button keypad

