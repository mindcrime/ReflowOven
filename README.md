Code for a reflow oven controller based on an Arduino Nano 33 IOT device.
The hardware associated with this project assumes:

1. a SolidStateRelay (SSR) to toggle power to the oven heating elements
2. an AdaFruit MAX385655 thermocouple board with a type K thermocouple to read the temperature in the oven
3. an I2C LCD display for user prompts and other output messages
4. user input using a 74HC922 keypad decoder IC and a 16 button keypad

## Note

Most of the code in this project represents little piecemeal experiments, as I worked with tiny aspects of this
project individually. So there's code for working out how to read input from the 74HC922 IC, code for interfacing
with the I2C LCD display, code for reading the thermocouple, etc.  Then there are pieces that start assembling the
small experimental pieces into larger chunks of the overall system, and so on and so on.

The "final" version that someone would use if they wanted to build an identical oven controller and actually run
this code is in the folder labeled "Oven_Control_with_LCD_combined".

Further note: as of 12-31-2022 the "final" code isn't quite finished, but all of the individual pieces have been
tested and shown to work, so wrapping up the final integration should happen fairly quickly now.

## Hardware

I'll put up some docs on the underlying hardware at some point. In the meantime, if anybody wants to take a stab
at building something like this, there is a ton of discussion, notes, pictures, etc. in [this thread](https://www.eevblog.com/forum/projects/finally-starting-on-this-convection-oven-gt-reflow-oven-conversion-project/) on the EEVBlog.com
forums.
